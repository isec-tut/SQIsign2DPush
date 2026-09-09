/*
 * Adapted from hiroshi-onuki/SQIsign2D-Push, commit
 * 19e2df792f1ecbf3ee33209dbdc0c1a34886955f (src/quaternion).
 *
 * MIT License
 *
 * Copyright (c) 2025 Hiroshi Onuki
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <quaternion.h>

/* The same SmallPrimes list as the Julia implementation. */
static const unsigned short small_primes[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41,
    43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
};

int ibz_sum_of_two_squares(ibz_t *x, ibz_t *y, const ibz_t *n,
                          int primality_iterations)
{
    int found = 0;
    ibz_t rest, prime, remainder, scale, a, b, s, t, real, imag;
    ibz_init(&rest); ibz_init(&prime); ibz_init(&remainder);
    ibz_init(&scale); ibz_init(&a); ibz_init(&b);
    ibz_init(&s); ibz_init(&t); ibz_init(&real); ibz_init(&imag);
    ibz_copy(&rest, n);
    ibz_set(&a, 1);
    ibz_set(&b, 0);
    if (ibz_cmp(&rest, &ibz_const_zero) <= 0)
        goto done;

    for (size_t i = 0; i < sizeof(small_primes)/sizeof(*small_primes); i++) {
        unsigned short l = small_primes[i];
        int64_t exponent = 0;
        ibz_set(&prime, l);
        while (ibz_mod_ui(&rest, l) == 0) {
            ibz_div(&rest, &remainder, &rest, &prime);
            exponent++;
        }
        /* Even valuations contribute a common scalar, including primes 3 mod 4. */
        if (exponent >= 2) {
            ibz_pow(&scale, &prime, exponent/2);
            ibz_mul(&a, &a, &scale);
            ibz_mul(&b, &b, &scale);
        }
        if (exponent & 1) {
            if (l % 4 == 3 || !ibz_cornacchia_prime(&s, &t, &ibz_const_one, &prime))
                goto done;
            ibz_mul(&real, &a, &s);
            ibz_mul(&imag, &b, &t);
            ibz_sub(&real, &real, &imag);
            ibz_mul(&imag, &a, &t);
            ibz_mul(&s, &b, &s);
            ibz_add(&imag, &imag, &s);
            ibz_copy(&a, &real);
            ibz_copy(&b, &imag);
        }
    }
    if (!ibz_is_one(&rest)) {
        if (ibz_mod_ui(&rest, 4) != 1 ||
            !ibz_probab_prime(&rest, primality_iterations) ||
            !ibz_cornacchia_prime(&s, &t, &ibz_const_one, &rest))
            goto done;
        ibz_mul(&real, &a, &s);
        ibz_mul(&imag, &b, &t);
        ibz_sub(&real, &real, &imag);
        ibz_mul(&imag, &a, &t);
        ibz_mul(&s, &b, &s);
        ibz_add(&b, &imag, &s);
        ibz_copy(&a, &real);
    }
    ibz_copy(x, &a);
    ibz_copy(y, &b);
    found = 1;
done:
    if (!found) {
        ibz_set(x, 0);
        ibz_set(y, 0);
    }
    ibz_finalize(&rest); ibz_finalize(&prime); ibz_finalize(&remainder);
    ibz_finalize(&scale); ibz_finalize(&a); ibz_finalize(&b);
    ibz_finalize(&s); ibz_finalize(&t); ibz_finalize(&real); ibz_finalize(&imag);
    return found;
}
