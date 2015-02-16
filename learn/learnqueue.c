#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#define __CAS __sync_bool_compare_and_swap

struct node;

/* TODO
   - propose that pointer_t is 64 bits wide (half pointer, half count)
 */
struct pointer_t {
    struct node *ptr;
    unsigned int count;
};

struct node {
    int val;
    struct pointer_t next;
};

struct queue {
    struct pointer_t head;
    struct pointer_t tail;
};

int
new_node(struct node *node, int val)
{
    node = (node *)malloc(sizeof(struct node));
    if (!node) {
	return -1;
    }
    node->val = val;
    node->next.ptr = NULL;
    node->next.count = 0;

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
    
    queue->head.ptr = init_node;
    queue->head.count = 0;
    queue->tail.ptr = init_node;
    queue->tail.count = 0;

    return 0;
}

int
enqueue(struct queue *queue, int val)
{
    struct node *node;
    struct pointer_t tail, next;

    if (new_node(node, val)) {
	return -1;
    }

    while (1) {
	tail = queue->tail;
	next = tail.ptr->next;
	if (tail = queue->tail) {
	    if (next.ptr == NULL) {
		if (__CAS(&tail.ptr->next,
			  next,
			  (struct pointer_t *)
			  { node, next.count + 1 })) {
		    break;
		}
	    } else {
		__CAS(&queue->tail,
		      tail,
		      (struct pointer_t *)
		      {
	    }
	}
    }


}
