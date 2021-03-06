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
#include "wait.h"

#define MAXPENDING 5    /* Maximum outstanding connection requests */

#define DISK_IO_BUF_SIZE 4096

/*
 * Implement a trivial threadpool of pre-created threads
 */
#define DEFAULT_THREADS 1
#define QUEUE_POISON (-5)


static int flag_term = 0; 
int futex_addr = 0; 

/*
 * Implements a message that can be posted on a blocking thread queue
 */
struct message {
    int sock; // Payload, in our case a new client connection
    struct message *next; // Next message on the list
};


/*
 * This structure implements a blocking POSIX thread queue. If a thread
 * attempts to pop an item from an empty queue it is blocked until another
 * thread appends a new item.
 */
struct queue {
    pthread_mutex_t mutex; // A mutex used to protect the queue itself
    pthread_cond_t cond;   // A condition variable for threads to sleep on
    struct message *first; // First message in the queue
    struct message *last;  // Last message in the queue
    unsigned int length;   // Number of elements on the queue
};

const char *webRoot = NULL;

static struct queue thread_queue;


static void die(const char *message)
{
    perror(message);
    exit(1);
}


static void queue_init(struct queue *q)
{
    memset(q, 0, sizeof(*q));

    if (pthread_cond_init(&q->cond, NULL) != 0)
        die("Cannot initialize queue condition variable");

    if (pthread_mutex_init(&q->mutex, NULL) != 0) {
        pthread_cond_destroy(&q->cond);
        die("Cannot initialize queue mutex");
    }

    futex_addr = 0; 
}


static void queue_destroy(struct queue *q)
{
    struct message *tmp;

    // First of all delete all elements currently on the queue
    pthread_mutex_lock(&q->mutex);
    while(q->first) {
        tmp = q->first;
        q->first = q->first->next;
        free(tmp);
    }
    q->last = NULL;
    q->length = 0;
    pthread_mutex_unlock(&q->mutex);

    // After that destroy the mutex and the condition variable
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}


static void queue_put(struct queue *q, int sock)
{
    struct message *msg;

    // Create a new message object, initialized to all zeroes and store the
    // socket file descriptor in its sock attribute
    msg = (struct message *)calloc(1, sizeof(*msg));
    if (msg == NULL)
        die("Out of memory");
    msg->sock = sock;

    // Append the new message to the queue
    pthread_mutex_lock(&q->mutex);
    if (q->last == NULL) {
        q->last = msg;
        q->first = msg;
    } else {
        q->last->next = msg;
        q->last = msg;
    }

    // If the queue was previously empty, wakep up any threads that might be
    // sleeping, waiting for the queue to contain data
    if (q->length == 0)
	broadcast(&futex_addr); 
    q->length++;
    pthread_mutex_unlock(&q->mutex);
}


int queue_get(struct queue *q)
{
    int sock;
    struct message *msg;

    pthread_mutex_lock(&q->mutex);

    /* Since all the sleeping threads content for the data on the queue, we
     * need to check if the queue is non-empty after we woke up. If yes we
     * won, if not we go back to sleep on the condition variable.
     * pthread_cond_wait unlocks the give mutex and suspends the thread.
     */
    while (q->first == NULL) {
        pthread_mutex_unlock(&q->mutex);
        wait(&futex_addr); 
        pthread_mutex_lock(&q->mutex); 
    }

    // We won this round, remove the data from the queue and return it.
    msg = q->first;
    q->first = q->first->next;
    q->length--;

    if (q->first == NULL) {
        q->last = NULL;
        q->length = 0;
    }

    sock = msg->sock;
    free(msg);
    pthread_mutex_unlock(&q->mutex);
    return sock;
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

    int nthreads;
    char *nthreads_str = getenv("NTHREADS");

    if (nthreads_str) {
	nthreads = atoi(nthreads_str);
	fprintf(stderr, "-- Starting server with %d worker threads\n", nthreads);
    } else {
	nthreads = DEFAULT_THREADS;
	fprintf(stderr, "-- Defaulting to 1 worker thread; you probably want to set NTHREADS\n");
    }
    
    pthread_t *thread_pool = malloc(sizeof(pthread_t) * nthreads);
    
    for(i = 0; i < nthreads; i++) {
        pthread_create(&thread_pool[i], NULL, threadMain, NULL);
    }

    // Run the main loop in the main thread. The main loop accepts new
    // connection and passes them to worker threads in the thread pool via a
    // synchronized blocking queue.
    mainLoop(servSock);

    /* Poison the queue to stop worker threads */ 
    for (i = 0; i< nthreads; i++) {
        queue_put(&thread_queue, QUEUE_POISON);
    }

    /* Wait for all the worker threads to exit and destroy queue */ 
    for(i = 0; i < nthreads; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    free(thread_pool);
    queue_destroy(&thread_queue);
    return 0;
}
