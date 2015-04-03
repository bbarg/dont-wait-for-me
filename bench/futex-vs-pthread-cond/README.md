# `http-server` with `futex_wait` vs. `pthread_cond_wait`

## Question

Is there a meaningful performance difference between the futex-waiting
and condition-variable-waiting versions of `http-server`?

## Hypothesis

The futex-waiting version will be slightly faster because there is
less overhead associated with the futex system call. However, the
performance differences will not be particularly pronounced since both
have to go through the `pthread` library to obtain the queue mutex.

## Testing strategy

Because this is to test the performance difference from different wait
queue strategies, we'll use many requests for a small file.

### Platform

`os2server`:

- Two quad-core 2.50 GHz Intel Xeon processors
- 16 GB RAM

### Test details

For each server,

- at 16 and 32 worker threads
- -c 50 -t 8

do the following:

1. Warmup of 10000 requests for a 10 byte file (to actually get some
   throughput).
1. 10000 requests for a 10 byte file (actually get the measurements
from this)

The test can be repeated as many times as needed to generate
additional results.

## Results

### `http-server`

Test 1:

> ## Question

Is there a meaningful performance difference between the futex-waiting
and condition-variable-waiting versions of `http-server`?

## Hypothesis

The futex-waiting version will be slightly faster because there is
less overhead associated with the futex system call. However, the
performance differences will not be particularly pronounced since both
have to go through the `pthread` library to obtain the queue mutex.

## Testing strategy

Because this is to test the performance difference from different wait
queue strategies, we'll use many requests for a small file.

### Platform

`os2server`:

- Two quad-core 2.50 GHz Intel Xeon processors
- 16 GB RAM

### Test details

For each server,

- at 16 and 32 worker threads
- -c 50 -t 8

do the following:

1. Warmup of 10000 requests for a 10 byte file (to actually get some
   throughput).
1. 10000 requests for a 10 byte file (actually get the measurements
from this)

The test can be repeated as many times as needed to generate
additional results.

## Results

### `http-server`

#### at 16 threads

(all of these tests are on the same "warmed-up" version of the http-server)

1. finished in 2 sec, 213 millisec and 536 microsec, 4517 req/s, 92 kbyte/s
1. finished in 1 sec, 643 millisec and 990 microsec, 6082 req/s, 122 kbyte/s
1. finished in 1 sec, 608 millisec and 451 microsec, 6217 req/s, 122 kbyte/s

#### at 32 threads

1. finished in 1 sec, 527 millisec and 569 microsec, 6546 req/s, 131 kbyte/s
1. finished in 1 sec, 522 millisec and 362 microsec, 6568 req/s, 131 kbyte/s
1. finished in 3 sec, 6 millisec and 821 microsec, 3325 req/s, 69 kbyte/s
	- what?

### `http-server-futex`

#### at 16 threads

(all of these tests are on the same "warmed-up" version of the
http-server)

1. finished in 1 sec, 407 millisec and 829 microsec, 7103 req/s, 148 kbyte/s
1. finished in 1 sec, 527 millisec and 322 microsec, 6547 req/s, 129 kbyte/s
1. finished in 1 sec, 445 millisec and 239 microsec, 6919 req/s, 137 kbyte/s

#### at 32 threads

1. finished in 1 sec, 529 millisec and 468 microsec, 6538 req/s, 132 kbyte/s
1. finished in 1 sec, 406 millisec and 897 microsec, 7107 req/s, 143 kbyte/s
1. finished in 1 sec, 512 millisec and 512 microsec, 6611 req/s, 132 kbyte/s

## Conclusion

The futex server seems to be slightly faster, to the point that I
think we can use it in general for our tests.

## Questions

It's possible that I didn't test at high enough concurrency to really
generate a difference, but I think that wouldn't make a huge
difference because the only difference between the two calls is the
small additional overhead of `pthread_cond_wait` vs. calling `futex()`
directly.
