#include <linux/futex.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include "wait.h"

#define MAX_WAKEUP 16

int wait(int* addr) {
    int old_val = *addr; 
    return syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, old_val, NULL, NULL, 0); 
}


int futex_wake(int* addr, int n) {
    if (n == 1) {
        return syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, n, NULL, NULL, 0); 
    }

    else if (n > 1) {
        //Not sure but maybe we can do something more 
        //efficient here with FUTEX_REQUE
        return syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, n, NULL, NULL, 0); 
    }

    else {
        return -1; 
    }
}

int futex_signal(int* addr) {
    return futex_wake(addr, 1); 
}


int broadcast(int* addr) {
    return futex_wake(addr, MAX_WAKEUP); 
}
