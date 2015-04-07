# Nieh's Prompt
An extended abstract is a condensed version of your paper. It should
be 5-6 pages, double column, 10 point font, single space, and 1 inch
margins. You should have most of your research completed at this
point. The abstract should include a complete discussion of all
related work, a complete description of the research ideas, including
mechanisms and algorithms developed. It should provide a complete
description of experiments that have been done or will be done to
validate the work, though some experiments and data may still be
missing.

# Research Question
Can a simple thread-pooled web server (tha uses a lock-free queue to pass
connections to works) compete with the performance of existing web
servers under heavy load?
- If so, under what conditions does it work better? What about the
  lock-free queue approach makes such a big difference?
- If not, what key factor makes web servers a bad application of
lock-free queues?

# Don't Wait For Me: Evaluating the Applicability of Lock-free Queues in High-Load Web Servers
## Abstract
Using lock-free queue implementations taken from various authors, we present a
measurement and comparison study of the performance of multithreaded web
servers that use these queues to perform inter-thread communication for a
producer-consumer workflow. We then compared our web servers to existing open
source options like Nginx and Lighttpd, to compare the relative performance of
a server architecture driven by queue message passing versus event polling or
thread spawning, to see how effective lock free queues are for this class of
program. Preliminary results on an 8 core system show that servers built with
various lock free queues are significantly more performant than ones built
with globally locked queues, even approaching the performance of Nginx when in-
application file caching is implemented. However, further experiments and
optimiztions are needed before we can draw further conclusions. 

## Introduction
In the past 20 years, there has been an explosion of research into
lock-free synchronization. Lock-free objects possess numerous provable
guarantees lacked by locked objects, including deadlock immunity and
async-signal safety. In addition, operations on lock-free data
structures have the potential to be significantly faster than those on
comparable locked objects, particuarly under heavy contention. In a
world where heavily multicore processors are widely available,
user-level applications are well-behooved to use data structures
optimized for the distributed nature of multiple cores.

High performance web servers provide an intriguing application for
lock-free algorithms, given that they deal with extremely high
concurrency and in certain situations would benefit from progress guarantees
(a good example would be an ad exchange server where each request
represents explicit monetary value). We observe that most modern
servers targeted towards high-concurrency (in particular \verb+nginx+) have
shied away from user-level job distribution and instead rely on kernel
mechanisms for reporting on file descriptors (the so-called
"event-based" server architecture). Our goal is to explore the
limitations of a thread-pooled server architecture that uses a queue
for job distribution through comparison with both a locked version of
the same architecture and with popular modern high-performance web
servers.

## Related Work

The related work for this paper can be split into two sections - that which is related to lock free queue implementations and that which is related to server performance testing. 

### Lock-free queues 

Lock-free programming has been an active area of research for at least the
past thirty years. Michael and Scott present a linked list based nonblocking
queue (referred to as the MS-queue). Their implementation relies on the
compare-and-swap primitive (CAS) that allows for atomic manipulation of linked 
list pointers. Kogan and Petrank give a wait-free variant of the MS-queue However both these, and other versions of the MS-queue suffer from scalability
problems after a small amount of concurrency because threads contend for
memory locations to perform CAS, and often get stuck in retry loops that are
prohibitively expensive. 

Other researchers (Hendler et al, Fatourou and Kallimanis) have shown that
combining-based queues perform better than CAS queues, where a combining queue
essentially serializes access to the queue by only allowing one thread to
perform operations on the queue. The other threads publish their intended
operation on a shared array. The idea is that past a small amount of threads,
CAS based queue performance degrades so much as to be entirely useless, and
instead sequential access is preferable. In practice, combining queues have an
even greater advantage as the combining thread is pinned to a single CPU, and
stays cache-hot throughout its execution. 

Finally, Afek and Morrison present a linearizable concurrent nonblocking queue based on a linked list of concurrent ring buffers (LCRQ). This queue avoids the CAS contention problems that plague MS-queue variants by using theoretically weaker primitives like fetch-and-add (FA). This queue ends up being significantly faster than all previous queues 

### Server performance testing 

Veal and Foong present a detailed analysis of the performance scalability of multicore web servers, from which they concludue that the primary bottlenecks inhibiting web server scalability were system bus hardware design flaws. Hashemian describes strategies for benchmarking servers on multicore systems and automation strategies.  

## Implementation

As of the current state of our research, we are testing three naive
implementations of thread-pooled, queue-based web servers, which we
refer to from here on as `http-server`, `msq-server`, and
`lcrq-server`. These implementations serve static content on a single
port, with worker threads sleeping if the work queue is empty. A
single acceptor thread loops on accept and adds connections to the
queue (in the form of client socket file descriptors) as they
arrive. All three servers are written solely in C and use the POSIX
sockets library directly to create and serve client sockets. By
default, the servers support logging of incoming connections to
`stdout`, although in Section 4 we observe a marked performance
increase when logging is disabled.

It should be clarified that none of `http-server`, `msq-server`, or
`lcrq-server` are intended as full-featured and robust servers that
would at this point in time be used to replace existing servers
(although the their feature-set isn't extremely far away from that of
`lighttpd`). Our goal is to compare event-based and queue-based server
architectures under extremely high load, so we have chosen minimal
queue-based implementation to isolate the performance of the queue
within the server.

### `http-server`

This version of the server is the basis for the others, and uses a
singly-locked queue (one lock is used for both enqueueing and
dequeueing). The queue also uses a condition variable that worker
threads may sleep on when no jobs are available.

### `msq-server`

This version is a modified copy of `http-server` with the single
locking queue replaced by an implementation of Michael and Scott's
seminal MPMC lock-free queue [ref to MSQ paper, ref to `sim`]. POSIX
condition variables can no longer be used to implement sleeping on an
empty queue; instead we use a light wrapper over the `futex` system
call. This particular implementation of the Michael and Scott queue
returns -1 whenever a `dequeue` fails on an empty queue; we use that
return value as our sleeping condition.

### `lcrq-server`

Also a modified copy of `http-sever`, `lcrq-server` replaces the
locking queue with an implementation of Morrisson and Afek's so-called
LCRQ [reference to lcrq paper]. The LCRQ is a linked list of ring
buffers that uses fetch-and-add as its primary atomic primitive (when
performing operations on an inidividual ring buffer), falling back to
compare-and-swap only when the new ring buffers need to be added to
the linked list. Although LCRQ is an MPMC queue, we only have a single
accepting thread and thus a single enqueuer. Like for the Michael
Scott queue, `dequeue` returns -1 on an empty queue, so we use the
same `futex` wrapper to implement sleeping.

### Acknowledged Limitations

Currently, we do not have a robust lock-free memory allocation or
memory reclamation strategy in place for `msq-server` and
`lcrq-server`. When new nodes are needed, the acceptor thread simply
calls `malloc` within each queue implementation to create a new
node. While this reliance on a locking `malloc` admittedly affects the
supposed progress guarantee of the lock-free algorithms we use, we
hold that it should not signicantly effect performance, as only the
accepting thread is contending for the `malloc` lock. Usage or
implementation of a lock-free (or otherwise robust) memory allocator
would likely *improve* server performance, given the options for
per-thread pooling [mckinney reference] and CPU memory locality
[mckinney also?].

As for memory reclamation, the standard and popular lock-free solution
is Maged M. Michael's hazard pointers
[hazard pointers reference]. Hazard pointers allow threads operating
on a shared lock-free object to temporarily ensure that hazardous
references (for example a pointer to the next item in a queue) will
remain valid as long as the thread holds one of a finite number of
hazard pointers to the object. There is a small amount of overhead
associated with hazard pointers, as the implementation requires both
declaring the lifetime of hazardous reference within operations on the
object and a periodic scanning of the global list of hazard pointers
to lazily free nodes. We acknowledge that performance for
`lcrq-server` and `msq-server` would likely be slower with a hazard
pointer implementation, but we view generating research claims via
server profiling as a higher priority in our current research than the
production of a hazard pointers implementation.

## Testing Strategy

Our testing strategy centers around two main goals:

1. What are the traditional bottlenecks of a queue-based web server
   architecture and how could a lock-free queue possibly circumvent
   those? 
2. How closely can an optimized version of a lock-free-queue based
   webserver approach the performance (under heavy load) of existing
   web servers `nginx`, `lighttpd`, and `apache`?

For testing, we make heavy use of HP's `httperf` utility, which allows
sending adjusting the per-second requests rate and setting timeouts,
and which has the crucial feature of continuing to send requests
without recieving replies from the server. This tool, combined with a
fast enough connection to the server, allows us to max out our
servers' capacity for concurrency.

Our tests were run on a rack server with two quad-core Intel Xeon
L5420 2.50 GHz processors, each with a 12 MB L2 cache. The system has
16 GB of RAM and runs Ubuntu 14.04.2 LTS (Linux kernel version
3.13.0-46-generic).

### Server latency 

This is a really simple experiment designed to test the server request latency. We use httperf to send increasing numbers of requests as fast as possible, and measured the total time required to process those requests. Larger numbers of files generate longer running test times, but this gives a better measurement of average processing rate (requests/second) that takes into account nondeterministic factors like TCP/IP warmup, network noise, filesystem caching and
others.  

### Server throughput 

Here, requests are 

### Goals of Testing
- specify that we're looking to see if lock-freedom can make the SPMC
  strategy viable for serving static content
- address as many variables as possible as we talk about design
  + network latency
  + file caching
  + scheduling policy
  + atomic primitive implementation
  + per-CPU cache activity
  + [other stuff we don't know about yet]

### Strengths of Lock-Free Algorithms in Web Servers

Our initial tests are with a two naive implementations of
thread-pooled servers that use a lock-free queue to distribute
connections from a single accepting thread to several worker threads. 

### Comparison with Existing Web Servers
## Conclusion
