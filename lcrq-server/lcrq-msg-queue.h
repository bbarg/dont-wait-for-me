#ifndef _MSG_QUEUE_H_
#define _MSG_QUEUE_H_

/* msg-queue.h
 *
 * A wrapper for the the message queue used by http server. Different
 * object files can be compiled behind this to give different queue
 * implementations.
 */

/*
 * Implements a message that can be posted on a blocking thread queue
 */
struct message {
    int sock; // Payload, in our case a new client connection
};

/* Shell structure so as not to disrupt http-server queue semantic. */
struct queue {
    int dummy;
};

void queue_init(struct queue *q);
void queue_destroy(struct queue *q);
void queue_put(struct queue *q, int sock);
int queue_get(struct queue *q);

#endif
