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
```
usage: rand [options] [lower_bound] [upper_bound]

rand prints an arbitrary-size random integer uniformly distributed in the
interval [lower_bound, upper_bound).

Options:
  -h    show this message and exit
  -r    read from /dev/random instead of /dev/urandom
  -b N  print the result in the given base (see NOTES)
  -s N  use the interval [0, 2^N)

Notes:
If only a single bound is provided, it is assumed to be the upper bound, and
the lower bound is assumed to be 0. If no bounds are provided, the range is
assumed to be [0, 65536).

The following bases are supported:
   2..36  : decimal digits, lowercase letters
  -2..-36 : decimal digits, uppercase letters
  37..62  : decimal digits, uppercase letters, lowercase letters
```
