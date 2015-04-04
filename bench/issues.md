# Issues doc for testing

- What is the overhead of logging to `stdout`?
- Why is there sometimes a huge spike in req/s when testing with `weighttp`?
- It seems light at least `lighttpd` (and probably apache and nginx)
  send differently sized HTTP headers. How can we take this into
  accounts for our tests?
