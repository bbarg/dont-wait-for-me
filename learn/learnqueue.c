#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <assert.h>
#include "learnqueue.h"

struct node*
new_node(int val)
{
    struct node* node = (struct node *) malloc(sizeof(struct node));
    if (!node) {
	return NULL;
    }
    node->val  = val;
    node->next = NULL;

    return node; 
}

struct queue* 
init_queue(int init_val)
{
    struct node *init_node = new_node(init_val); 
    
    if (!init_node) {
	return NULL;
    }

    struct queue* queue = (struct queue *) malloc(sizeof(struct queue));
    if (!queue) {
	return NULL;
    }
    
    queue->head = init_node;
    queue->tail = init_node;

    return queue;
}

int
enqueue(struct queue *queue, int val)
{
    struct node *node = new_node(val);
    struct node* tail, *next;


    if (!node) {
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
	    } 
	    else {
		__CAS(&queue->tail, tail, next); 
	    }
		     
		  
	}
    }
    __CAS(&queue->tail, tail, node); //update tail to point to newly inserted node
    return 0; 	

}
