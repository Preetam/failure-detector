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

This repository uses submodules. The following will fetch them if you haven't done so already:

```sh
$ git submodule update --init --recursive
```

You'll also need to install glog, a C++ implementation of the Google logging module.
Installing glog should be as simple as...

```sh
$ git clone https://github.com/google/glog.git && cd glog
$ ./configure
$ make
$ sudo make install
```

```sh
$ make       # Build main program
$ make test  # Runs tests (not that many at this point)
$ make clean # Cleans up
```

License
---
BSD (see LICENSE)
