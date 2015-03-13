#define R 50 

struct crq_node {
    int idx; 
    int val; // can later change to void* 
}; 

struct lcrq {
    int head; 
    int tail; 
    struct lcrq* next; 
    struct node ring[R]; 
}; 
