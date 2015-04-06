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
## Introduction
In the past 20 years, there has been an explosion of research into
lock-free synchronization. Lock-free objects possess numerous provable
guarantees lacked by locked objects, including deadlock immunity and
async-signal safety. In addition, operations on lock-free data
structures have the potential to be significantly faster than those on
comparable locked objects, particuarly under heavy contention. In a
world where heavily multicore processors are widely available,
it user-level applications are well-behooved to use data structures
optimized for the distributed nature of multiple cores.

High performance web servers provide an intriguing application for
lock-free algorithms, given that they deal with extremely high
concurrency and in certain situations benefit from progress guarantees
(a good example would be an ad exchange server where each request
represents explicit monetary value). 

## Related Work
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
