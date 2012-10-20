rand
======
`rand` is a small command-line utility that prints a random integer to the
console. It is an interface to the system random number generator(s) (e.g.
`/dev/urandom` on \*nix), so it doesn't actually perform any PRNG generation
itself. For usage details, see `rand -h`.

Compiling
---------
To build `rand`, you need the following on your system.

*   [CMake](http://www.cmake.org)
*   A compiler that supports C11.
*   The [GMP](http://gmplib.org/) library.
*   The ppuryear/simple-strconv source tree. It should be installed or
    symlinked to a subdirectory of the `rand` source tree named
    `simple-strconv`.

Invoke `cmake [...] <srcdir>` to generate build files for your system.
