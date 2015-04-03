# Manipulating `NTHREADS`

## Question

How does the performance of the lock-free and locked servers change
with the number of worker threads?

## Hypothesis

Performance for the locked queue will reach some maximum around the
number of CPUs and then degrade from there. Performance for ther
lock-free queue will increase slightly past number of CPUs and plateau
closer to 2 per core. 

The idea here is that the primary difference between the locked and
lock-free approaches is their ability to handle heavy contention of
the dequeue side of the queue. Contention increases with the number of
threads, but eventually hits a plateau when the context switch
overhead is too bad.

## Testing strategy

Warmup:

```
weighttp -n 10000 -c 100 -t 10 localhost:8888/1k
```

For each server we'll 

## Results


