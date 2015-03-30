#include <linux/futex.h>


int wait(int* addr); 
int futex_wake(int* addr, int n); 
int futex_signal(int* addr); 
int broadcast(int* addr);
