/*
 * http-server.c
 */

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <sys/wait.h>   /* for wait() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <time.h>       /* for time() */
#include <netdb.h>      /* for gethostbyname() */
#include <signal.h>     /* for signal() */
#include <sys/stat.h>   /* for stat() */
#include <pthread.h>    /* POSIX threads, require -lpthread */
#include <errno.h>
#include <omp.h>
#include <stdint.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */

#define DISK_IO_BUF_SIZE 4096

/*
 * Implement a trivial threadpool of pre-created threads
 */
#define N_THREADS 16
#define QUEUE_POISON (-5)

#include "config.h"
#include "primitives.h"
#include "backoff.h"
#include "rand.h"
#include "thread.h"

#define POOL_SIZE                  1024

int MIN_BAK;
int MAX_BAK;



typedef struct Node {
  Object value;
  struct Node *next;
} Node;

typedef union pointer_t {
    struct{
        Node *ptr;
        long seq;
    } sep;
    long double con;
} pointer_t;

struct queue {
    struct Node* first; 
    struct Node* last; 
}; 

static struct queue thread_queue; 


volatile Node *head CACHE_ALIGN;
volatile Node *tail CACHE_ALIGN;
int64_t d1 CACHE_ALIGN, d2;

__thread Node *pool_node = null;
__thread int_fast32_t pool_node_index = 0;
__thread BackoffStruct backoff;


void SHARED_OBJECT_INIT(void) {
    Node *p = getMemory(sizeof(Node));
    p->next = null;
    head = p;
    tail = p;
    return;
}


inline void enqueue(Object arg, int pid) {
    Node *p;
    Node *next, *last;

    if ( (pool_node_index - 1) < 0) {
        pool_node = getMemory(POOL_SIZE * sizeof(Node));
        pool_node_index = POOL_SIZE;
    }
    pool_node_index -= 1;
    p = &pool_node[pool_node_index];

    p->value = arg;                       
    p->next = null;  
    reset_backoff(&backoff);
    while (true) {
        last = (Node *)tail;
        next = last->next;
        if (last == tail) {
            if (next == null) { 
                reset_backoff(&backoff);
                if (CASPTR(&last->next, next, p))
                    break;
            }
            else {
                CASPTR(&tail, last, next);
                backoff_delay(&backoff);
            }
        }
    }
    CASPTR(&tail, last, p);

    return;
}

inline Object dequeue(int pid) {
    Node *first, *last, *next;
    Object value;

    reset_backoff(&backoff);
    while (true) {
        first = (Node *)head;              // read Head.ptr and Head.count
        last = (Node *)tail;               // read Tail.ptr and Tail.count
        next = first->next;                // read next.ptr and next.count
        if (first == head) {               // are first, last, next still
             if (first == last) {          // is queue empty or
                 if (next == null) return -1;
                 CASPTR(&tail, last, next);
                 backoff_delay(&backoff);
             }
             else {
                  value = next->value;     // read before CAS
                  if (CASPTR(&head, first, next))
                      break;
                  backoff_delay(&backoff);
             } 
        } 
     }
     return value;
}


static int flag_term = 0; 


static pthread_t thread_pool[N_THREADS];

const char *webRoot = NULL;



static void die(const char *message)
{
    perror(message);
    exit(1);
}

void queue_init(struct queue* q) {
    SHARED_OBJECT_INIT(); 
}

void queue_destroy(struct queue* q) {
    fprintf(stderr, "queue destroy does nothing \n"); 
}

void queue_put(struct queue* q, int sock) {
    enqueue(sock, 0); 

}

int queue_get(struct queue *q ) {
    int x = 1;
    int ret; 
    while ((ret = dequeue(0)) == -1) {
        sleep(x); 
        x = (++x % 10); 
    }
    return ret; 
}

/*
 * Create a listening socket bound to the given port.
 */
static int createServerSocket(unsigned short port)
{
    int servSock;
    struct sockaddr_in servAddr;

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        die("socket() failed");

    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));       /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(port);              /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        die("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        die("listen() failed");

    return servSock;
}

/*
 * A wrapper around send() that does error checking and logging.
 * Returns -1 on failure.
 *
 * This function assumes that buf is a null-terminated string, so
 * don't use this function to send binary data.
 */
ssize_t Send(int sock, const char *buf)
{
    size_t len = strlen(buf);
    ssize_t res = send(sock, buf, len, 0);
    if (res != len) {
        perror("send() failed");
        return -1;
    }
    else
        return res;
}

/*
 * HTTP/1.0 status codes and the corresponding reason phrases.
 */

static struct {
    int status;
    char *reason;
} HTTP_StatusCodes[] = {
    { 200, "OK" },
    { 201, "Created" },
    { 202, "Accepted" },
    { 204, "No Content" },
    { 301, "Moved Permanently" },
    { 302, "Moved Temporarily" },
    { 304, "Not Modified" },
    { 400, "Bad Request" },
    { 401, "Unauthorized" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
    { 500, "Internal Server Error" },
    { 501, "Not Implemented" },
    { 502, "Bad Gateway" },
    { 503, "Service Unavailable" },
    { 0, NULL } // marks the end of the list
};

static inline const char *getReasonPhrase(int statusCode)
{
    int i = 0;
    while (HTTP_StatusCodes[i].status > 0) {
        if (HTTP_StatusCodes[i].status == statusCode)
            return HTTP_StatusCodes[i].reason;
        i++;
    }
    return "Unknown Status Code";
}


/*
 * Send HTTP status line followed by a blank line.
 */
static void sendStatusLine(int clntSock, int statusCode)
{
    char buf[1000];
    const char *reasonPhrase = getReasonPhrase(statusCode);

    // print the status line into the buffer
    sprintf(buf, "HTTP/1.0 %d ", statusCode);
    strcat(buf, reasonPhrase);
    strcat(buf, "\r\n");

    // We don't send any HTTP header in this simple server.
    // We need to send a blank line to signal the end of headers.
    strcat(buf, "\r\n");

    // For non-200 status, format the status line as an HTML content
    // so that browers can display it.
    if (statusCode != 200) {
        char body[1000];
        sprintf(body,
                "<html><body>\n"
                "<h1>%d %s</h1>\n"
                "</body></html>\n",
                statusCode, reasonPhrase);
        strcat(buf, body);
    }

    // send the buffer to the browser
    Send(clntSock, buf);
}

/*
 * Handle static file requests.
 * Returns the HTTP status code that was sent to the browser.
 */
static int handleFileRequest(
        const char *webRoot, const char *requestURI, int clntSock)
{
    int statusCode;
    FILE *fp = NULL;

    // Compose the file path from webRoot and requestURI.
    // If requestURI ends with '/', append "index.html".

    char *file = (char *)malloc(strlen(webRoot) + strlen(requestURI) + 100);
    if (file == NULL)
        die("malloc failed");
    strcpy(file, webRoot);
    strcat(file, requestURI);
    if (file[strlen(file)-1] == '/') {
        strcat(file, "index.html");
    }

    // See if the requested file is a directory.
    // Our server does not support directory listing.

    struct stat st;
    if (stat(file, &st) == 0 && S_ISDIR(st.st_mode)) {
        statusCode = 403; // "Forbidden"
        sendStatusLine(clntSock, statusCode);
        goto func_end;
    }

    // If unable to open the file, send "404 Not Found".

    fp = fopen(file, "rb");
    if (fp == NULL) {
        statusCode = 404; // "Not Found"
        sendStatusLine(clntSock, statusCode);
        goto func_end;
    }

    // Otherwise, send "200 OK" followed by the file content.

    statusCode = 200; // "OK"
    sendStatusLine(clntSock, statusCode);

    // send the file
    size_t n;
    char buf[DISK_IO_BUF_SIZE];
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        if (send(clntSock, buf, n, 0) != n) {
            // send() failed.
            // We log the failure, break out of the loop,
            // and let the server continue on with the next request.
            perror("\nsend() failed");
            break;
        }
    }
    // fread() returns 0 both on EOF and on error.
    // Let's check if there was an error.
    if (ferror(fp))
        perror("fread failed");

func_end:

    // clean up
    free(file);
    if (fp)
        fclose(fp);

    return statusCode;
}


void handleConnection(int clntSock)
{
    int statusCode;
    char line[1000];
    char requestLine[1000];
    struct sockaddr_in clntAddr;
    socklen_t clntLen;
    char *brk;

    FILE *clntFp = fdopen(clntSock, "r");
    /* Received an invalid or poisoned socket 
     * don't kill the whole system because we want to try to clean up 
     */
    if (clntFp == NULL)
        pthread_exit(NULL); 

    clntLen = sizeof(clntAddr);
    if (getpeername(clntSock, (struct sockaddr *)&clntAddr, &clntLen) != 0)
        die("getpeername failed");

    /*
     * Let's parse the request line.
     */

    char *method      = "";
    char *requestURI  = "";
    char *httpVersion = "";

    if (fgets(requestLine, sizeof(requestLine), clntFp) == NULL) {
        // socket closed - there isn't much we can do
        statusCode = 400; // "Bad Request"
        goto end;
    }

    char *token_separators = "\t \r\n"; // tab, space, new line
    method = strtok_r(requestLine, token_separators, &brk);
    requestURI = strtok_r(NULL, token_separators, &brk);
    httpVersion = strtok_r(NULL, token_separators, &brk);
    char *extraThingsOnRequestLine = strtok_r(NULL, token_separators, &brk);

    // check if we have 3 (and only 3) things in the request line
    if (!method || !requestURI || !httpVersion ||
        extraThingsOnRequestLine) {
        statusCode = 501; // "Not Implemented"
        sendStatusLine(clntSock, statusCode);
        goto end;
    }

    // we only support GET method
    if (strcmp(method, "GET") != 0) {
        statusCode = 501; // "Not Implemented"
        sendStatusLine(clntSock, statusCode);
        goto end;
    }

    // we only support HTTP/1.0 and HTTP/1.1
    if (strcmp(httpVersion, "HTTP/1.0") != 0 &&
        strcmp(httpVersion, "HTTP/1.1") != 0) {
        statusCode = 501; // "Not Implemented"
        sendStatusLine(clntSock, statusCode);
        goto end;
    }

    // requestURI must begin with "/"
    if (!requestURI || *requestURI != '/') {
        statusCode = 400; // "Bad Request"
        sendStatusLine(clntSock, statusCode);
        goto end;
    }

    // make sure that the requestURI does not contain "/../" and
    // does not end with "/..", which would be a big security hole!
    int len = strlen(requestURI);
    if (len >= 3) {
        char *tail = requestURI + (len - 3);
        if (strcmp(tail, "/..") == 0 ||
            strstr(requestURI, "/../") != NULL)
        {
            statusCode = 400; // "Bad Request"
            sendStatusLine(clntSock, statusCode);
            goto end;
        }
    }

    /*
     * Now let's skip all headers.
     */

    while (1) {
        if (fgets(line, sizeof(line), clntFp) == NULL) {
            // socket closed prematurely - there isn't much we can do
            statusCode = 400; // "Bad Request"
            goto end;
        }
        if (strcmp("\r\n", line) == 0 || strcmp("\n", line) == 0) {
            // This marks the end of headers.
            // Break out of the while loop.
            break;
        }
    }

    /*
     * At this point, we have a well-formed HTTP GET request.
     * Let's handle it.
     */
    statusCode = handleFileRequest(webRoot, requestURI, clntSock);

end:
    /*
     * Done with client request.
     * Log it, close the client socket.
     */

    inet_ntop(AF_INET, &clntAddr.sin_addr, line, clntLen);
    fprintf(stderr, "%s \"%s %s %s\" %d %s\n",
            line,
            method,
            requestURI,
            httpVersion,
            statusCode,
            getReasonPhrase(statusCode));
    // close the client socket
    fclose(clntFp);
}


static void *threadMain(void *data)
{
    int clntSock;

    for (;;) {
        clntSock = queue_get(&thread_queue);
        handleConnection(clntSock);
    }
    return NULL;
}


static void *mainLoop(int servSock)
{
    struct sockaddr_in clntAddr;

    for(;;) {
        /*
         * wait for a client to connect
         */

        // initialize the in-out parameter
        unsigned int clntLen = sizeof(clntAddr);
        int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntLen);
        if (errno == EINTR && flag_term) {
            //Received SIGINT and can exit loop on accept
            return NULL; 
        }
        if (clntSock < 0)
            die("accept() failed");

        // Put the socket on the queue and wakep sleeping threads
        queue_put(&thread_queue, clntSock);
    }

    return NULL;
}
void cleanup_handler(int signum) {
    flag_term = 1; 
}

int main(int argc, char *argv[])
{
    int i;
    struct sigaction sa; 
    memset(&sa, 0, sizeof(sa)); 
    sa.sa_handler = cleanup_handler; 
    sa.sa_flags = 0; 

    // Ignore SIGPIPE so that we don't terminate when we call
    // send() on a disconnected socket.
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        die("signal() failed");
    if (sigaction(SIGINT, &sa, NULL) < 0)
        die("sigaction() failed");
    


    if (argc != 3) {
        fprintf(stderr, "usage: %s <server_port> <web_root>\n", argv[0]);
        exit(1);
    }

    unsigned short servPort = atoi(argv[1]);
    webRoot = argv[2];

    queue_init(&thread_queue);

    int servSock = createServerSocket(servPort);

    for(i = 0; i < N_THREADS; i++) {
        pthread_create(&thread_pool[i], NULL, threadMain, NULL);
    }

    // Run the main loop in the main thread. The main loop accepts new
    // connection and passes them to worker threads in the thread pool via a
    // synchronized blocking queue.
    mainLoop(servSock);

    /* Poison the queue to stop worker threads */ 
    for (i = 0; i< N_THREADS; i++) {
        queue_put(&thread_queue, QUEUE_POISON);
    }

    /* Wait for all the worker threads to exit and destroy queue */ 
    for(i = 0; i < N_THREADS; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    queue_destroy(&thread_queue);
    return 0;
}
