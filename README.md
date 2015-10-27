failure-detector
===
An implementation of a failure detector. It is still a work-in-progress.

A [failure detector](http://www.cs.yale.edu/homes/aspnes/pinewiki/FailureDetectors.html)
is one of the building blocks for distributed consensus. Conceptually, a failure detector
"suspects" other processes (e.g. servers) in a cluster using pings.

Building
---
You will need a compiler that supports C++14. Only Linux and OS X are supported at the moment,
but things should be fine on other BSDs.

```sh
$ make       # Build main program
$ make test  # Runs tests (not that many at this point)
$ make clean # Cleans up
```

License
---
BSD (see LICENSE)
