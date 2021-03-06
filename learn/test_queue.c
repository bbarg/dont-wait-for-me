#include "learnqueue.h"
#include <stdlib.h>
#include <stdio.h> 

void test_queue_simple() {
    /*
     * Simple single threaded test that 
     * initializes the queue and sentinel {0}
     * and then enqueues 10 nodes and 
     * deques them 
     */
    struct queue* q = init_queue(0); 

    int i, r, enq_ret;
    for (i = 1; i < 10; i++) {
        enq_ret = enqueue(q, i); 
        printf("Enqueue %d result %d\n", i, enq_ret); 
    }
    struct node* curr = q->head; 
    while (curr) {
        printf("iterating through queue - current node: %d\n", curr->val);
        curr = curr->next; 
    }

    int deq_ret = -2;  
    for (i = 0; i < 10; i++) {
        deq_ret = dequeue(q, &r); 
        printf("Dequeue %d returned %d\n", r, deq_ret); 
        r = -2; //reset r; 
    }

    delete_queue(q);  
}


void test_queue_mt() {
    printf("Testing ms-queue with multiple produces and consumers"); 

}

int main(int argc, char** argv){ 
    
    test_queue_simple();
    return 0; 
}
