# Concurrency ceiling test

## Question

What is the maximum level of concurrency/load that we can throw at a
server over localhost?

*OR*

What is the relationship between `-t` and `-c` flags in `weighttp`?

## Hypothesis

You need a certain amount of threads to generate a certain amount of
concurrency.

## Test strategy

Basically just send a bunch of requests at varying values of `-t` and
`-c` and see how much concurrency we can actually generate. We're also
interested in requests per second.

## Results

`weighttp` supports up to 10 concurrent connections per thread. It
basically shards concurrency over however many threads it has but
won't assign one thread more than 10 concurrent connections.

### Secondary question, req/s

*Is there any difference in the number of requests
per second for equivalent concurrency but differing number of
threads*

#### Data

(for `-n 1000 -c 100`)

`-t` results

10   finished in 0 sec, 36 millisec and 726 microsec, 13613 req/s, 284 kbyte/s
11   finished in 1 sec, 205 millisec and 136 microsec, 414 req/s, 8 kbyte/s
12   finished in 1 sec, 24 millisec and 99 microsec, 488 req/s, 9 kbyte/s
13   finished in 0 sec, 34 millisec and 124 microsec, 14652 req/s, 302 kbyte/s
14   finished in 1 sec, 206 millisec and 720 microsec, 414 req/s, 8 kbyte/s
15   finished in 1 sec, 4 millisec and 987 microsec, 497 req/s, 10 kbyte/s
16   finished in 1 sec, 207 millisec and 559 microsec, 414 req/s, 8 kbyte/s
17   finished in 1 sec, 4 millisec and 137 microsec, 497 req/s, 10 kbyte/s
18   finished in 1 sec, 4 millisec and 281 microsec, 497 req/s, 10 kbyte/s
19   finished in 1 sec, 3 millisec and 488 microsec, 498 req/s, 10 kbyte/s
20   finished in 1 sec, 3 millisec and 960 microsec, 498 req/s, 10 kbyte/s
21   finished in 1 sec, 7 millisec and 754 microsec, 496 req/s, 10 kbyte/s
22   finished in 1 sec, 23 millisec and 838 microsec, 488 req/s, 10 kbyte/s
23   finished in 1 sec, 3 millisec and 699 microsec, 498 req/s, 10 kbyte/s
24   finished in 1 sec, 4 millisec and 66 microsec, 497 req/s, 10 kbyte/s
25   finished in 1 sec, 19 millisec and 276 microsec, 490 req/s, 9 kbyte/s

#### Conclusions

Basically it doesn't matter how you shard concurrency among threads
which is good to know.

## Further Questions

Every now and then there's a huge spike up to around 15000 req/s. How
does this happen? Is there some buffer that's getting flushed?
