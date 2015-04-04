# dont-wait-for-me

Code for Columbia University Advanced Operating Systems project under
Prof. Jason Nieh on wait-free queues and linked lists

## The state of our testing

- It seems like `lighttpd` has significantly less ability to handle
  large numbers of connections (and maybe apache and nginx will also
  have this problem). Is `lighttpd` actually really bad at handling
  like 30000 concurrent connections, or are we just missing something?
  + potential solution: try to test on one core to eliminate
    contention effects.
