random
======
`random` is a small command-line utility that outputs a random integer
uniformly distributed in a specified range. For more information, see
[the manual](manual.asciidoc).

Compiling
---------
To build `random`, you need the following on your system.

*   [CMake](http://www.cmake.org)
*   A compiler that supports C99.
*   The [GMP](http://gmplib.org/) library.
*   [AsciiDoc](http://www.methods.co.nz/asciidoc/)

Populate the submodule directories with `git submodule update --init`, then
invoke `cmake [...] <srcdir>` to generate build files for your system.

Examples
--------
Decimal number in [0, 100):
```
$ random 100
73
```

Binary 32-bit word:
```
$ random -b 2 -s 32
10000111111001111000100101010010
```

256-bit hexadecimal number from `/dev/random`:
```
$ random -b 16 -s 256 -r
753a4c5e2db2057b1eeadadfb00c7f42d56395467e8f45166059f583f219e7e8
```
