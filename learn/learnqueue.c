#include <stdio.h>
#include <pthread.h>
#include <assert.h>

struct node;

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
    queue = (queue *)malloc(sizeof(struct 



}
