#include <encoded_sizes.h>
#include <fp2.h>

extern const digit_t R[NWORDS_FIELD];

// uint64_t fp2_add_count = 0;
// uint64_t fp2_mul_count = 0;
// uint64_t fp2_inv_count = 0;
// uint64_t fp2_sqr_count = 0;
// uint64_t fp2_sqrt_count = 0;

fp2_counters_t *active_fp2_counters = NULL;

/* Arithmetic modulo X^2 + 1 */

void fp2_encode(void *dst, const fp2_t *a) {
  uint8_t *buf = dst;
  fp_encode(buf, &(a->re));
  fp_encode(buf + FP_ENCODED_BYTES, &(a->im));
}

uint32_t fp2_decode(fp2_t *d, const void *src) {
  const uint8_t *buf = src;
  uint32_t re, im;

  re = fp_decode(&(d->re), buf);
  im = fp_decode(&(d->im), buf + FP_ENCODED_BYTES);
  return re & im;
}

void fp2_set(fp2_t *x, const digit_t val) {
  fp_set(x->re, val);
  fp_set(x->im, 0);
}

void fp2_set_zero(fp2_t *x) {
  fp_set(x->re, 0);
  fp_set(x->im, 0);
}

void fp2_set_one(fp2_t *x) {
  // Initialize fp_t 1
  fp_set_one(x->re);
  fp_set(x->im, 0);
}

void fp2_batched_inv(fp2_t *x, int len) {
  fp2_t *t = malloc(len * sizeof(fp2_t));
  fp2_t acc;
  fp2_set_one(&acc);

  for (int i = 0; i < len; i++) {
    fp2_copy(&t[i], &acc);
    fp2_mul(&acc, &acc, &x[i]);
  }

  fp2_inv(&acc);

  for (int i = len - 1; i >= 0; i--) {
    fp2_mul(&t[i], &t[i], &acc);
    fp2_mul(&acc, &acc, &x[i]);
    fp2_copy(&x[i], &t[i]);
  }
  free(t);
}

void fp2_set_small(fp2_t *x, const digit_t val) {
  fp_set(x->re, val);
  fp_set(x->im, 0);
}

bool fp2_is_zero(
    const fp2_t *a) { // Is a GF(p^2) element zero?
                      // Returns 1 (true) if a=0, 0 (false) otherwise

  return fp_is_zero(a->re) & fp_is_zero(a->im);
}

bool fp2_is_equal(
    const fp2_t *a,
    const fp2_t *b) { // Compare two GF(p^2) elements in constant time
                      // Returns 1 (true) if a=b, 0 (false) otherwise

  return fp_is_equal(a->re, b->re) & fp_is_equal(a->im, b->im);
}

bool fp2_is_one(const fp2_t *a) { // Is a GF(p^2) element one?
  // Returns 1 (true) if a=0, 0 (false) otherwise
  fp2_t one;
  fp2_set_one(&one);
  return fp2_is_equal(a, &one);
}

void fp2_copy(fp2_t *x, const fp2_t *y) {
  fp_copy(x->re, y->re);
  fp_copy(x->im, y->im);
}

fp2_t fp2_non_residue() { // 2 + i is a quadratic non-residue for p
  fp_t one = {0};
  fp2_t res;

  one[0] = 1;
  fp_tomont(one, one);
  fp_add(res.re, one, one);
  fp_copy(res.im, one);
  return res;
}

void fp2_add(fp2_t *x, const fp2_t *y, const fp2_t *z) {
  // if (active_fp2_counters) active_fp2_counters->add++;
  fp_add(x->re, y->re, z->re);
  fp_add(x->im, y->im, z->im);
}

void fp2_sub(fp2_t *x, const fp2_t *y, const fp2_t *z) {
  // if (active_fp2_counters) active_fp2_counters->add++;
  fp_sub(x->re, y->re, z->re);
  fp_sub(x->im, y->im, z->im);
}

void fp2_select(fp2_t *out, const fp2_t *a, const fp2_t *b, uint32_t c) {
  fp_select(out->re, a->re, b->re, c);
  fp_select(out->im, a->im, b->im, c);
}

void fp2_neg(fp2_t *x, const fp2_t *y) {
  // fp2_add_count++;
  fp_neg(x->re, y->re);
  fp_neg(x->im, y->im);
}

void fp2_mul(fp2_t *x, const fp2_t *y, const fp2_t *z) {
  // if (active_fp2_counters) active_fp2_counters->mul++;
  fp_t t0, t1;

  fp_add(t0, y->re, y->im);
  fp_add(t1, z->re, z->im);
  fp_mul(t0, t0, t1);
  fp_mul(t1, y->im, z->im);
  fp_mul(x->re, y->re, z->re);
  fp_sub(x->im, t0, t1);
  fp_sub(x->im, x->im, x->re);
  fp_sub(x->re, x->re, t1);
}

void fp2_sqr(fp2_t *x, const fp2_t *y) {
  // if (active_fp2_counters) active_fp2_counters->sqr++;
  fp_t sum, diff;

  fp_add(sum, y->re, y->im);
  fp_sub(diff, y->re, y->im);
  fp_mul(x->im, y->re, y->im);
  fp_add(x->im, x->im, x->im);
  fp_mul(x->re, sum, diff);
}

void fp2_inv(fp2_t *x) {
  // if (active_fp2_counters) active_fp2_counters->inv++;
  fp_t t0, t1;

  fp_sqr(t0, x->re);
  fp_sqr(t1, x->im);
  fp_add(t0, t0, t1);
  fp_inv(t0);
  fp_mul(x->re, x->re, t0);
  fp_mul(x->im, x->im, t0);
  fp_neg(x->im, x->im);
}

bool fp2_is_square(const fp2_t *x) {
  fp_t t0, t1;

  fp_sqr(t0, x->re);
  fp_sqr(t1, x->im);
  fp_add(t0, t0, t1);

  return fp_is_square(t0);
}

void fp2_frob(fp2_t *x, const fp2_t *y) {
  memcpy((digit_t *)x->re, (digit_t *)y->re, NWORDS_FIELD * RADIX / 8);
  fp_neg(x->im, y->im);
}

void fp2_tomont(fp2_t *x, const fp2_t *y) {
  fp_tomont(x->re, y->re);
  fp_tomont(x->im, y->im);
}

void fp2_frommont(fp2_t *x, const fp2_t *y) {
  fp_frommont(x->re, y->re);
  fp_frommont(x->im, y->im);
}

// From "Optimized One-Dimensional SQIsign Verification on Intel and
// Cortex-M4" by Aardal et al: https://eprint.iacr.org/2024/1563
void fp2_sqrt(fp2_t *x) {
  // if (active_fp2_counters) active_fp2_counters->sqrt++;
  fp_t x0, x1, t0, t1;

  // x0 = \delta = sqrt(x.re^2 + x.im^2).
  fp_sqr(x0, x->re);
  fp_sqr(x1, x->im);
  fp_add(x0, x0, x1);
  fp_sqrt(x0);
  // If x.im = 0, there is a risk of \delta = -x.re, which makes x0 = 0 below.
  // In that case, we restore the value \delta = x.re.
  fp_select(x0, x0, x->re, fp_is_zero(x->im));
  // x0 = \delta + x.re, t0 = 2 * x0.
  fp_add(x0, x0, x->re);
  fp_add(t0, x0, x0);
  // x1 = t0^(p-3)/4.
  fp_copy(x1, t0);
  fp_exp3div4(x1, x1);
  // x0 = x0 * x1, x1 = x1 * x.im, t1 = (2x0)^2.
  fp_mul(x0, x0, x1);
  fp_mul(x1, x1, x->im);
  fp_add(t1, x0, x0);
  fp_sqr(t1, t1);
  // If t1 = t0, return x0 + x1*i, otherwise x1 - x0*i.
  fp_sub(t0, t0, t1);
  uint32_t f = fp_is_zero(t0);
  fp_neg(t1, x0);
  fp_copy(t0, x1);
  fp_select(t0, t0, x0, f);
  fp_select(t1, t1, x1, f);

  // Check if t0 is zero
  uint32_t t0_is_zero = fp_is_zero(t0);
  // Check whether t0, t1 are odd
  // Note: we encode to ensure canonical representation
  uint8_t tmp_bytes[FP_ENCODED_BYTES];
  fp_encode(tmp_bytes, &t0);
  uint32_t t0_is_odd = -((uint32_t)tmp_bytes[0] & 1);
  fp_encode(tmp_bytes, &t1);
  uint32_t t1_is_odd = -((uint32_t)tmp_bytes[0] & 1);
  // We negate the output if:
  // t0 is odd, or
  // t0 is zero and t1 is odd
  uint32_t negate_output = t0_is_odd | (t0_is_zero & t1_is_odd);
  fp_neg(x0, t0);
  fp_select(x->re, t0, x0, negate_output);
  fp_neg(x0, t1);
  fp_select(x->im, t1, x0, negate_output);
}

// Lexicographic comparison of two field elements. Returns +1 if x > y, -1 if x
// < y, 0 if x = y
int fp2_cmp(fp2_t *x, fp2_t *y) {
  fp2_t a, b;
  fp2_frommont(&a, x);
  fp2_frommont(&b, y);
  for (int i = NWORDS_FIELD - 1; i >= 0; i--) {
    if (a.re[i] > b.re[i])
      return 1;
    if (a.re[i] < b.re[i])
      return -1;
  }
  for (int i = NWORDS_FIELD - 1; i >= 0; i--) {
    if (a.im[i] > b.im[i])
      return 1;
    if (a.im[i] < b.im[i])
      return -1;
  }
  return 0;
}

// exponentiation
// TODO could be improved
void fp2_pow_vartime(fp2_t *out, const fp2_t *x, const digit_t *exp,
                     const int size) {
  fp2_t acc;
  digit_t bit;

  fp2_copy(&acc, x);
  fp2_set_one(out);

  // Iterate over each word of exp
  for (int j = 0; j < size; j++) {
    // Iterate over each bit of the word
    for (int i = 0; i < RADIX; i++) {
      bit = (exp[j] >> i) & 1;
      if (bit == 1) {
        fp2_mul(out, out, &acc);
      }
      fp2_sqr(&acc, &acc);
    }
  }
}
