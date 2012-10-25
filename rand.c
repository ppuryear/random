// Copyright 2012 Philip Puryear
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <gmp.h>
#include "simple_strconv.h"

#define DEFAULT_UPPER_BOUND (1 << 15)
#define MAX_TRIES 100

static void fatal(const char *msg, ...) {
    va_list ap;
    fprintf(stderr, "rand: fatal: ");
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void print_usage() {
    printf(
"usage: rand [options] [lower_bound] [upper_bound]\n"
"\n"
"rand prints an arbitrary-size random integer uniformly distributed in the\n"
"interval [lower_bound, upper_bound).\n"
"\n"
"Options:\n"
"  -h    show this message and exit\n"
"  -r    read from /dev/random instead of /dev/urandom\n"
"  -b N  print the result in the given base (see NOTES)\n"
"  -s N  use the interval [0, 2^N)\n"
"\n"
"Notes:\n"
"If only a single bound is provided, it is assumed to be the upper bound, and\n"
"the lower bound is assumed to be 0. If no bounds are provided, the range is\n"
"assumed to be [0, %d).\n"
"\n"
"The following bases are supported:\n"
"   2..36  : decimal digits, lowercase letters\n"
"  -2..-36 : decimal digits, uppercase letters\n"
"  37..62  : decimal digits, uppercase letters, lowercase letters\n",
    DEFAULT_UPPER_BOUND);
}

static void string_to_mpz(mpz_t result, const char *str) {
    char guard;
    if (gmp_sscanf(str, "%Zi%c", result, &guard) != 1)
        fatal("invalid argument: '%s'", str);
}

static void read_mpz(mpz_t result, const char *filename, size_t num_bits) {
    FILE *file = fopen(filename, "r");
    if (!file)
        fatal("could not open %s: %s", filename, strerror(errno));
    // Turn off buffering to avoid reading more data than we need.
    setbuf(file, NULL);

    size_t num_bytes = 1 + (num_bits - 1) / CHAR_BIT;
    char *bytes = (char*) malloc(num_bytes);
    if (!bytes)
        fatal("out of memory");

    if (fread(bytes, 1, num_bytes, file) != num_bytes)
        fatal("error reading from %s", filename);
    fclose(file);

    // Chop off unnecessary leading bits.
    if (num_bits % CHAR_BIT != 0)
        bytes[0] &= (1 << (num_bits % CHAR_BIT)) - 1;
    mpz_import(result, num_bytes, 1, 1, 0, 0, bytes);
    free(bytes);
}

static void get_random_mpz(mpz_t result, mpz_t low, mpz_t high,
                           const char *source) {
    // To conserve memory, repurpose |high| to be a temporary.
    mpz_sub(high, high, low);
    if (mpz_sgn(high) <= 0)
        fatal("upper bound must be strictly greater than lower bound");

    // If |high - low| == 1, there's only one possible result, so return it.
    mpz_sub_ui(high, high, 1);
    if (mpz_sgn(high) == 0) {
        mpz_set(result, low);
        return;
    }
    size_t num_bits = mpz_sizeinbase(high, 2);

    // If the range is not a power of 2, then read_mpz can return a number
    // larger than the range (by at most a factor of 2). If this happens, retry
    // until we get a valid number. This approach avoids the slight
    // non-uniformity of the simpler scale-and-truncate algorithm.
    //
    // Strictly speaking, there is a chance that we will never read a valid
    // number, so cap the attempts at some reasonable value. For a cap of N,
    // the chance that we'll never read a valid number is at most 1/2^N, which
    // for N=100 is less than one in a nonillion (assuming the generator is
    // uniform).
    int tries = 0;
    while (1) {
        read_mpz(result, source, num_bits);
        if (mpz_cmp(result, high) <= 0)
            break;
        if (++tries == MAX_TRIES) {
            fatal("system did not return a number within the given bounds"
                  " (tried %d times)", MAX_TRIES);
        }
    }
    mpz_add(result, result, low);
}

int main(int argc, char **argv) {
    const char *random_source = "/dev/urandom";
    bool use_bits = false;
    mp_bitcnt_t bits;
    int base = 10;

    static const struct option long_options[] = {
        { "help", no_argument, NULL, 'h' },
        { "use-random", no_argument, NULL, 'r' },
        { "base", required_argument, NULL, 'b' },
        { "size", required_argument, NULL, 's' },
        { NULL, 0, NULL, 0 }
    };
    while (1) {
        int c = getopt_long(argc, argv, "hrb:s:", long_options, NULL);
        if (c == -1)
            break;

        switch (c) {
        case 'r':
            random_source = "/dev/random";
            break;
        case 'b':
            if (simple_strtoi(&base, optarg, 10) < 0)
                fatal("invalid base: '%s'", optarg);
            if (base < -36 || (base > -2 && base < 2) || base > 62)
                fatal("unsupported base: '%s'", optarg);
            break;
        case 's':
            use_bits = true;
            if (simple_strtoul(&bits, optarg, 10) < 0)
                fatal("invalid bit width: '%s'", optarg);
            break;
        case 'h':
            print_usage();
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
        }
    }
    argv += optind;
    argc -= optind;
    if (argc > 2 || (use_bits && argc > 0))
        fatal("too many arguments");

    mpz_t low, high;
    mpz_inits(low, high, NULL);
    if (argc == 2) {
        string_to_mpz(low, argv[0]);
        string_to_mpz(high, argv[1]);
    } else if (argc == 1) {
        string_to_mpz(high, argv[0]);
    } else {
        if (use_bits)
            mpz_setbit(high, bits);
        else
            mpz_set_ui(high, DEFAULT_UPPER_BOUND);
    }

    mpz_t result;
    mpz_init(result);
    get_random_mpz(result, low, high, random_source);
    mpz_out_str(stdout, base, result);
    putchar('\n');
    mpz_clears(low, high, result, NULL);
    return EXIT_SUCCESS;
}
