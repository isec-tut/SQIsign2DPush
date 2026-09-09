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
#include <klpt.h>

/* FullRepresentInteger, including the primitive retry used by its Julia callers.
 * Keep the requested norm exactly; never divide out a nontrivial content.
 * The standard order basis is (1, i, (i+j)/2, (1+ij)/2).
 */
int represent_integer(quat_alg_elem_t *gamma, const ibz_t *n_gamma,
                      const quat_alg_t *Bpoo)
{
    int found = 0;
    ibz_t target, bound, lower, w_bound, temp, rest, x, y, z, w, content;
    quat_alg_coord_t order_coords;
    ibz_init(&target); ibz_init(&bound); ibz_init(&lower);
    ibz_init(&w_bound); ibz_init(&temp); ibz_init(&rest);
    ibz_init(&x); ibz_init(&y); ibz_init(&z); ibz_init(&w); ibz_init(&content);
    quat_alg_coord_init(&order_coords);
    if (ibz_cmp(n_gamma, &ibz_const_zero) <= 0 ||
        ibz_cmp(&Bpoo->p, &ibz_const_zero) <= 0)
        goto done;

    ibz_mul_2exp(&target, n_gamma, 2);
    ibz_div(&temp, &rest, &target, &Bpoo->p);
    ibz_sqrt_floor(&bound, &temp);
    for (int trial = 0; trial < KLPT_repres_num_gamma_trial; trial++) {
        ibz_neg(&lower, &bound);
        if (!ibz_rand_interval(&z, &lower, &bound))
            goto done;
        /* Match upstream's bound sqrt(floor((4N-z^2)/p)). This deliberately
         * oversamples the norm ellipse; reject nonpositive residuals below. */
        ibz_mul(&temp, &z, &z);
        ibz_sub(&rest, &target, &temp);
        ibz_div(&rest, &w_bound, &rest, &Bpoo->p);
        ibz_sqrt_floor(&w_bound, &rest);
        ibz_neg(&lower, &w_bound);
        if (!ibz_rand_interval(&w, &lower, &w_bound))
            goto done;
        ibz_mul(&rest, &w, &w);
        ibz_add(&rest, &rest, &temp);
        ibz_mul(&rest, &rest, &Bpoo->p);
        ibz_sub(&rest, &target, &rest);
        if (!ibz_sum_of_two_squares(&x, &y, &rest, KLPT_primality_num_iter))
            continue;
        /* Use parity predicates, also for negative coordinates. */
        if (ibz_is_even(&x) != ibz_is_even(&w) ||
            ibz_is_even(&y) != ibz_is_even(&z))
            continue;
        ibz_sub(&order_coords[0], &x, &w);
        ibz_div_2exp(&order_coords[0], &order_coords[0], 1);
        ibz_sub(&order_coords[1], &y, &z);
        ibz_div_2exp(&order_coords[1], &order_coords[1], 1);
        ibz_copy(&order_coords[2], &z);
        ibz_copy(&order_coords[3], &w);
        ibz_content(&content, &order_coords);
        if (!ibz_is_one(&content))
            continue;
        ibz_mat_4x4_eval(&gamma->coord, &STANDARD_EXTREMAL_ORDER.order.basis, &order_coords);
        ibz_copy(&gamma->denom, &STANDARD_EXTREMAL_ORDER.order.denom);
        quat_alg_normalize(gamma);
        found = 1;
        break;
    }
done:
    if (!found)
        quat_alg_scalar(gamma, &ibz_const_zero, &ibz_const_one);
    ibz_finalize(&target); ibz_finalize(&bound); ibz_finalize(&lower);
    ibz_finalize(&w_bound); ibz_finalize(&temp); ibz_finalize(&rest);
    ibz_finalize(&x); ibz_finalize(&y); ibz_finalize(&z); ibz_finalize(&w); ibz_finalize(&content);
    quat_alg_coord_finalize(&order_coords);
    return found;
}
