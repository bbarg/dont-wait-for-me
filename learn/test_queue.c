#include "learnqueue.h"
#include <stdlib.h>
#include <stdio.h> 

void test_queue() {

    struct queue* q = init_queue(0); 

    int i, r; 
    for (i = 1; i < 10; i++) {
        printf("Enqueue %d result %d\n", i, enqueue(q, i)); 
    }
    int deq_ret; 
    for (i = 0; i < 10; i++) {
        deq_ret = dequeue(q, &r); 
        printf("Dequeue %d returned %d\n", r, deq_ret); 
    }
}

int main(int argc, char** argv){ 
    
    test_queue();
    return 0; 
}
