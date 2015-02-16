#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#define NTHREADS 10
#define NINCREMENTS 1000000

void *
thread(void *p)
{
    int i, tmp;
    int *loc = (int *) p;

    for(i = 0; i < NINCREMENTS; i++) {
        tmp = *loc;
	while (!__sync_bool_compare_and_swap(loc, tmp, tmp + 1)) {
	    tmp = *loc;
	}
    }

    pthread_exit(NULL);
}


int
main(int argc, char **argv)
{
    pthread_t tds[NTHREADS];
    int shmem, i;

    for(i = 0; i < NTHREADS; i++) {
	assert(pthread_create(&tds[i], NULL, &thread, (void *) &shmem) == 0);
    }

    for(i = 0; i < NTHREADS; i++) {
	assert(pthread_join(tds[i], (void **)0) == 0);
    }

    if (shmem == NTHREADS * NINCREMENTS) {
        printf("FUCK YEAH!!!\n");
    } else {
        printf("FUCK NO!!!\n");
    }
}
