rand
======
`rand` is a small command-line utility that outputs a random integer uniformly
distributed in a specified range. It is designed to be purely an interface to
the system PRNG(s) (e.g. `/dev/urandom` on \*nix), so it doesn't actually
perform any PRN generation itself.

Compiling
---------
To build `rand`, you need the following on your system.

*   [CMake](http://www.cmake.org)
*   A compiler that supports C11.
*   The [GMP](http://gmplib.org/) library.

Invoke `cmake [...] <srcdir>` to generate build files for your system.

Running
-------
See `rand -h`.
