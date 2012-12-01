rand
======
`rand` is a small command-line utility that outputs a random integer uniformly
distributed in a specified range. It is designed to be purely an interface to
the system PRNG(s) (e.g. `/dev/urandom` on \*nix), so it doesn't actually
perform any PRN generation itself. For more information, see
[the manual](docs/manual.asciidoc).

Compiling
---------
To build `rand`, you need the following on your system.

*   [CMake](http://www.cmake.org)
*   A compiler that supports C99.
*   The [GMP](http://gmplib.org/) library.
*   [AsciiDoc](http://www.methods.co.nz/asciidoc/).

Invoke `cmake [...] <srcdir>` to generate build files for your system.
