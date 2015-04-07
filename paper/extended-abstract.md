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
represents explicit monetary value). The dual needs of high throughput and low latency are characteristic of programs that are well suited for lock free
data structures. Furthermore, FIFO queues are commonly used to create a producer-consumer data structure that is commonly used in many other parallel applications and operating systems, and this study is relevant to those as well.

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
### Basic HTTP-Server
### Michael and Scott Lock-Free Queue Server
### Morrisson and Afek LCRQ Server
## Testing Strategy
- platform
- note of the `dont-wait-for-me` git repo for testing reproduction
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
### Comparison with Existing Web Servers
## Conclusion
