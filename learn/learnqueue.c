#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <assert.h>
#include "learnqueue.h"
int
new_node(struct node *node, int val)
{
    node = (struct node *) malloc(sizeof(struct node));
    if (!node) {
	return -1;
    }
    node->val  = val;
    node->next = NULL;

    return 0;
}

int
init_queue(struct queue *queue, int init_val)
{
    struct node *init_node;
    
    if (new_node(init_node, init_val)) {
	return -1;
    }

    queue = (struct queue *) malloc(sizeof(struct queue));
    if (!queue) {
	return -1;
    }
    
    queue->head = init_node;
    queue->tail = init_node;

    return 0;
}

int
enqueue(struct queue *queue, int val)
{
    struct node *node;
    struct node* tail, *next;


    if (new_node(node, val)) {
	return -1;
    }

    while (1) {
	tail = queue->tail;
	next = tail->next;
	if (tail == queue->tail ) { //elementwise compare earlier tail and current tail 
	    if (next == NULL) {
		if (__CAS(&tail->next, next, node )) {
		    break;
		}
	    } else {
	      __CAS(&queue->tail, tail, tail->next);
	    }
	}
    }


}

int
dequeue(struct queue *queue, int *val)
{
    struct node *head, *tail, *next;

    while (1) {
	head = queue->head;
	tail = queue->tail;
	next = head.next;
	if (head == queue->head) {
	    if (head == tail) {	      /* list might be empty */
		if (head->next == NULL) { /* list is definitely empty */
		    return -1;	      /* dequeue fails on empty list */
		}
		__CAS(queue->tail, tail, next);
	    }
	}
    }
}
