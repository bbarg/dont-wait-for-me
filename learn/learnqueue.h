#define __CAS __sync_bool_compare_and_swap
#define POINTER_INIT(node, count)		\
  {						\
      .ptr = node,				\
      .count = count				\
  }

struct node;

/*struct pointer_t {
    struct node *ptr;
    unsigned int count;
};*/

struct node {
    int val;
    struct node*  next;
};

struct queue {
    struct node* head;
    struct node* tail;
};

struct node* new_node(int); 
struct queue* init_queue(int); 
int enqueue(struct queue*, int); 


