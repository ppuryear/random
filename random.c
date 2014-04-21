// Copyright 2014 Philip Puryear
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
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <gmp.h>
#include "simple_strconv.h"

static const int kDefaultBase = 10;
static const unsigned long kDefaultUpperBound = 1UL << 15;
static const int kMaxReads = 100;
static const char *gRNGFilename = "/dev/urandom";

static void fatal(const char *msg, ...) {
    va_list ap;
    fprintf(stderr, "random: error: ");
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void print_usage() {
    printf(
"usage: random [OPTIONS] [LOWERBOUND] [UPPERBOUND]\n"
"\n"
"Options:\n"
"  -h    show this message and exit\n"
"  -r    read from /dev/random instead of /dev/urandom\n"
"  -b N  print the result in the given base (default: %d)\n"
"  -s N  use the interval [0, 2^N)\n",
    kDefaultBase);
}

static void arg_to_mpz(mpz_t result, const char *arg) {
    char guard;
    if (gmp_sscanf(arg, "%Zi%c", result, &guard) != 1)
        fatal("invalid argument: '%s'", arg);
}

static void get_random_mpz(mpz_t result, mpz_t low, mpz_t high) {
    mpz_t range;
    mpz_init(range);
    mpz_sub(range, high, low);
    if (mpz_sgn(range) <= 0)
        fatal("upper bound must be greater than lower bound");

    // If range == 1, there's only one possible result, so return it.
    mpz_sub_ui(range, range, 1);
    if (mpz_sgn(range) == 0) {
        mpz_set(result, low);
        return;
    }

    // Allocate a buffer to hold the RNG data.
    size_t num_bits = mpz_sizeinbase(range, 2);
    size_t num_bytes = 1 + (num_bits - 1) / CHAR_BIT;
    char *random_data = malloc(num_bytes);
    if (!random_data)
        fatal("out of memory");

    int rng_fd = open(gRNGFilename, O_RDONLY);
    if (rng_fd < 0)
        fatal("could not open %s: %s", gRNGFilename, strerror(errno));

    // Read a number R in the interval [0, 2^|num_bits|) from the RNG.
    // Note that if the range L is not a power of 2, then L < 2^|num_bits|, so
    // R may be >= L by at most a factor of 2. If this happens, retry until
    // R < L.
    //
    // Strictly speaking, there is a chance that no matter how many times we
    // read, R will be >= L each time, so cap the attempts at some reasonable
    // value. For a cap of N, the chance that we'll never read a valid number
    // is at most 1/2^N, which for N=100 is less than one in a nonillion.
    int num_reads = 0;
    do {
        ssize_t num_bytes_read = read(rng_fd, random_data, num_bytes);
        if (num_bytes_read != num_bytes)
            fatal("error reading from %s: %s", gRNGFilename,
                num_bytes_read < 0 ? strerror(errno) : "not enough bytes read");

        // Chop off unnecessary leading bits.
        if (num_bits % CHAR_BIT != 0)
            random_data[0] &= (1 << (num_bits % CHAR_BIT)) - 1;

        mpz_import(result, num_bytes, 1, 1, 0, 0, random_data);
    } while (mpz_cmp(result, range) > 0 && ++num_reads < kMaxReads);
    if (num_reads == kMaxReads)
        fatal("system did not return a number within the given bounds");

    mpz_add(result, result, low);
    close(rng_fd);
    free(random_data);
    mpz_clear(range);
}

int main(int argc, char **argv) {
    mp_bitcnt_t bit_width = 0;
    int base = kDefaultBase;

    static const struct option long_options[] = {
        { "help", no_argument, NULL, 'h' },
        { "use-random", no_argument, NULL, 'r' },
        { "base", required_argument, NULL, 'b' },
        { "bit-width", required_argument, NULL, 's' },
        { NULL, 0, NULL, 0 }
    };
    for (;;) {
        int opt = getopt_long(argc, argv, "hrb:s:", long_options, NULL);
        if (opt == -1)
            break;

        switch (opt) {
        case 'r':
            gRNGFilename = "/dev/random";
            break;
        case 'b':
            if (simple_strtoi(&base, optarg, 10) < 0 ||
                (base > -2 && base < 2) || base < -36 || base > 62)
                fatal("invalid base: '%s'", optarg);
            break;
        case 's':
            if (simple_strtoul(&bit_width, optarg, 10) < 0 || bit_width == 0)
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
    if (argc > 2 || (bit_width != 0 && argc > 0))
        fatal("too many arguments");

    mpz_t low, high, result;
    mpz_inits(low, high, result, NULL);
    if (argc == 2) {
        arg_to_mpz(low, argv[0]);
        arg_to_mpz(high, argv[1]);
    } else if (argc == 1) {
        arg_to_mpz(high, argv[0]);
    } else if (bit_width != 0) {
        mpz_setbit(high, bit_width);
    } else {
        mpz_set_ui(high, kDefaultUpperBound);
    }

    get_random_mpz(result, low, high);
    mpz_out_str(stdout, base, result);
    putchar('\n');
    mpz_clears(low, high, result, NULL);
    return EXIT_SUCCESS;
}
