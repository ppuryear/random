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

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <gmp.h>
#include "simple_strconv.h"

#define DEFAULT_UPPER_BOUND (1 << 16)

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
"usage: rand [-hr] [-b base] [-s bits] [lower_bound] [upper_bound]\n"
"\n"
"rand prints an arbitrary-sized random integer uniformly distributed in the\n"
"range [lower_bound, upper_bound).\n"
"\n"
"Options:\n"
"  -h, --help       show this message and exit\n"
"  -r, --use-random read from /dev/random instead of /dev/urandom\n"
"  -b N, --base=N   print the result in the given base\n"
"  -s N, --size=N   use the range [0, 2^N)\n"
"\n"
"Notes:\n"
"If only a single bound is provided, it is assumed to be the upper bound, and\n"
"the lower bound is assumed to be 0. If no bounds are provided, the range is\n"
"assumed to be [0, %d).\n", DEFAULT_UPPER_BOUND);
}

static void string_to_mpz(mpz_t result, const char *str) {
    char guard;
    if (gmp_sscanf(str, "%Zi%c", result, &guard) != 1)
        fatal("invalid argument: '%s'", str);
}

static void read_mpz(mpz_t result, const char *filename, size_t num_bytes) {
    FILE* file = fopen(filename, "r");
    if (!file)
        fatal("could not open file for reading: %s", filename);
    // Turn off buffering to avoid reading more data than we need.
    setbuf(file, NULL);

    char* bytes = malloc(num_bytes);
    if (!bytes)
        fatal("out of memory");

    if (fread(bytes, 1, num_bytes, file) != num_bytes)
        fatal("error reading from file: %s", filename);
    fclose(file);

    mpz_import(result, num_bytes, 1, 1, 0, 0, bytes);
    free(bytes);
}

static void get_random_number(mpz_t result, mpz_t low, mpz_t high,
                              const char *source) {
    mpz_t range;
    mpz_init(range);
    mpz_sub(range, high, low);
    if (mpz_sgn(range) <= 0)
        fatal("upper bound must be strictly greater than lower bound");

    // Read sufficient random bytes to fill the range.
    mpz_t range_inclusive;
    mpz_init(range_inclusive);
    mpz_sub_ui(range_inclusive, range, 1);
    size_t range_bits = mpz_sizeinbase(range_inclusive, 2);
    size_t num_bytes = 1 + (range_bits - 1) / CHAR_BIT;
    if (num_bytes == 0) {
        mpz_set(result, low);
        goto finish;
    }
    read_mpz(result, source, num_bytes);

    // We need to scale |result| to fit in the range *only if*
    // |range| != 2^(8*n), where n is an integer. Otherwise, we'll have read
    // precisely the right number of bytes out of the random source to
    // saturate the range.
    mpz_and(range_inclusive, range, range_inclusive);
    if (mpz_sgn(range_inclusive) != 0 || range_bits % CHAR_BIT != 0) {
        mpz_mul(result, result, range);
        mpz_tdiv_q_2exp(result, result, num_bytes * CHAR_BIT);
    }
    mpz_add(result, result, low);

finish:
    mpz_clears(range, range_inclusive, NULL);
}

int main(int argc, char **argv) {
    const char *random_source = "/dev/urandom";
    size_t bits = 0;
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

        int error;
        switch (c) {
        case 'r':
            random_source = "/dev/random";
            break;
        case 'b':
            error = simple_strtoi(&base, optarg, 10);
            if (error < 0 || base < -36 || (base > -2 && base < 2) || base > 62)
                fatal("invalid base: '%s'", optarg);
            break;
        case 's':
            error = simple_strtoz(&bits, optarg, 10);
            if (error < 0 || bits == 0)
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
    if (argc > 2 || (bits > 0 && argc > 0))
        fatal("too many arguments");

    mpz_t low, high;
    mpz_inits(low, high, NULL);
    if (argc == 2) {
        string_to_mpz(low, argv[0]);
        string_to_mpz(high, argv[1]);
    } else if (argc == 1) {
        string_to_mpz(high, argv[0]);
    } else {
        if (bits > 0)
            mpz_setbit(high, bits);
        else
            mpz_set_ui(high, DEFAULT_UPPER_BOUND);
    }

    mpz_t random_number;
    mpz_init(random_number);
    get_random_number(random_number, low, high, random_source);
    mpz_out_str(stdout, base, random_number);
    putchar('\n');

    mpz_clears(low, high, random_number, NULL);
    return EXIT_SUCCESS;
}
