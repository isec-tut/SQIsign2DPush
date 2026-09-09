#include "curve_extras.h"
#include "isog.h"
#include "tedwards.h"
#include <assert.h>
#include <ec_params.h>
#include <fp.h>
#include <fp2.h>
#include <fp_constants.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

// isog_counters_t global_isog_counters = {0};

uint64_t count_ec_dlog_2 = 0;
uint64_t count_ec_dlog_2_tate_R = 0;
uint64_t count_ec_dlog_2_tate = 0;
uint64_t count_ec_dlog_3 = 0;
uint64_t count_ec_dlog_3_tate_R = 0;
uint64_t count_ec_dlog_3_tate = 0;

fp2_counters_t count_ec_dlog_2_tate_R_fp2 = {0};
fp2_counters_t count_ec_dlog_2_tate_fp2 = {0};
fp2_counters_t count_ec_dlog_3_tate_R_fp2 = {0};
fp2_counters_t count_ec_dlog_3_tate_fp2 = {0};

static void fp2_print(char *name, fp2_t const a) {
  fp2_t b;
  fp2_frommont(&b, &a);
  printf("%s0x", name);
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016llx", (unsigned long long)b.re[i]);
  printf(" + i*0x");
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016llx", (unsigned long long)b.im[i]);
  printf(", ");
}

static void point_print(char *name, ec_point_t P) {
  fp2_t a;
  if (fp2_is_zero(&P.z)) {
    printf("%s = INF, ", name);
  } else {
    fp2_copy(&a, &P.z);
    fp2_inv(&a);
    fp2_mul(&a, &a, &P.x);
    fp2_print(name, a);
  }
}

static void curve_print(char *name, ec_curve_t E) {
  fp2_t a;
  fp2_copy(&a, &E.C);
  fp2_inv(&a);
  fp2_mul(&a, &a, &E.A);
  fp2_print(name, a);
}

static void debug_print_digits(const char *label, const digit_t *a,
                               int nwords) {
  printf("%s", label);
  for (int i = nwords - 1; i >= 0; i--) {
    printf("%016llx", (unsigned long long)a[i]);
  }
  printf("\n");
}

static void debug_print_fp2(const char *label, const fp2_t *a) {
  fp2_t t;
  fp2_copy(&t, a);
  fp_frommont(t.re, t.re);
  fp_frommont(t.im, t.im);
  printf(" + i*0x");
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016llx", (unsigned long long)t.im[i]);
  printf("\n");
}

// -----------------------------------------------------------------------------

static void mp_shiftl_any(digit_t *a, unsigned int shift, unsigned int nwords) {
  unsigned int w = shift / RADIX; // RADIX=64
  unsigned int b = shift % RADIX;

  if (w) {
    for (int i = (int)nwords - 1; i >= 0; --i) {
      a[i] = (i >= (int)w) ? a[i - w] : 0;
    }
  }
  if (b) {
    digit_t carry = 0;
    for (unsigned int i = 0; i < nwords; i++) {
      digit_t newcarry = a[i] >> (RADIX - b);
      a[i] = (a[i] << b) | carry;
      carry = newcarry;
    }
  }
}

bool ec_is_zero(ec_point_t const *P) { return fp2_is_zero(&P->z); }

void ec_point_init(ec_point_t *P) {
  /* Identity element in projective Montgomery coordinates: (1:0). */
  fp2_set_one(&P->x);
  fp2_set_zero(&P->z);
}

void ec_init(ec_point_t *P) { // Initialize point as identity element (1:0)
  fp_t one = {0};

  memset((digit_t *)P, 0, NWORDS_FIELD * RADIX * 4 / 8);
  one[0] = 1;
  fp_tomont(P->x.re, one);
}

void ec_curve_init(ec_curve_t *curve) {
  fp_t one = {0};

  memset((digit_t *)curve, 0, sizeof(ec_curve_t));
  one[0] = 1;
  fp_tomont(curve->C.re, one);
  /* v2.0 uses the identity point as the uncomputed A24 sentinel. */
  fp2_set_one(&curve->A24.x);
  fp2_set_zero(&curve->A24.z);
  curve->is_A24_computed_and_normalized = false;
}

int ec_curve_init_from_A(ec_curve_t *curve, const fp2_t *A) {
  ec_curve_init(curve);
  fp2_copy(&curve->A, A);
  return ec_curve_verify_A(A);
}

void ec_set_zero(ec_point_t *P) {
  fp2_set(&(P->x), 1);
  fp2_set(&(P->z), 0);
  return;
}

void copy_point(ec_point_t *P, ec_point_t const *Q) {
  fp2_copy(&(P->x), &(Q->x));
  fp2_copy(&(P->z), &(Q->z));
}

void copy_curve(ec_curve_t *E1, ec_curve_t const *E2) {
  fp2_copy(&(E1->A), &(E2->A));
  fp2_copy(&(E1->C), &(E2->C));
  copy_point(&(E1->A24), &(E2->A24));
  E1->is_A24_computed_and_normalized = E2->is_A24_computed_and_normalized;
}

void ec_normalize_curve(ec_curve_t *E) {
  fp2_inv(&E->C);
  fp2_mul(&E->A, &E->A, &E->C);
  fp2_set_one(&E->C);
  E->is_A24_computed_and_normalized = false;
}

void ec_normalize_point(ec_point_t *P) {
  fp2_inv(&P->z);
  fp2_mul(&P->x, &P->x, &P->z);
  fp2_set_one(&(P->z));
}

void ec_neg(ec_point_t *res, const ec_point_t *P) {
  // DOES NOTHING
  copy_point(res, P);
}

/* return the *normalised* point (A+2C)/4C */
void A24_from_AC(ec_point_t *A24, ec_curve_t const *E) {
  fp2_add(&A24->z, &E->C, &E->C);
  fp2_add(&A24->x, &E->A, &A24->z);
  fp2_add(&A24->z, &A24->z, &A24->z); //(A+2C: 4C)

  // ec_normalize_point(A24);
}

/* return the *normalised* point (A+2C)/4C */
void A24_from_AC_and_normalize(ec_point_t *A24, ec_curve_t const *E) {
  A24_from_AC(A24, E);
  ec_normalize_point(A24);
}

void ec_curve_normalize_A24(ec_curve_t *E) {
  if (!E->is_A24_computed_and_normalized) {
    A24_from_AC_and_normalize(&E->A24, E);
    E->is_A24_computed_and_normalized = true;
  }
}

void xDBL(ec_point_t *Q, ec_point_t const *P, ec_point_t const *AC) {
  // This version computes the coefficient values A+2C and 4C on-the-fly
  // The curve coefficients are passed via AC = (A:C)
  fp2_t t0, t1, t2, t3;

  fp2_add(&t0, &P->x, &P->z);
  fp2_sqr(&t0, &t0);
  fp2_sub(&t1, &P->x, &P->z);
  fp2_sqr(&t1, &t1);
  fp2_sub(&t2, &t0, &t1);
  fp2_add(&t3, &AC->z, &AC->z);
  fp2_mul(&t1, &t1, &t3);
  fp2_add(&t1, &t1, &t1);
  fp2_mul(&Q->x, &t0, &t1);
  fp2_add(&t0, &t3, &AC->x);
  fp2_mul(&t0, &t0, &t2);
  fp2_add(&t0, &t0, &t1);
  fp2_mul(&Q->z, &t0, &t2);
}

void xDBLv2(ec_point_t *Q, ec_point_t const *P, ec_point_t const *A24) {
  // ISOG_MEASURE_START(s);
  // global_isog_counters.dbl_count++;
  // This version receives the coefficient value A24 = (A+2C:4C)
  fp2_t t0, t1, t2;

  fp2_add(&t0, &P->x, &P->z);
  fp2_sqr(&t0, &t0);
  fp2_sub(&t1, &P->x, &P->z);
  fp2_sqr(&t1, &t1);
  fp2_sub(&t2, &t0, &t1);
  fp2_mul(&t1, &t1, &A24->z);
  fp2_mul(&Q->x, &t0, &t1);
  fp2_mul(&t0, &t2, &A24->x);
  fp2_add(&t0, &t0, &t1);
  fp2_mul(&Q->z, &t0, &t2);
  // ISOG_MEASURE_END(&global_isog_counters.dbl_ops, s);
}

void xADD(ec_point_t *R, ec_point_t const *P, ec_point_t const *Q,
          ec_point_t const *PQ) {
  fp2_t t0, t1, t2, t3;

  fp2_add(&t0, &P->x, &P->z);
  fp2_sub(&t1, &P->x, &P->z);
  fp2_add(&t2, &Q->x, &Q->z);
  fp2_sub(&t3, &Q->x, &Q->z);
  fp2_mul(&t0, &t0, &t3);
  fp2_mul(&t1, &t1, &t2);
  fp2_add(&t2, &t0, &t1);
  fp2_sub(&t3, &t0, &t1);
  fp2_sqr(&t2, &t2);
  fp2_sqr(&t3, &t3);
  fp2_mul(&t2, &PQ->z, &t2);
  fp2_mul(&R->z, &PQ->x, &t3);
  fp2_copy(&R->x, &t2);
}

void xDBLADD(ec_point_t *R, ec_point_t *S, ec_point_t const *P,
             ec_point_t const *Q, ec_point_t const *PQ, ec_point_t const *A24) {
  // Requires precomputation of A24 = (A+2C:4C)
  fp2_t t0, t1, t2;

  fp2_add(&t0, &P->x, &P->z);
  fp2_sub(&t1, &P->x, &P->z);
  fp2_sqr(&R->x, &t0);
  fp2_sub(&t2, &Q->x, &Q->z);
  fp2_add(&S->x, &Q->x, &Q->z);
  fp2_mul(&t0, &t0, &t2);
  fp2_sqr(&R->z, &t1);
  fp2_mul(&t1, &t1, &S->x);
  fp2_sub(&t2, &R->x, &R->z);
  fp2_mul(&R->z, &R->z, &A24->z);
  fp2_mul(&R->x, &R->x, &R->z);
  fp2_mul(&S->x, &A24->x, &t2);
  fp2_sub(&S->z, &t0, &t1);
  fp2_add(&R->z, &R->z, &S->x);
  fp2_add(&S->x, &t0, &t1);
  fp2_mul(&R->z, &R->z, &t2);
  fp2_sqr(&S->z, &S->z);
  fp2_sqr(&S->x, &S->x);
  fp2_mul(&S->z, &S->z, &PQ->x);
  fp2_mul(&S->x, &S->x, &PQ->z);
}

bool is_point_equal(
    const ec_point_t *P,
    const ec_point_t
        *Q) { // Evaluate if two points in Montgomery coordinates (X:Z) are
              // equal Returns 1 (true) if P=Q, 0 (false) otherwise
  fp2_t t0, t1;

  if ((fp2_is_zero(&P->x) && fp2_is_zero(&P->z)) ||
      (fp2_is_zero(&Q->x) && fp2_is_zero(&Q->z))) {
    return fp2_is_zero(&P->x) && fp2_is_zero(&P->z) && fp2_is_zero(&Q->x) &&
           fp2_is_zero(&Q->z);
  }

  fp2_mul(&t0, &P->x, &Q->z);
  fp2_mul(&t1, &Q->x, &P->z);
  fp2_sub(&t0, &t0, &t1);
  // fp2_print("t0: ", t0);

  return fp2_is_zero(&t0);
}

void swap_points(
    ec_point_t *P, ec_point_t *Q,
    const digit_t option) { // Swap points
                            // If option = 0 then P <- P and Q <- Q, else if
                            // option = 0xFF...FF then P <- Q and Q <- P
  digit_t temp;

  for (int i = 0; i < NWORDS_FIELD; i++) {
    temp = option & (P->x.re[i] ^ Q->x.re[i]);
    P->x.re[i] = temp ^ P->x.re[i];
    Q->x.re[i] = temp ^ Q->x.re[i];
    temp = option & (P->x.im[i] ^ Q->x.im[i]);
    P->x.im[i] = temp ^ P->x.im[i];
    Q->x.im[i] = temp ^ Q->x.im[i];
    temp = option & (P->z.re[i] ^ Q->z.re[i]);
    P->z.re[i] = temp ^ P->z.re[i];
    Q->z.re[i] = temp ^ Q->z.re[i];
    temp = option & (P->z.im[i] ^ Q->z.im[i]);
    P->z.im[i] = temp ^ P->z.im[i];
    Q->z.im[i] = temp ^ Q->z.im[i];
  }
}

void xMUL(ec_point_t *Q, ec_point_t const *P, digit_t const *k,
          ec_curve_t const *curve) {
  ec_point_t R0, R1, A24;
  digit_t mask;
  unsigned int bit = 0, prevbit = 0, swap;
  const int torsion_plus_even_power = POWER_OF_2;

  // printf("xMUL\n");
  // fp2_add(&A24.x, &curve->C, &curve->C); // Precomputation of A24=(A+2C:4C)
  // fp2_add(&A24.z, &A24.x, &A24.x);
  // fp2_add(&A24.x, &A24.x, &curve->A);
  // point_print("A24: ", A24);
  // printf("\n");
  copy_point(&A24, &curve->A24);
  // point_print("A24: ", A24);
  // printf("\n");

  // R0 <- (1:0), R1 <- P
  ec_init(&R0);
  fp2_copy(&R1.x, &P->x);
  fp2_copy(&R1.z, &P->z);

  // Main loop
  for (int i = torsion_plus_even_power - 1; i >= 0; i--) {
    bit = (k[i >> LOG2RADIX] >> (i & (RADIX - 1))) & 1;
    swap = bit ^ prevbit;
    prevbit = bit;
    mask = 0 - (digit_t)swap;

    swap_points(&R0, &R1, mask);
    xDBLADD(&R0, &R1, &R0, &R1, P, &A24);
  }
  swap = 0 ^ prevbit;
  mask = 0 - (digit_t)swap;
  swap_points(&R0, &R1, mask);

  fp2_copy(&Q->x, &R0.x);
  fp2_copy(&Q->z, &R0.z);
}

void xMULv2(ec_point_t *Q, ec_point_t const *P, digit_t const *k,
            const int kbits, ec_point_t const *A24) {
  // This version receives the coefficient value A24 = (A+2C:4C)
  ec_point_t R0, R1;
  digit_t mask;
  unsigned int bit = 0, prevbit = 0, swap;

  // R0 <- (1:0), R1 <- P
  ec_init(&R0);
  fp2_copy(&R1.x, &P->x);
  fp2_copy(&R1.z, &P->z);

  // Main loop
  for (int i = kbits - 1; i >= 0; i--) {
    bit = (k[i >> LOG2RADIX] >> (i & (RADIX - 1))) & 1;
    swap = bit ^ prevbit;
    prevbit = bit;
    mask = 0 - (digit_t)swap;

    swap_points(&R0, &R1, mask);
    xDBLADD(&R0, &R1, &R0, &R1, P, A24);
  }
  swap = 0 ^ prevbit;
  mask = 0 - (digit_t)swap;
  swap_points(&R0, &R1, mask);

  fp2_copy(&Q->x, &R0.x);
  fp2_copy(&Q->z, &R0.z);
}

static digit_t digit_pow3(int L) {
  digit_t res = 1;
  for (int i = 0; i < L; i++)
    res *= 3;
  return res;
}

static void mp_mul_digit_add(digit_t *r, const digit_t *a, digit_t b, digit_t c,
                             unsigned int nwords) {
  digit_t carry = c;
  for (unsigned int i = 0; i < nwords; i++) {
    digit_t prod[2];
    MUL(prod, a[i], b);
    digit_t sum, t_carry, carry_out;
    ADDC(sum, t_carry, prod[0], carry, 0);
    r[i] = sum;
    carry = prod[1] + t_carry;
  }
}

static void mp_add(digit_t *c, const digit_t *a, const digit_t *b,
                   const unsigned int nwords) { // Multiprecision addition
  unsigned int i, carry = 0;

  for (i = 0; i < nwords; i++) {
    ADDC(c[i], carry, a[i], b[i], carry);
  }
}

static void mp_sub(
    digit_t *c, digit_t const *a, digit_t const *b,
    const unsigned int nwords) { // Multiprecision subtraction, assuming a > b
  unsigned int i, borrow = 0;

  for (i = 0; i < nwords; i++) {
    SUBC(c[i], borrow, a[i], b[i], borrow);
  }
}

void select_ct(digit_t *c, const digit_t *a, const digit_t *b,
               const digit_t mask,
               const int nwords) { // Select c <- a if mask = 0, select c <- b
                                   // if mask = 1...1

  for (int i = 0; i < nwords; i++) {
    c[i] = ((a[i] ^ b[i]) & mask) ^ a[i];
  }
}

void swap_ct(digit_t *a, digit_t *b, const digit_t option,
             const int nwords) { // Swap entries
                                 // If option = 0 then P <- P and Q <- Q, else
                                 // if option = 0xFF...FF then a <- b and b <- a
  digit_t temp;

  for (int i = 0; i < nwords; i++) {
    temp = option & (a[i] ^ b[i]);
    a[i] = temp ^ a[i];
    b[i] = temp ^ b[i];
  }
}

// Compute S = k*P + l*Q, with PQ = P+Q
void xDBLMUL(ec_point_t *S, ec_point_t const *P, digit_t const *k,
             ec_point_t const *Q, digit_t const *l, ec_point_t const *PQ,
             ec_curve_t const *curve) {
  const int torsion_plus_even_power = POWER_OF_2;
  int i;
  digit_t evens, mevens, bitk0, bitl0, maskk, maskl, temp, bs1_ip1, bs2_ip1,
      bs1_i, bs2_i, h;
  digit_t sigma[2] = {0}, pre_sigma = 0;
  digit_t k_t[NWORDS_ORDER], l_t[NWORDS_ORDER], one[NWORDS_ORDER] = {0};
  digit_t r[2 * POWER_OF_2] = {0};
  ec_point_t A24, DIFF1a, DIFF1b, DIFF2a, DIFF2b, R[3] = {0}, T[3];

  assert(torsion_plus_even_power > 0);

  // Derive sigma according to parity
  bitk0 = (k[0] & 1);
  bitl0 = (l[0] & 1);
  maskk = 0 - bitk0; // Parity masks: 0 if even, otherwise 1...1
  maskl = 0 - bitl0;
  sigma[0] = (bitk0 ^ 1);
  sigma[1] = (bitl0 ^ 1);
  evens = sigma[0] + sigma[1]; // Count number of even scalars
  mevens = 0 - (evens & 1);    // Mask mevens <- 0 if # even scalars = 0 or 2,
                               // otherwise mevens = 1...1

  // If k and l are both even or both odd, pick sigma = (0,1)
  sigma[0] = (sigma[0] & mevens);
  sigma[1] = (sigma[1] & mevens) | (1 & ~mevens);

  // Convert even scalars to odd
  one[0] = 1;
  mp_sub(k_t, k, one, NWORDS_ORDER);
  mp_sub(l_t, l, one, NWORDS_ORDER);
  select_ct(k_t, k_t, k, maskk, NWORDS_ORDER);
  select_ct(l_t, l_t, l, maskl, NWORDS_ORDER);

  // Scalar recoding
  for (i = 0; i < torsion_plus_even_power; i++) {
    // If sigma[0] = 1 swap k_t and l_t
    maskk = 0 - (sigma[0] ^ pre_sigma);
    swap_ct(k_t, l_t, maskk, NWORDS_ORDER);

    if (i == torsion_plus_even_power - 1) {
      bs1_ip1 = 0;
      bs2_ip1 = 0;
    } else {
      bs1_ip1 = mp_shiftr(k_t, 1, NWORDS_ORDER);
      bs2_ip1 = mp_shiftr(l_t, 1, NWORDS_ORDER);
    }
    bs1_i = k_t[0] & 1;
    bs2_i = l_t[0] & 1;

    r[2 * i] = bs1_i ^ bs1_ip1;
    r[2 * i + 1] = bs2_i ^ bs2_ip1;

    // Revert sigma if second bit, r_(2i+1), is 1
    pre_sigma = sigma[0];
    maskk = 0 - r[2 * i + 1];
    select_ct(&temp, &sigma[0], &sigma[1], maskk, 1);
    select_ct(&sigma[1], &sigma[1], &sigma[0], maskk, 1);
    sigma[0] = temp;
  }

  // Point initialization
  ec_init(&R[0]);
  maskk = 0 - sigma[0];
  select_ct((digit_t *)&R[1], (digit_t *)P, (digit_t *)Q, maskk,
            4 * NWORDS_FIELD);
  select_ct((digit_t *)&R[2], (digit_t *)Q, (digit_t *)P, maskk,
            4 * NWORDS_FIELD);
  fp2_copy(&DIFF1a.x, &R[1].x);
  fp2_copy(&DIFF1a.z, &R[1].z);
  fp2_copy(&DIFF1b.x, &R[2].x);
  fp2_copy(&DIFF1b.z, &R[2].z);

  // Initialize DIFF2a <- P+Q, DIFF2b <- P-Q
  xADD(&R[2], &R[1], &R[2], PQ);
  fp2_copy(&DIFF2a.x, &R[2].x);
  fp2_copy(&DIFF2a.z, &R[2].z);
  fp2_copy(&DIFF2b.x, &PQ->x);
  fp2_copy(&DIFF2b.z, &PQ->z);

  // printf("xDBLMUL\n");
  // fp2_add(&A24.x, &curve->C, &curve->C); // Precomputation of A24=(A+2C:4C)
  // fp2_add(&A24.z, &A24.x, &A24.x);
  // fp2_add(&A24.x, &A24.x, &curve->A);
  // point_print("A24: ", A24);
  // printf("\n");
  copy_point(&A24, &curve->A24);
  // point_print("A24: ", A24);
  // printf("\n");

  // Main loop
  for (i = torsion_plus_even_power - 1; i >= 0; i--) {
    h = r[2 * i] + r[2 * i + 1]; // in {0, 1, 2}
    maskk = 0 - (h & 1);
    select_ct((digit_t *)&T[0], (digit_t *)&R[0], (digit_t *)&R[1], maskk,
              4 * NWORDS_FIELD);
    maskk = 0 - (h >> 1);
    select_ct((digit_t *)&T[0], (digit_t *)&T[0], (digit_t *)&R[2], maskk,
              4 * NWORDS_FIELD);
    xDBLv2(&T[0], &T[0], &A24);

    maskk = 0 - r[2 * i + 1]; // in {0, 1}
    select_ct((digit_t *)&T[1], (digit_t *)&R[0], (digit_t *)&R[1], maskk,
              4 * NWORDS_FIELD);
    select_ct((digit_t *)&T[2], (digit_t *)&R[1], (digit_t *)&R[2], maskk,
              4 * NWORDS_FIELD);
    swap_points(&DIFF1a, &DIFF1b, maskk);
    xADD(&T[1], &T[1], &T[2], &DIFF1a);
    xADD(&T[2], &R[0], &R[2], &DIFF2a);

    // If hw (mod 2) = 1 then swap DIFF2a and DIFF2b
    maskk = 0 - (h & 1);
    swap_points(&DIFF2a, &DIFF2b, maskk);

    // R <- T
    memcpy((digit_t *)&R[0], (digit_t *)&T[0], NWORDS_FIELD * RADIX * 4 / 8);
    memcpy((digit_t *)&R[1], (digit_t *)&T[1], NWORDS_FIELD * RADIX * 4 / 8);
    memcpy((digit_t *)&R[2], (digit_t *)&T[2], NWORDS_FIELD * RADIX * 4 / 8);
  }

  // Output R[evens]
  select_ct((digit_t *)S, (digit_t *)&R[0], (digit_t *)&R[1], mevens,
            4 * NWORDS_FIELD);
  maskk = 0 - (bitk0 & bitl0);
  select_ct((digit_t *)S, (digit_t *)S, (digit_t *)&R[2], maskk,
            4 * NWORDS_FIELD);
}

void ec_ladder3pt(ec_point_t *R, fp_t const m, ec_point_t const *P,
                  ec_point_t const *Q, ec_point_t const *PQ,
                  ec_curve_t const *A) {
  // Curve constant in the form A24=(A+2C:4C)
  ec_point_t A24;
  // printf("ec_ladder3pt\n");
  // fp2_add(&A24.x, &A->C, &A->C); // Precomputation of A24=(A+2C:4C)
  // fp2_add(&A24.z, &A24.x, &A24.x);
  // fp2_add(&A24.x, &A24.x, &A->A);
  // point_print("A24: ", A24);
  // printf("\n");
  copy_point(&A24, &A->A24);
  // point_print("A24: ", A24);
  // printf("\n");

  ec_point_t X0, X1, X2;
  copy_point(&X0, Q);
  copy_point(&X1, P);
  copy_point(&X2, PQ);

  int i, j;
  uint64_t t;
  for (i = 0; i < NWORDS_FIELD; i++) {
    t = 1;
    for (j = 0; j < 64; j++) {
      swap_points(&X1, &X2, -((t & m[i]) == 0));
      xDBLADD(&X0, &X1, &X0, &X1, &X2, &A24);
      swap_points(&X1, &X2, -((t & m[i]) == 0));
      t <<= 1;
    };
  };
  copy_point(R, &X1);
}

void ec_j_inv(fp2_t *j_inv, const ec_curve_t *curve) {
  /* j-invariant computation for montgommery coefficient A2=(A+2C:4C) */
  fp2_t t0, t1;

  fp2_sqr(&t1, &curve->C);
  fp2_sqr(j_inv, &curve->A);
  fp2_add(&t0, &t1, &t1);
  fp2_sub(&t0, j_inv, &t0);
  fp2_sub(&t0, &t0, &t1);
  fp2_sub(j_inv, &t0, &t1);
  fp2_sqr(&t1, &t1);
  fp2_mul(j_inv, j_inv, &t1);
  fp2_add(&t0, &t0, &t0);
  fp2_add(&t0, &t0, &t0);
  fp2_sqr(&t1, &t0);
  fp2_mul(&t0, &t0, &t1);
  fp2_add(&t0, &t0, &t0);
  fp2_add(&t0, &t0, &t0);
  fp2_inv(j_inv);
  fp2_mul(j_inv, &t0, j_inv);
}

void jac_init(jac_point_t *P) { // Initialize Montgomery in Jacobian coordinates
                                // as identity element (0:1:0)
  fp2_set_zero(&P->x);
  fp2_set(&P->y, 1);
  fp2_set_zero(&P->z);
}

bool is_jac_equal(
    const jac_point_t *P,
    const jac_point_t
        *Q) { // Evaluate if two points in Jacobian coordinates (X:Y:Z) are
              // equal Returns 1 (true) if P=Q, 0 (false) otherwise
  fp2_t t0, t1, t2, t3;

  fp2_sqr(&t0, &Q->z);
  fp2_mul(&t2, &P->x, &t0); // x1*z2^2
  fp2_sqr(&t1, &P->z);
  fp2_mul(&t3, &Q->x, &t1); // x2*z1^2
  fp2_sub(&t2, &t2, &t3);

  fp2_mul(&t0, &t0, &Q->z);
  fp2_mul(&t0, &P->y, &t0); // y1*z2^3
  fp2_mul(&t1, &t1, &P->z);
  fp2_mul(&t1, &Q->y, &t1); // y2*z1^3
  fp2_sub(&t0, &t0, &t1);

  return fp2_is_zero(&t0) && fp2_is_zero(&t2);
}

void jac_to_xz(ec_point_t *P, const jac_point_t *xyP) {
  fp2_copy(&P->x, &xyP->x);
  fp2_copy(&P->z, &xyP->z);
  fp2_sqr(&P->z, &P->z);

  // If xyP = (0:1:0), we currently have P=(0 : 0) but we want to set P=(1:0)
  // uint32_t c1, c2;
  // fp2_t one;
  // fp2_set(&one, 1);

  // c1 = fp2_is_zero(&P->x);
  // c2 = fp2_is_zero(&P->z);
  // fp2_select(&P->x, &P->x, &one, c1 & c2);
}

static bool is_jac_xz_equal(
    const jac_point_t *P,
    const ec_point_t
        *Q) { // Evaluate if point P in Jacobian coordinates is equal to Q in
              // homogeneous projective coordinates (X:Z) Comparison is up to
              // sign (only compares X and Z coordinates) Returns 1 (true) if
              // P=Q, 0 (false) otherwise
  fp2_t t0, t1;

  fp2_mul(&t0, &P->x, &Q->z); // x1*z2
  fp2_sqr(&t1, &P->z);
  fp2_mul(&t1, &Q->x, &t1); // x2*z1^2
  fp2_sub(&t0, &t0, &t1);

  return fp2_is_zero(&t0);
}

void jac_to_ws(jac_point_t *Q, fp2_t *t, fp2_t *ao3, const jac_point_t *P,
               const ec_curve_t *curve) {
  // Cost of 3M + 2S when A != 0.
  fp_t one;
  fp2_t a;
  /* a = 1 - A^2/3, U = X + (A*Z^2)/3, V = Y, W = Z, T = a*Z^4*/
  fp_mont_setone(one);
  if (!fp2_is_zero(&(curve->A))) {
    fp_div3(ao3->re, curve->A.re);
    fp_div3(ao3->im, curve->A.im);
    fp2_sqr(t, &P->z);
    fp2_mul(&Q->x, ao3, t);
    fp2_add(&Q->x, &Q->x, &P->x);
    fp2_sqr(t, t);
    fp2_mul(&a, ao3, &(curve->A));
    fp_sub(a.re, one, a.re);
    fp_neg(a.im, a.im);
    fp2_mul(t, t, &a);
  } else {
    fp2_copy(&Q->x, &P->x);
    fp2_sqr(t, &P->z);
    fp2_sqr(t, t);
  }
  fp2_copy(&Q->y, &P->y);
  fp2_copy(&Q->z, &P->z);
}

void jac_from_ws(jac_point_t *Q, const jac_point_t *P, const fp2_t *ao3,
                 const ec_curve_t *curve) {
  // Cost of 1M + 1S when A != 0.
  fp2_t t;
  /* X = U - (A*W^2)/3, Y = V, Z = W. */
  if (!fp2_is_zero(&(curve->A))) {
    fp2_sqr(&t, &P->z);
    fp2_mul(&t, &t, ao3);
    fp2_sub(&Q->x, &P->x, &t);
  }
  fp2_copy(&Q->y, &P->y);
  fp2_copy(&Q->z, &P->z);
}

void copy_jac_point(jac_point_t *P, const jac_point_t *Q) {
  fp2_copy(&(P->x), &(Q->x));
  fp2_copy(&(P->y), &(Q->y));
  fp2_copy(&(P->z), &(Q->z));
}

void jac_neg(jac_point_t *Q, const jac_point_t *P) {
  fp2_copy(&Q->x, &P->x);
  fp2_neg(&Q->y, &P->y);
  fp2_copy(&Q->z, &P->z);
}

void DBL(jac_point_t *Q, jac_point_t const *P, ec_curve_t const *AC) {
  // Doubling on a Montgomery curve, representation in Jacobian
  // coordinates (X:Y:Z) corresponding to (X/Z^2,Y/Z^3)
  // This version handles projective curve coefficients (A:C)
  // Formula derived for C Y^2 = C X^3 + A X^2 Z^2 + C X Z^4

  if (fp2_is_zero(&P->x) && fp2_is_zero(&P->z)) {
    jac_init(Q);
    return;
  }

  // Use curve normalized to A=1, so C=1 for simpler formulas
  ec_curve_t normalized_AC;
  if (!fp2_is_one(&(AC->C))) {
    copy_curve(&normalized_AC, AC);
    ec_normalize_curve(&normalized_AC);
    AC = &normalized_AC;
  }

  fp2_t t0, t1, t2, t3;

  fp2_sqr(&t0, &P->x); // t0 = x1^2
  fp2_add(&t1, &t0, &t0);
  fp2_add(&t0, &t0, &t1); // t0 = 3x1^2
  fp2_sqr(&t1, &P->z);    // t1 = z1^2
  fp2_mul(&t2, &P->x, &AC->A);
  fp2_add(&t2, &t2, &t2); // t2 = 2Ax1
  fp2_add(&t2, &t1, &t2); // t2 = 2Ax1+z1^2
  fp2_mul(&t2, &t1, &t2); // t2 = z1^2(2Ax1+z1^2)
  fp2_add(&t2, &t0, &t2); // t2 = alpha = 3x1^2 + z1^2(2Ax1+z1^2)
  fp2_mul(&Q->z, &P->y, &P->z);
  fp2_add(&Q->z, &Q->z, &Q->z); // z2 = 2y1z1
  fp2_sqr(&t0, &Q->z);
  fp2_mul(&t0, &t0, &AC->A); // t0 = 4Ay1^2z1^2
  fp2_sqr(&t1, &P->y);
  fp2_add(&t1, &t1, &t1);     // t1 = 2y1^2
  fp2_add(&t3, &P->x, &P->x); // t3 = 2x1
  fp2_mul(&t3, &t1, &t3);     // t3 = 4x1y1^2
  fp2_sqr(&Q->x, &t2);        // x2 = alpha^2
  fp2_sub(&Q->x, &Q->x, &t0); // x2 = alpha^2 - 4Ay1^2z1^2
  fp2_sub(&Q->x, &Q->x, &t3);
  fp2_sub(&Q->x, &Q->x, &t3); // x2 = alpha^2 - 4Ay1^2z1^2 - 8x1y1^2
  fp2_sub(&Q->y, &t3, &Q->x); // y2 = 4x1y1^2 - x2
  fp2_mul(&Q->y, &Q->y, &t2); // y2 = alpha(4x1y1^2 - x2)
  fp2_sqr(&t1, &t1);          // t1 = 4y1^4
  fp2_sub(&Q->y, &Q->y, &t1);
  fp2_sub(&Q->y, &Q->y, &t1); // y2 = alpha(4x1y1^2 - x2) - 8y1^4

  // fp2_t t0, t1, t2, t3, alpha, term1, term2;

  // // t0 = X^2
  // fp2_sqr(&t0, &P->x);
  // // t1 = Z^2
  // fp2_sqr(&t1, &P->z);

  // // alpha = 3*C*X^2 + 2*A*X*Z^2 + C*Z^4
  // fp2_add(&alpha, &t0, &t0);
  // fp2_add(&alpha, &alpha, &t0);    // 3*X^2
  // fp2_mul(&alpha, &alpha, &AC->C); // 3*C*X^2

  // fp2_mul(&t2, &P->x, &AC->A);
  // fp2_add(&t2, &t2, &t2); // 2*A*X
  // fp2_mul(&t2, &t2, &t1); // 2*A*X*Z^2
  // fp2_add(&alpha, &alpha, &t2);

  // fp2_sqr(&t2, &t1);         // Z^4
  // fp2_mul(&t2, &t2, &AC->C); // C*Z^4
  // fp2_add(&alpha, &alpha, &t2);

  // // Z3 = 2*Y*Z*C
  // fp2_mul(&Q->z, &P->y, &P->z);
  // fp2_add(&Q->z, &Q->z, &Q->z);
  // fp2_mul(&Q->z, &Q->z, &AC->C);

  // // term1 = 4*A*C*Y^2*Z^2
  // fp2_sqr(&t2, &P->y);    // Y^2
  // fp2_mul(&t3, &t2, &t1); // Y^2*Z^2
  // fp2_add(&t3, &t3, &t3);
  // fp2_add(&t3, &t3, &t3); // 4*Y^2*Z^2
  // fp2_mul(&term1, &t3, &AC->C);
  // fp2_mul(&term1, &term1, &AC->A);

  // // term2 = 4*C^2*X*Y^2 (used for X3 and Y3)
  // fp2_mul(&term2, &t2, &P->x); // X*Y^2
  // fp2_add(&term2, &term2, &term2);
  // fp2_add(&term2, &term2, &term2); // 4*X*Y^2
  // fp2_mul(&term2, &term2, &AC->C);
  // fp2_mul(&term2, &term2, &AC->C); // 4*C^2*X*Y^2

  // // X3 = alpha^2 - term1 - 2*term2
  // fp2_sqr(&Q->x, &alpha);
  // fp2_sub(&Q->x, &Q->x, &term1);
  // fp2_sub(&Q->x, &Q->x, &term2);
  // fp2_sub(&Q->x, &Q->x, &term2); // Subtract 8*C^2*X*Y^2 total

  // // Y3 = alpha * (term2 - X3) - 8*C^3*Y^4
  // fp2_sub(&Q->y, &term2, &Q->x);
  // fp2_mul(&Q->y, &Q->y, &alpha);

  // fp2_sqr(&t2, &t2); // Y^4
  // fp2_add(&t2, &t2, &t2);
  // fp2_add(&t2, &t2, &t2);
  // fp2_add(&t2, &t2, &t2); // 8*Y^4
  // fp2_mul(&t2, &t2, &AC->C);
  // fp2_mul(&t2, &t2, &AC->C);
  // fp2_mul(&t2, &t2, &AC->C); // 8*C^3*Y^4

  // fp2_sub(&Q->y, &Q->y, &t2);
}

// void DBL(jac_point_t *Q, jac_point_t const *P,
//          ec_curve_t const *
//              AC) { // Doubling on a Montgomery curve, representation in
//              Jacobian
//   // coordinates (X:Y:Z) corresponding to (X/Z^2,Y/Z^3) This
//   // version receives the coefficient value A
//   fp2_t t0, t1, t2, t3, A;

//   if (fp2_is_zero(&P->x) && fp2_is_zero(&P->z)) {
//     jac_init(Q);
//     return;
//   }
//   fp2_copy(&t0, &AC->C);
//   fp2_inv(&t0);
//   fp2_mul(&A, &AC->A, &t0);

//   fp2_sqr(&t0, &P->x); // t0 = x1^2
//   fp2_add(&t1, &t0, &t0);
//   fp2_add(&t0, &t0, &t1); // t0 = 3x1^2
//   fp2_sqr(&t1, &P->z);    // t1 = z1^2
//   fp2_mul(&t2, &P->x, &A);
//   fp2_add(&t2, &t2, &t2); // t2 = 2Ax1
//   fp2_add(&t2, &t1, &t2); // t2 = 2Ax1+z1^2
//   fp2_mul(&t2, &t1, &t2); // t2 = z1^2(2Ax1+z1^2)
//   fp2_add(&t2, &t0, &t2); // t2 = alpha = 3x1^2 + z1^2(2Ax1+z1^2)
//   fp2_mul(&Q->z, &P->y, &P->z);
//   fp2_add(&Q->z, &Q->z, &Q->z); // z2 = 2y1z1
//   fp2_sqr(&t0, &Q->z);
//   fp2_mul(&t0, &t0, &AC->A); // t0 = 4Ay1^2z1^2
//   fp2_sqr(&t1, &P->y);
//   fp2_add(&t1, &t1, &t1);     // t1 = 2y1^2
//   fp2_add(&t3, &P->x, &P->x); // t3 = 2x1
//   fp2_mul(&t3, &t1, &t3);     // t3 = 4x1y1^2
//   fp2_sqr(&Q->x, &t2);        // x2 = alpha^2
//   fp2_sub(&Q->x, &Q->x, &t0); // x2 = alpha^2 - 4Ay1^2z1^2
//   fp2_sub(&Q->x, &Q->x, &t3);
//   fp2_sub(&Q->x, &Q->x, &t3); // x2 = alpha^2 - 4Ay1^2z1^2 - 8x1y1^2
//   fp2_sub(&Q->y, &t3, &Q->x); // y2 = 4x1y1^2 - x2
//   fp2_mul(&Q->y, &Q->y, &t2); // y2 = alpha(4x1y1^2 - x2)
//   fp2_sqr(&t1, &t1);          // t1 = 4y1^4
//   fp2_sub(&Q->y, &Q->y, &t1);
//   fp2_sub(&Q->y, &Q->y, &t1); // y2 = alpha(4x1y1^2 - x2) - 8y1^4
// }

// void DBL2(jac_point_t *Q, jac_point_t const *P, ec_curve_t const *AC) {
//   // Doubling on a Montgomery curve, representation in Jacobian
//   // coordinates (X:Y:Z) corresponding to (X/Z^2,Y/Z^3)
//   // This version handles projective curve coefficients (A:C)
//   // Formula derived for C Y^2 = C X^3 + A X^2 Z^2 + C X Z^4

//   if (fp2_is_zero(&P->x) && fp2_is_zero(&P->z)) {
//     jac_init(Q);
//     return;
//   }

//   fp2_t t0, t1, t2, t3, A24, C24, xP;
//   fp2_copy(&C24, &AC->C);
//   fp2_add(&C24, &C24, &C24);
//   fp2_add(&A24, &AC->A, &C24); // A+2C
//   fp2_add(&C24, &C24, &C24);   // 4C

//   fp2_sqr(&t0, &P->x);
//   fp2_add(&t1, &t0, &t0);
//   fp2_add(&t0, &t0, &t1);
//   fp2_sqr(&t1, &P->z);
//   fp2_mul(&t2, &P->x, &A24);
//   fp2_add(&t2, &t2, &t2);
//   fp2_add(&t2, &t1, &t2);
//   fp2_mul(&t2, &t1, &t2);
//   fp2_add(&t2, &t0, &t2);

//   fp2_mul(&Q->z, &P->y, &P->z);
//   fp2_add(&Q->z, &Q->z, &Q->z);
//   fp2_sqr(&t0, &Q->z);
//   fp2_mul(&t0, &t0, &A24);
//   fp2_sqr(&t1, &P->y);
//   fp2_add(&t1, &t1, &t1);

//   fp2_copy(&xP, &P->z);
//   fp2_inv(&xP);
//   fp2_mul(&xP, &P->x, &xP);
//   fp2_add(&t3, &xP, &xP);
//   fp2_mul(&t3, &t1, &t3);

//   fp2_sqr(&Q->x, &t2);
//   fp2_sub(&Q->x, &Q->x, &t3);
//   fp2_sub(&Q->x, &Q->x, &t0);
//   fp2_sub(&Q->x, &Q->x, &t3);
//   fp2_sub(&Q->x, &Q->x, &t3);

//   fp2_sub(&Q->y, &t3, &Q->x);
//   fp2_mul(&Q->y, &Q->y, &t2);
//   fp2_sqr(&t1, &t1);
//   fp2_sub(&Q->y, &Q->y, &t1);
//   fp2_sub(&Q->y, &Q->y, &t1);
// }

void DBLW(
    jac_point_t *Q, fp2_t *u, const jac_point_t *P,
    const fp2_t
        *t) { // Cost of 3M + 5S.
              // Doubling on a Weierstrass curve, representation in modified
              // Jacobian coordinates (X:Y:Z:T=a*Z^4) corresponding to
              // (X/Z^2,Y/Z^3), where a is the curve coefficient. Formula from
              // https://hyperelliptic.org/EFD/g1p/auto-shortw-modified.html

  digit_t flag =
      (digit_t)0 - (digit_t)(fp2_is_zero(&P->x) & fp2_is_zero(&P->z));

  fp2_t xx, c, cc, r, s, m;
  // XX = X^2
  fp2_sqr(&xx, &P->x);
  // A = 2*Y^2
  fp2_sqr(&c, &P->y);
  fp2_add(&c, &c, &c);
  // AA = A^2
  fp2_sqr(&cc, &c);
  // R = 2*AA
  fp2_add(&r, &cc, &cc);
  // S = (X+A)^2-XX-AA
  fp2_add(&s, &P->x, &c);
  fp2_sqr(&s, &s);
  fp2_sub(&s, &s, &xx);
  fp2_sub(&s, &s, &cc);
  // M = 3*XX+T1
  fp2_add(&m, &xx, &xx);
  fp2_add(&m, &m, &xx);
  fp2_add(&m, &m, t);
  // X3 = M^2-2*S
  fp2_sqr(&Q->x, &m);
  fp2_sub(&Q->x, &Q->x, &s);
  fp2_sub(&Q->x, &Q->x, &s);
  // Z3 = 2*Y*Z
  fp2_mul(&Q->z, &P->y, &P->z);
  fp2_add(&Q->z, &Q->z, &Q->z);
  // Y3 = M*(S-X3)-R
  fp2_sub(&Q->y, &s, &Q->x);
  fp2_mul(&Q->y, &Q->y, &m);
  fp2_sub(&Q->y, &Q->y, &r);
  // T3 = 2*R*T1
  fp2_mul(u, t, &r);
  fp2_add(u, u, u);

  fp2_select(&Q->x, &P->x, &Q->x, flag);
  fp2_select(&Q->y, &P->y, &Q->y, flag);
  fp2_select(&Q->z, &P->z, &Q->z, flag);
}

void select_jac_point(
    jac_point_t *Q, const jac_point_t *P1, const jac_point_t *P2,
    const digit_t option) { // Select points
                            // If option = 0 then Q <- P1, else if option =
                            // 0xFF...FF then Q <- P2
  fp2_select(&(Q->x), &(P1->x), &(P2->x), option);
  fp2_select(&(Q->y), &(P1->y), &(P2->y), option);
  fp2_select(&(Q->z), &(P1->z), &(P2->z), option);
}

// void ADD(jac_point_t *R, jac_point_t const *P, jac_point_t const *Q,
//          ec_curve_t const *
//              AC) { // Addition on a Montgomery curve, representation in
//              Jacobian
//                    // coordinates (X:Y:Z) corresponding to (X/Z^2,Y/Z^3) This
//                    // version receives the coefficient value A
//   fp2_t t0, t1, t2, t3, t4, t5, t6;
//   jac_point_t T;

//   if (is_jac_equal(P, Q)) {
//     DBL(R, P, AC);
//     return;
//   }
//   jac_neg(&T, P);
//   if (is_jac_equal(&T, Q)) {
//     jac_init(R);
//     return;
//   }
//   if (fp2_is_zero(&P->x) && fp2_is_zero(&P->z)) {
//     copy_jac_point(R, Q);
//     return;
//   } else if (fp2_is_zero(&Q->x) && fp2_is_zero(&Q->z)) {
//     copy_jac_point(R, P);
//     return;
//   }

//   fp2_sqr(&t0, &P->z);      // t0 = z1^2
//   fp2_mul(&t1, &t0, &P->z); // t1 = z1^3
//   fp2_sqr(&t2, &Q->z);      // t2 = z2^2
//   fp2_mul(&t3, &t2, &Q->z); // t3 = z2^3
//   fp2_mul(&t1, &t1, &Q->y); // t1 = y2z1^3
//   fp2_mul(&t3, &t3, &P->y); // t3 = y1z2^3
//   fp2_sub(&t1, &t1, &t3);   // t1 = lambda1 = y2z1^3 - y1z2^3

//   // DEBUG PRINT
//   // printf("DEBUG ADD: P.y=");
//   // fp2_print("", P->y);
//   // printf("\n");
//   // printf("DEBUG ADD: Q.y=");
//   // fp2_print("", Q->y);
//   // printf("\n");
//   // printf("DEBUG ADD: t1(dy)=");
//   // fp2_print("", t1);
//   // printf("\n");

//   fp2_mul(&t0, &t0, &Q->x);      // t0 = x2z1^2
//   fp2_mul(&t2, &t2, &P->x);      // t2 = x1z2^2
//   fp2_sub(&t4, &t0, &t2);        // t4 = lambda3 = x2z1^2 - x1z2^2
//   fp2_add(&t0, &t0, &t2);        // t0 = lambda2 = x2z1^2 + x1z2^2
//   fp2_mul(&t5, &P->z, &Q->z);    // t5 = z1z2
//   fp2_mul(&R->z, &t4, &t5);      // z3 = z1z2*lambda3
//   fp2_mul(&R->z, &R->z, &AC->C); // Scale z3 by C

//   fp2_sqr(&t5, &t5);         // t5 = z1^2z2^2
//   fp2_mul(&t5, &AC->A, &t5); // t5 = Az1^2z2^2
//   fp2_mul(&t0, &t0, &AC->C); // t0 = C*lambda2
//   fp2_add(&t0, &t0, &t5);    // t0 = Az1^2z2^2 + C*lambda2
//   fp2_sqr(&t6, &t4);         // t6 = lambda3^2
//   fp2_mul(&t5, &t0, &t6);    // t5 = lambda3^2(Az1^2z2^2 + C*lambda2)
//   fp2_mul(&t5, &t5, &AC->C); // Scale t5 by C (total C^2 scaling for X3
//   terms)

//   fp2_sqr(&R->x, &t1); // x3 = lambda1^2
//   fp2_mul(&R->x, &R->x, &AC->C);
//   fp2_mul(&R->x, &R->x, &AC->C); // Scale x3 (part 1) by C^2
//   fp2_sub(&R->x, &R->x, &t5);    // x3 = C^2*lambda1^2 - C*lambda3^2(...)

//   fp2_mul(&t3, &t3, &t4); // t3 = y1z2^3*lambda3
//   fp2_mul(&t3, &t3, &t6); // t3 = y1z2^3*lambda3^3
//   // Scale t3 by C^3
//   fp2_mul(&t3, &t3, &AC->C);
//   fp2_mul(&t3, &t3, &AC->C);
//   fp2_mul(&t3, &t3, &AC->C);

//   fp2_mul(&t2, &t2, &t6); // t2 = x1z2^2*lambda3^2
//   // Scale t2 by C^2
//   fp2_mul(&t2, &t2, &AC->C);
//   fp2_mul(&t2, &t2, &AC->C);

//   fp2_sub(&R->y, &t2, &R->x);    // y3 = x1z2^2*lambda3^2 - x3
//   fp2_mul(&R->y, &R->y, &t1);    // y3 = lambda1(...)
//   fp2_mul(&R->y, &R->y, &AC->C); // Scale by C (total C^3)
//   fp2_sub(&R->y, &R->y, &t3);    // y3 = ... - y1z2^3*lambda3^3
// }

void ADD(jac_point_t *R, jac_point_t const *P, jac_point_t const *Q,
         ec_curve_t const *
             AC) { // Addition on a Montgomery curve, representation in Jacobian
                   // coordinates (X:Y:Z) corresponding to (X/Z^2,Y/Z^3) This
                   // version receives the coefficient value A
  fp2_t t0, t1, t2, t3, t4, t5, t6, A;
  jac_point_t T;

  if (is_jac_equal(P, Q)) {
    DBL(R, P, AC);
    return;
  }
  jac_neg(&T, P);
  if (is_jac_equal(&T, Q)) {
    jac_init(R);
    return;
  }
  if (fp2_is_zero(&P->x) && fp2_is_zero(&P->z)) {
    copy_jac_point(R, Q);
    return;
  } else if (fp2_is_zero(&Q->x) && fp2_is_zero(&Q->z)) {
    copy_jac_point(R, P);
    return;
  }

  if (!fp2_is_one(&AC->C)) {
    fp2_copy(&t0, &AC->C);
    fp2_inv(&t0);
    fp2_mul(&A, &AC->A, &t0);
  } else {
    fp2_copy(&A, &AC->A);
  }

  fp2_sqr(&t0, &P->z);        // t0 = z1^2
  fp2_mul(&t1, &t0, &P->z);   // t1 = z1^3
  fp2_sqr(&t2, &Q->z);        // t2 = z2^2
  fp2_mul(&t3, &t2, &Q->z);   // t3 = z2^3
  fp2_mul(&t1, &t1, &Q->y);   // t1 = y2z1^3
  fp2_mul(&t3, &t3, &P->y);   // t3 = y1z2^3
  fp2_sub(&t1, &t1, &t3);     // t1 = lambda1 = y2z1^3 - y1z2^3
  fp2_mul(&t0, &t0, &Q->x);   // t0 = x2z1^2
  fp2_mul(&t2, &t2, &P->x);   // t2 = x1z2^2
  fp2_sub(&t4, &t0, &t2);     // t4 = lambda3 = x2z1^2 - x1z2^2
  fp2_add(&t0, &t0, &t2);     // t0 = lambda2 = x2z1^2 + x1z2^2
  fp2_mul(&t5, &P->z, &Q->z); // t5 = z1z2
  fp2_mul(&R->z, &t4, &t5);   // z3 = z1z2*lambda3
  fp2_sqr(&t5, &t5);          // t5 = z1^2z2^2
  fp2_mul(&t5, &A, &t5);      // t5 = Az1^2z2^2
  fp2_add(&t0, &t0, &t5);     // t0 = Az1^2z2^2 + lambda2
  fp2_sqr(&t6, &t4);          // t6 = lambda3^2
  fp2_mul(&t5, &t0, &t6);     // t5 = lambda3^2(Az1^2z2^2 + lambda2)
  fp2_sqr(&R->x, &t1);        // x3 = lambda1^2
  fp2_sub(&R->x, &R->x, &t5); // x3 = lambda1^2 - lambda3^2(Az1^2z2^2 + lambda2)
  fp2_mul(&t3, &t3, &t4);     // t3 = y1z2^3*lambda3
  fp2_mul(&t3, &t3, &t6);     // t3 = y1z2^3*lambda3^3
  fp2_mul(&t2, &t2, &t6);     // t2 = x1z2^2*lambda3^2
  fp2_sub(&R->y, &t2, &R->x); // y3 = x1z2^2*lambda3^2 - x3
  fp2_mul(&R->y, &R->y, &t1); // y3 = lambda1(x1z2^2*lambda3^2 - x3)
  fp2_sub(&R->y, &R->y,
          &t3); // y3 = lambda1(x1z2^2*lambda3^2 - x3) - y1z2^3*lambda3^3
}

void jac_to_xz_add_components(add_components_t *add_comp, const jac_point_t *P,
                              const jac_point_t *Q, const ec_curve_t *AC) {
  // Take P and Q in E distinct, two jac_point_t, return three components u,v
  // and w in Fp2 such that the xz coordinates of P+Q are (u-v:w) and of P-Q are
  // (u+v:w)

  fp2_t t0, t1, t2, t3, t4, t5, t6;

  fp2_sqr(&t0, &P->z);             // t0 = z1^2
  fp2_sqr(&t1, &Q->z);             // t1 = z2^2
  fp2_mul(&t2, &P->x, &t1);        // t2 = x1z2^2
  fp2_mul(&t3, &t0, &Q->x);        // t3 = z1^2x2
  fp2_mul(&t4, &P->y, &Q->z);      // t4 = y1z2
  fp2_mul(&t4, &t4, &t1);          // t4 = y1z2^3
  fp2_mul(&t5, &P->z, &Q->y);      // t5 = z1y2
  fp2_mul(&t5, &t5, &t0);          // t5 = z1^3y2
  fp2_mul(&t0, &t0, &t1);          // t0 = (z1z2)^2
  fp2_mul(&t6, &t4, &t5);          // t6 = (z1z_2)^3y1y2
  fp2_add(&add_comp->v, &t6, &t6); // v  = 2(z1z_2)^3y1y2
  fp2_sqr(&t4, &t4);               // t4 = y1^2z2^6
  fp2_sqr(&t5, &t5);               // t5 = z1^6y_2^2
  fp2_add(&t4, &t4, &t5);          // t4 = z1^6y_2^2 + y1^2z2^6
  fp2_add(&t5, &t2, &t3);          // t5 = x1z2^2 +z_1^2x2
  fp2_add(&t6, &t3, &t3);          // t6 = 2z_1^2x2
  fp2_sub(&t6, &t5, &t6);          // t6 = lambda = x1z2^2 - z_1^2x2
  fp2_sqr(&t6, &t6);               // t6 = lambda^2 = (x1z2^2 - z_1^2x2)^2
  fp2_mul(&t1, &AC->A, &t0);       // t1 = A*(z1z2)^2
  fp2_add(&t1, &t5, &t1);          // t1 = gamma =A*(z1z2)^2 + x1z2^2 +z_1^2x2
  fp2_mul(&t1, &t1, &t6);          // t1 = gamma*lambda^2
  fp2_sub(&add_comp->u, &t4, &t1); // u  = z1^6y_2^2 + y1^2z2^6 - gamma*lambda^2
  fp2_mul(&add_comp->w, &t6, &t0); // w  = (z1z2)^2(lambda)^2
}

// void jac_to_xz_add_components(add_components_t *add_comp, const jac_point_t
// *P,
//                               const jac_point_t *Q, const ec_curve_t *AC) {
//   // Take P and Q in E distinct, two jac_point_t, return three components u,v
//   // and w in Fp2 such that the xz coordinates of P+Q are (u-v:w) and of P-Q
//   are
//   // (u+v:w)

//   fp2_t t0, t1, t2, t3, t4, t5, t6;

//   fp2_sqr(&t0, &P->z);                         // t0 = z1^2
//   fp2_sqr(&t1, &Q->z);                         // t1 = z2^2
//   fp2_mul(&t2, &P->x, &t1);                    // t2 = x1z2^2
//   fp2_mul(&t3, &t0, &Q->x);                    // t3 = z1^2x2
//   fp2_mul(&t4, &P->y, &Q->z);                  // t4 = y1z2
//   fp2_mul(&t4, &t4, &t1);                      // t4 = y1z2^3
//   fp2_mul(&t5, &P->z, &Q->y);                  // t5 = z1y2
//   fp2_mul(&t5, &t5, &t0);                      // t5 = z1^3y2
//   fp2_mul(&t0, &t0, &t1);                      // t0 = (z1z2)^2
//   fp2_mul(&t6, &t4, &t5);                      // t6 = (z1z_2)^3y1y2
//   fp2_add(&add_comp->v, &t6, &t6);             // v  = 2(z1z_2)^3y1y2
//   fp2_mul(&add_comp->v, &add_comp->v, &AC->C); // Scale v by C

//   fp2_sqr(&t4, &t4);         // t4 = y1^2z2^6
//   fp2_sqr(&t5, &t5);         // t5 = z1^6y_2^2
//   fp2_add(&t4, &t4, &t5);    // t4 = z1^6y_2^2 + y1^2z2^6
//   fp2_mul(&t4, &t4, &AC->C); // Scale t4 by C

//   fp2_add(&t5, &t2, &t3); // t5 = x1z2^2 +z_1^2x2
//   fp2_add(&t6, &t3, &t3); // t6 = 2z_1^2x2
//   fp2_sub(&t6, &t5, &t6); // t6 = lambda = x1z2^2 - z_1^2x2
//   fp2_sqr(&t6, &t6);      // t6 = lambda^2 = (x1z2^2 - z_1^2x2)^2

//   fp2_mul(&t1, &AC->A, &t0); // t1 = A*(z1z2)^2
//   fp2_mul(&t5, &t5, &AC->C); // t5 = C * (x1z2^2 +z_1^2x2)
//   fp2_add(&t1, &t5, &t1);    // t1 = gamma =A*(z1z2)^2 + C*(x1z2^2 +z_1^2x2)

//   fp2_mul(&t1, &t1, &t6); // t1 = gamma*lambda^2
//   fp2_sub(&add_comp->u, &t4,
//           &t1); // u  = C*... - gamma*lambda^2 (Total scaled by C)

//   fp2_mul(&add_comp->w, &t6, &t0);             // w  = (z1z2)^2(lambda)^2
//   fp2_mul(&add_comp->w, &add_comp->w, &AC->C); // Scale w by C
// }

void ec_mul(ec_point_t *res, const ec_curve_t *curve, const digit_t *scalar,
            const ec_point_t *P) {
  xMUL(res, P, scalar, curve);
}

void ec_biscalar_mul(ec_point_t *res, const ec_curve_t *curve,
                     const digit_t *scalarP, const digit_t *scalarQ,
                     const ec_basis_t *PQ) {
  xDBLMUL(res, &PQ->P, scalarP, &PQ->Q, scalarQ, &PQ->PmQ, curve);
}

// Helper: lift single point to Jacobian on normalized curve
static int lift_point(jac_point_t *dst, const ec_point_t *src,
                      const ec_curve_t *curve) {
  if (ec_is_zero(src)) {
    jac_init(dst);
    return 1;
  }

  fp2_t x, A_aff, y, rhs;
  fp2_t invs[2];

  fp2_copy(&invs[0], &curve->C);
  fp2_copy(&invs[1], &src->z);
  fp2_batched_inv(invs, 2);

  // A_aff = A/C
  fp2_mul(&A_aff, &curve->A, &invs[0]);

  // x = X/Z
  fp2_mul(&x, &src->x, &invs[1]);

  // y = x^3 + A_aff x^2 + x
  fp2_mul(&rhs, &x, &x);       // x^2
  fp2_mul(&y, &rhs, &x);       // x^3
  fp2_mul(&rhs, &rhs, &A_aff); // A x^2
  fp2_add(&y, &y, &rhs);       // x^3 + A x^2
  fp2_add(&y, &y, &x);         // x^3 + A x^2 + x
  assert(fp2_is_square(&y));
  // if (!fp2_is_square(&y)) {
  //   printf("lift_point: y is not a square\n");
  //   debug_print_fp2("x = ", &x);
  //   debug_print_fp2("A_aff = ", &A_aff);
  //   debug_print_fp2("y = ", &y);
  //   return 0;
  // }
  fp2_sqrt(&y);

  fp2_copy(&dst->x, &x);
  fp2_copy(&dst->y, &y);
  fp_mont_setone(dst->z.re);
  fp_set(dst->z.im, 0);
  // Debug: print dst coordinates after lift
  // printf("lift_point: dst->x = ");
  // fp2_print("", dst->x);
  // printf("\n");
  // printf("lift_point: dst->y = ");
  // fp2_print("", dst->y);
  // printf("\n");
  // printf("lift_point: dst->z = ");
  // fp2_print("", dst->z);
  // printf("\n");
  return 1;
}

void ec_recover_y(
    fp2_t *y, const fp2_t *Px,
    const ec_curve_t *curve) { // Recover y-coordinate of a point on the
                               // Montgomery curve y^2 = x^3 + Ax^2 + x
  fp2_t t0;

  fp2_sqr(&t0, Px);
  fp2_mul(y, &t0, &curve->A); // Ax^2
  fp2_add(y, y, Px);          // Ax^2 + x
  fp2_mul(&t0, &t0, Px);
  fp2_add(y, y, &t0); // x^3 + Ax^2 + x
  // This is required, because we do not yet know that our curves are
  // supersingular so our points live on the twist with B = 1.
  fp2_sqrt(y);
  return;
}

int lift_basis_normalized(jac_point_t *P, jac_point_t *Q, ec_basis_t *B,
                          ec_curve_t *E) {
  assert(fp2_is_one(&B->P.z));
  assert(fp2_is_one(&E->C));

  fp2_copy(&P->x, &B->P.x);
  fp2_copy(&Q->x, &B->Q.x);
  fp2_copy(&Q->z, &B->Q.z);
  fp2_set_one(&P->z);
  ec_recover_y(&P->y, &P->x, E);

  // Algorithm of Okeya-Sakurai to recover y.Q in the montgomery model
  fp2_t v1, v2, v3, v4;
  fp2_mul(&v1, &P->x, &Q->z);
  fp2_add(&v2, &Q->x, &v1);
  fp2_sub(&v3, &Q->x, &v1);
  fp2_sqr(&v3, &v3);
  fp2_mul(&v3, &v3, &B->PmQ.x);
  fp2_add(&v1, &E->A, &E->A);
  fp2_mul(&v1, &v1, &Q->z);
  fp2_add(&v2, &v2, &v1);
  fp2_mul(&v4, &P->x, &Q->x);
  fp2_add(&v4, &v4, &Q->z);
  fp2_mul(&v2, &v2, &v4);
  fp2_mul(&v1, &v1, &Q->z);
  fp2_sub(&v2, &v2, &v1);
  fp2_mul(&v2, &v2, &B->PmQ.z);
  fp2_sub(&Q->y, &v3, &v2);
  fp2_add(&v1, &P->y, &P->y);
  fp2_mul(&v1, &v1, &Q->z);
  fp2_mul(&v1, &v1, &B->PmQ.z);
  fp2_mul(&Q->x, &Q->x, &v1);
  fp2_mul(&Q->z, &Q->z, &v1);

  // Transforming to a jacobian coordinate
  fp2_sqr(&v1, &Q->z);
  fp2_mul(&Q->y, &Q->y, &v1);
  fp2_mul(&Q->x, &Q->x, &Q->z);
  return 1;
}

int lift_basis(jac_point_t *P, jac_point_t *Q, const ec_basis_t *B,
               ec_curve_t *E) {
  // Normalise the curve E such that (A : C) is (A/C : 1)
  // and the point x(P) = (X/Z : 1).
  fp2_t inverses[2];
  ec_basis_t normalized_B = *B;

  fp2_copy(&inverses[0], &normalized_B.P.z);
  fp2_copy(&inverses[1], &E->C);

  fp2_batched_inv(inverses, 2);
  fp2_set_one(&normalized_B.P.z);
  fp2_set_one(&E->C);

  fp2_mul(&normalized_B.P.x, &normalized_B.P.x, &inverses[0]);
  fp2_mul(&E->A, &E->A, &inverses[1]);

  // Lift the basis to Jacobian points P, Q
  return lift_basis_normalized(P, Q, &normalized_B, E);
}

// int lift_basis(jac_point_t *P, jac_point_t *Q, const ec_basis_t *B,
//                const ec_curve_t *curve) {
//   int ret;
//   ec_point_t xz_diff;
//   ec_curve_t norm;
//   add_components_t add_comp;

//   ret = lift_point(P, &B->P, curve);
//   if (!ret)
//     return 0;

//   ret = lift_point(Q, &B->Q, curve);
//   if (!ret)
//     return 0;

//   // Prepare normalized curve for internal checks
//   fp2_copy(&norm.C, &curve->C);
//   fp2_inv(&norm.C);
//   fp2_mul(&norm.A, &curve->A, &norm.C);
//   fp_mont_setone(norm.C.re);
//   fp_set(norm.C.im, 0);

//   // Compute P-Q matching B->PmQ
//   jac_to_xz_add_components(&add_comp, P, Q, &norm);
//   fp2_add(&xz_diff.x, &add_comp.u, &add_comp.v);
//   fp2_copy(&xz_diff.z, &add_comp.w);

//   if (is_point_equal(&xz_diff, &B->PmQ))
//     return 1;

//   // Try -Q (equivalent to flipping sign of Q->y)
//   fp2_sub(&xz_diff.x, &add_comp.u, &add_comp.v);

//   if (is_point_equal(&xz_diff, &B->PmQ)) {
//     jac_neg(Q, Q);
//     return 1;
//   }

//   return 0;
// }

int test_jac_order_twof(const jac_point_t *P, const ec_curve_t *curve, int f) {
  jac_point_t tmp;
  copy_jac_point(&tmp, P);

  ec_curve_t norm;
  fp2_copy(&norm.C, &curve->C);
  fp2_inv(&norm.C);
  fp2_mul(&norm.A, &curve->A, &norm.C);
  fp_mont_setone(norm.C.re);
  fp_set(norm.C.im, 0);

  // Check if zero initially? (Optional, loop handles)
  if (fp2_is_zero(&tmp.z))
    return 0;

  for (int i = 0; i < f - 1; i++) {
    DBL(&tmp, &tmp, &norm);
    if (fp2_is_zero(&tmp.z))
      return 0;
  }
  DBL(&tmp, &tmp, &norm);
  return fp2_is_zero(&tmp.z);
}

void TPL(jac_point_t *Q, jac_point_t const *P,
         ec_curve_t const *AC) { // Naive tripling on a Montgomery curve,
                                 // representation in Jacobian coordinates
                                 // (X:Y:Z) corresponding to (X/Z^2,Y/Z^3) This
                                 // version receives the coefficient value A
  // ISOG_MEASURE_START(s);
  // global_isog_counters.tpl_count++;
  jac_point_t R;

  DBL(&R, P, AC);
  ADD(Q, &R, P, AC);
  // ISOG_MEASURE_END(&global_isog_counters.tpl_ops, s);
}

void TPL_A3(ec_point_t *Q, const ec_point_t *P, const ec_point_t *A3) {
  /* -----------------------------------------------------------------------------
   * * Differential point tripling given the montgomery coefficient A3 =
   * (A+2C:A-2C)
   * -----------------------------------------------------------------------------
   */
  // ISOG_MEASURE_START(s);
  // global_isog_counters.tpl_count++;

  fp2_t t0, t1, t2, t3, t4;
  fp2_sub(&t0, &P->x, &P->z);
  fp2_sqr(&t2, &t0);
  fp2_add(&t1, &P->x, &P->z);
  fp2_sqr(&t3, &t1);
  fp2_add(&t4, &t1, &t0);
  fp2_sub(&t0, &t1, &t0);
  fp2_sqr(&t1, &t4);
  fp2_sub(&t1, &t1, &t3);
  fp2_sub(&t1, &t1, &t2);
  fp2_mul(&Q->x, &t3, &A3->x);
  fp2_mul(&t3, &Q->x, &t3);
  fp2_mul(&Q->z, &t2, &A3->z);
  fp2_mul(&t2, &t2, &Q->z);
  fp2_sub(&t3, &t2, &t3);
  fp2_sub(&t2, &Q->x, &Q->z);
  fp2_mul(&t1, &t2, &t1);
  fp2_add(&t2, &t3, &t1);
  fp2_sqr(&t2, &t2);
  fp2_mul(&Q->x, &t2, &t4);
  fp2_sub(&t1, &t3, &t1);
  fp2_sqr(&t1, &t1);
  fp2_mul(&Q->z, &t1, &t0);
  // ISOG_MEASURE_END(&global_isog_counters.tpl_ops, s);
}

void recover_y(
    fp2_t *y, fp2_t const *Px,
    ec_curve_t const *curve) { // Recover y-coordinate of a point on the
                               // Montgomery curve y^2 = x^3 + (A/C)x^2 + x
  fp2_t t0, t1;

  // Compute A_aff = A/C
  fp2_copy(&t1, &curve->C);
  fp2_inv(&t1);
  fp2_mul(&t1, &t1, &curve->A);

  fp2_sqr(&t0, Px);
  fp2_mul(y, &t0, &t1); // (A/C)x^2
  fp2_add(y, y, Px);    // (A/C)x^2 + x
  fp2_mul(&t0, &t0, Px);
  fp2_add(y, y, &t0); // x^3 + (A/C)x^2 + x
  fp2_sqrt(y);
}

static int mp_compare(digit_t *a, digit_t *b,
                      unsigned int nwords) { // Multiprecision comparison, a=b?
                                             // : (1) a>b, (0) a=b, (-1) a<b

  for (int i = nwords - 1; i >= 0; i--) {
    if (a[i] > b[i])
      return 1;
    else if (a[i] < b[i])
      return -1;
  }
  return 0;
}

static bool mp_is_zero(
    const digit_t *a,
    unsigned int nwords) { // Is a multiprecision element zero?
                           // Returns 1 (true) if a=0, 0 (false) otherwise
  digit_t r = 0;

  for (unsigned int i = 0; i < nwords; i++)
    r |= a[i] ^ 0;

  return (bool)is_digit_zero_ct(r);
}

void DBLMUL(jac_point_t *R, const jac_point_t *P, const digit_t k,
            const jac_point_t *Q, const digit_t l,
            const ec_curve_t *curve) { // Double-scalar multiplication R <- k*P
                                       // + l*Q, fixed for 64-bit scalars
  digit_t k_t, l_t;
  jac_point_t PQ;

  ADD(&PQ, P, Q, curve);
  jac_init(R);

  for (int i = 0; i < 64; i++) {
    k_t = k >> (63 - i);
    k_t &= 0x01;
    l_t = l >> (63 - i);
    l_t &= 0x01;
    DBL(R, R, curve);
    if (k_t == 1 && l_t == 1) {
      ADD(R, R, &PQ, curve);
    } else if (k_t == 1) {
      ADD(R, R, P, curve);
    } else if (l_t == 1) {
      ADD(R, R, Q, curve);
    }
  }
}

// #define SCALAR_BITS (NWORDS_ORDER * 64)

void DBLMUL2(jac_point_t *R, const jac_point_t *P, const digit_t *k,
             const jac_point_t *Q, const digit_t *l,
             const ec_curve_t *curve) { // Double-scalar multiplication R <- k*P
                                        // + l*Q
  jac_point_t PQ;

  ADD(&PQ, P, Q, curve);
  jac_init(R);

  for (int i = NWORDS_ORDER - 1; i >= 0; i--) {
    for (int j = 63; j >= 0; j--) {
      DBL(R, R, curve);
      digit_t bitk = (k[i] >> j) & 1;
      digit_t bitl = (l[i] >> j) & 1;
      if (bitk & bitl) {
        ADD(R, R, &PQ, curve);
      } else if (bitk) {
        ADD(R, R, P, curve);
      } else if (bitl) {
        ADD(R, R, Q, curve);
      }
    }
  }
}

static void mp_mul2(digit_t *c, const digit_t *a,
                    const digit_t *b) { // Multiprecision multiplication fixed
                                        // to two-digit operands
  unsigned int carry = 0;
  digit_t t0[2], t1[2], t2[2];

  MUL(t0, a[0], b[0]);
  MUL(t1, a[0], b[1]);
  ADDC(t0[1], carry, t0[1], t1[0], carry);
  ADDC(t1[1], carry, 0, t1[1], carry);
  MUL(t2, a[1], b[1]);
  ADDC(t2[0], carry, t2[0], t1[1], carry);
  ADDC(t2[1], carry, 0, t2[1], carry);
  c[0] = t0[0];
  c[1] = t0[1];
  c[2] = t2[0];
  c[3] = t2[1];
}

static void solve_order_2_dlog(int *x, int *y, const jac_point_t *R,
                               const jac_point_t *P2, const jac_point_t *Q2,
                               const jac_point_t *PQ2,
                               const ec_curve_t *curve) {
  if (fp2_is_zero(&R->z)) {
    *x = 0;
    *y = 0;
    return;
  }
  if (is_jac_equal(R, P2)) {
    *x = 1;
    *y = 0;
    return;
  }
  if (is_jac_equal(R, Q2)) {
    *x = 0;
    *y = 1;
    return;
  }
  if (is_jac_equal(R, PQ2)) {
    *x = 1;
    *y = 1;
    return;
  }
  *x = 0;
  *y = 0;
}

static void subtract_dlog_multiple_2(jac_point_t *R, int dP, int dQ,
                                     const jac_point_t *P, const jac_point_t *Q,
                                     const ec_curve_t *curve) {
  jac_point_t T;
  if (dP == 1) {
    jac_neg(&T, P);
    ADD(R, R, &T, curve);
  }
  if (dQ == 1) {
    jac_neg(&T, Q);
    ADD(R, R, &T, curve);
  }
}

static void ec_dlog_2_strategy_rec(int *digitsP, int *digitsQ, jac_point_t *R,
                                   int length, const jac_point_t *Pe2,
                                   const jac_point_t *Qe2,
                                   const jac_point_t *PQe2,
                                   const ec_curve_t *curve) {
  if (length == 1) {
    solve_order_2_dlog(&digitsP[0], &digitsQ[0], R, &Pe2[0], &Qe2[0], &PQe2[0],
                       curve);
    return;
  }
  int right_len = length / 2;
  int left_len = length - right_len;

  jac_point_t R_saved = *R;
  for (int i = 0; i < left_len; i++) {
    DBL(R, R, curve);
  }
  ec_dlog_2_strategy_rec(digitsP, digitsQ, R, right_len, Pe2, Qe2, PQe2, curve);

  *R = R_saved;
  for (int j = 0; j < right_len; j++) {
    int dP = digitsP[j];
    int dQ = digitsQ[j];
    if (dP != 0 || dQ != 0) {
      subtract_dlog_multiple_2(R, dP, dQ, &Pe2[length - 1 - j],
                               &Qe2[length - 1 - j], curve);
    }
  }
  ec_dlog_2_strategy_rec(digitsP + right_len, digitsQ + right_len, R, left_len,
                         Pe2, Qe2, PQe2, curve);
}

void ec_dlog_2(
    digit_t *scalarP, digit_t *scalarQ, const ec_basis_t *PQ2,
    const ec_point_t *R,
    const ec_curve_t *curve) { // Optimized implementation based on Montgomery
                               // formulas using Jacobian coordinates
  // count_ec_dlog_2++;
  int i;
  digit_t f;
  digit_t fp2[NWORDS_ORDER] = {0};
  jac_point_t P, Q, RR, TT;
  jac_point_t Pe2[POWER_OF_2], Qe2[POWER_OF_2], PQe2[POWER_OF_2];
  ec_point_t Rnorm;
  ec_curve_t curvenorm;
  ec_basis_t PQ2norm;

  f = POWER_OF_2;
  memset(scalarP, 0, NWORDS_ORDER * RADIX / 8);
  memset(scalarQ, 0, NWORDS_ORDER * RADIX / 8);

  // Normalize R,PQ2,curve
  // fp2_t D;
  // fp2_mul(&D, &PQ2->P.z, &PQ2->Q.z);
  // fp2_mul(&D, &D, &PQ2->PmQ.z);
  // fp2_mul(&D, &D, &R->z);
  // fp2_mul(&D, &D, &curve->C);
  // fp2_inv(&D);
  // fp_mont_setone(Rnorm.z.re);
  // fp_set(Rnorm.z.im, 0);
  // fp2_copy(&PQ2norm.P.z, &Rnorm.z);
  // fp2_copy(&PQ2norm.Q.z, &Rnorm.z);
  // fp2_copy(&PQ2norm.PmQ.z, &Rnorm.z);
  // fp2_copy(&curvenorm.C, &Rnorm.z);
  // fp2_mul(&Rnorm.x, &R->x, &D);
  // fp2_mul(&Rnorm.x, &Rnorm.x, &PQ2->P.z);
  // fp2_mul(&Rnorm.x, &Rnorm.x, &PQ2->Q.z);
  // fp2_mul(&Rnorm.x, &Rnorm.x, &PQ2->PmQ.z);
  // fp2_mul(&Rnorm.x, &Rnorm.x, &curve->C);
  // fp2_mul(&PQ2norm.P.x, &PQ2->P.x, &D);
  // fp2_mul(&PQ2norm.P.x, &PQ2norm.P.x, &R->z);
  // fp2_mul(&PQ2norm.P.x, &PQ2norm.P.x, &PQ2->Q.z);
  // fp2_mul(&PQ2norm.P.x, &PQ2norm.P.x, &PQ2->PmQ.z);
  // fp2_mul(&PQ2norm.P.x, &PQ2norm.P.x, &curve->C);
  // fp2_mul(&PQ2norm.Q.x, &PQ2->Q.x, &D);
  // fp2_mul(&PQ2norm.Q.x, &PQ2norm.Q.x, &R->z);
  // fp2_mul(&PQ2norm.Q.x, &PQ2norm.Q.x, &PQ2->P.z);
  // fp2_mul(&PQ2norm.Q.x, &PQ2norm.Q.x, &PQ2->PmQ.z);
  // fp2_mul(&PQ2norm.Q.x, &PQ2norm.Q.x, &curve->C);
  // fp2_mul(&PQ2norm.PmQ.x, &PQ2->PmQ.x, &D);
  // fp2_mul(&PQ2norm.PmQ.x, &PQ2norm.PmQ.x, &R->z);
  // fp2_mul(&PQ2norm.PmQ.x, &PQ2norm.PmQ.x, &PQ2->P.z);
  // fp2_mul(&PQ2norm.PmQ.x, &PQ2norm.PmQ.x, &PQ2->Q.z);
  // fp2_mul(&PQ2norm.PmQ.x, &PQ2norm.PmQ.x, &curve->C);
  // fp2_mul(&curvenorm.A, &curve->A, &D);
  // fp2_mul(&curvenorm.A, &curvenorm.A, &R->z);
  // fp2_mul(&curvenorm.A, &curvenorm.A, &PQ2->P.z);
  // fp2_mul(&curvenorm.A, &curvenorm.A, &PQ2->Q.z);
  // fp2_mul(&curvenorm.A, &curvenorm.A, &PQ2->PmQ.z);
  copy_point(&Rnorm, R);
  copy_point(&PQ2norm.P, &PQ2->P);
  copy_point(&PQ2norm.Q, &PQ2->Q);
  copy_point(&PQ2norm.PmQ, &PQ2->PmQ);
  copy_curve(&curvenorm, curve);
  fp2_t invs5[5];
  fp2_copy(&invs5[0], &Rnorm.z);
  fp2_copy(&invs5[1], &PQ2norm.P.z);
  fp2_copy(&invs5[2], &PQ2norm.Q.z);
  fp2_copy(&invs5[3], &PQ2norm.PmQ.z);
  fp2_copy(&invs5[4], &curvenorm.C);
  fp2_batched_inv(invs5, 5);

  fp2_mul(&Rnorm.x, &Rnorm.x, &invs5[0]);
  fp2_set_one(&Rnorm.z);
  fp2_mul(&PQ2norm.P.x, &PQ2norm.P.x, &invs5[1]);
  fp2_set_one(&PQ2norm.P.z);
  fp2_mul(&PQ2norm.Q.x, &PQ2norm.Q.x, &invs5[2]);
  fp2_set_one(&PQ2norm.Q.z);
  fp2_mul(&PQ2norm.PmQ.x, &PQ2norm.PmQ.x, &invs5[3]);
  fp2_set_one(&PQ2norm.PmQ.z);
  fp2_mul(&curvenorm.A, &curvenorm.A, &invs5[4]);
  fp2_set_one(&curvenorm.C);

  recover_y(&P.y, &PQ2norm.P.x, &curvenorm);
  fp2_copy(&P.x, &PQ2norm.P.x);
  fp2_set_one(&P.z);
  // recover_y(&Q.y, &PQ2norm.Q.x,
  //           &curvenorm); // TODO: THIS SECOND SQRT CAN BE ELIMINATED
  // fp2_copy(&Q.x, &PQ2norm.Q.x);
  // fp2_set_one(&Q.z);

  // Algorithm of Okeya-Sakurai to recover y.Q in the montgomery model

  fp2_t v1, v2, v3, v4;
  fp2_mul(&v1, &P.x, &PQ2norm.Q.z);
  fp2_add(&v2, &PQ2norm.Q.x, &v1);
  fp2_sub(&v3, &PQ2norm.Q.x, &v1);
  fp2_sqr(&v3, &v3);
  fp2_mul(&v3, &v3, &PQ2norm.PmQ.x);
  fp2_add(&v1, &curvenorm.A, &curvenorm.A);
  fp2_mul(&v1, &v1, &PQ2norm.Q.z);
  fp2_add(&v2, &v2, &v1);
  fp2_mul(&v4, &P.x, &PQ2norm.Q.x);
  fp2_add(&v4, &v4, &PQ2norm.Q.z);
  fp2_mul(&v2, &v2, &v4);
  fp2_mul(&v1, &v1, &PQ2norm.Q.z);
  fp2_sub(&v2, &v2, &v1);
  fp2_mul(&v2, &v2, &PQ2norm.PmQ.z);
  fp2_sub(&Q.y, &v3, &v2);
  fp2_add(&v1, &P.y, &P.y);
  fp2_mul(&v1, &v1, &PQ2norm.Q.z);
  fp2_mul(&v1, &v1, &PQ2norm.PmQ.z);
  fp2_mul(&Q.x, &PQ2norm.Q.x, &v1);
  fp2_mul(&Q.z, &PQ2norm.Q.z, &v1);

  // Transforming to a jacobian coordinate
  fp2_sqr(&v1, &Q.z);
  fp2_mul(&Q.y, &Q.y, &v1);
  fp2_mul(&Q.x, &Q.x, &Q.z);

  recover_y(&RR.y, &Rnorm.x, &curvenorm);
  fp2_copy(&RR.x, &Rnorm.x);
  fp2_set_one(&RR.z);

  jac_neg(&TT, &Q);
  ADD(&TT, &P, &TT, &curvenorm);
  if (!is_jac_xz_equal(&TT, &PQ2norm.PmQ))
    jac_neg(&Q, &Q);

  // Computing torsion-2^f points, multiples of P, Q and P+Q
  copy_jac_point(&Pe2[POWER_OF_2 - 1], &P);
  copy_jac_point(&Qe2[POWER_OF_2 - 1], &Q);
  ADD(&PQe2[POWER_OF_2 - 1], &P, &Q, &curvenorm); // P+Q

  for (i = 0; i < (POWER_OF_2 - 1); i++) {
    DBL(&Pe2[POWER_OF_2 - i - 2], &Pe2[POWER_OF_2 - i - 1], &curvenorm);
    DBL(&Qe2[POWER_OF_2 - i - 2], &Qe2[POWER_OF_2 - i - 1], &curvenorm);
    DBL(&PQe2[POWER_OF_2 - i - 2], &PQe2[POWER_OF_2 - i - 1], &curvenorm);
  }

  // Zero scalars
  int digitsP_arr[POWER_OF_2], digitsQ_arr[POWER_OF_2];
  memset(digitsP_arr, 0, sizeof(digitsP_arr));
  memset(digitsQ_arr, 0, sizeof(digitsQ_arr));

  ec_dlog_2_strategy_rec(digitsP_arr, digitsQ_arr, &RR, (int)f, Pe2, Qe2, PQe2,
                         &curvenorm);

  memset(scalarP, 0, NWORDS_ORDER * sizeof(digit_t));
  memset(scalarQ, 0, NWORDS_ORDER * sizeof(digit_t));
  for (int j = (int)f - 1; j >= 0; j--) {
    mp_mul_digit_add(scalarP, scalarP, 2, (digit_t)digitsP_arr[j],
                     NWORDS_ORDER);
    mp_mul_digit_add(scalarQ, scalarQ, 2, (digit_t)digitsQ_arr[j],
                     NWORDS_ORDER);
  }

  memcpy((digit_t *)fp2, (digit_t *)TWOpFm1, NWORDS_ORDER * RADIX / 8);

  if (mp_compare(scalarP, fp2, NWORDS_ORDER) == 1 ||
      (mp_compare(scalarQ, fp2, NWORDS_ORDER) == 1 &&
       (mp_is_zero(scalarP, NWORDS_ORDER) == 1 ||
        mp_compare(scalarP, fp2, NWORDS_ORDER) == 0))) {
    mp_shiftl(fp2, 1, NWORDS_ORDER); // Get 2^f
    if (mp_is_zero(scalarP, NWORDS_ORDER) != 1)
      mp_sub(scalarP, fp2, scalarP, NWORDS_ORDER);
    if (mp_is_zero(scalarQ, NWORDS_ORDER) != 1)
      mp_sub(scalarQ, fp2, scalarQ, NWORDS_ORDER);
  }
}

// Check if a point P has order 2^f (or a divisor)
int test_point_order_twof(const ec_point_t *P, const ec_curve_t *curve, int f) {
  ec_point_t Q;
  fp2_copy(&Q.x, &P->x);
  fp2_copy(&Q.z, &P->z);

  // Check if point is zero initially (order 1 dividing 2^f)
  if (ec_is_zero(&Q))
    return 1;

  // Perform f doublings
  for (int i = 0; i < f; i++) {
    ec_dbl(&Q, curve, &Q);
  }
  return ec_is_zero(&Q);
}

static void solve_order_3_dlog(int *x, int *y, const jac_point_t *R,
                               const jac_point_t *P3, const jac_point_t *Q3,
                               const jac_point_t *PQ3,
                               const ec_curve_t *curve) {
  if (fp2_is_zero(&R->z)) {
    *x = 0;
    *y = 0;
    return;
  }
  if (is_jac_equal(R, P3)) {
    *x = 1;
    *y = 0;
    return;
  }
  jac_point_t T;
  jac_neg(&T, P3);
  if (is_jac_equal(R, &T)) {
    *x = 2;
    *y = 0;
    return;
  }
  if (is_jac_equal(R, Q3)) {
    *x = 0;
    *y = 1;
    return;
  }
  jac_neg(&T, Q3);
  if (is_jac_equal(R, &T)) {
    *x = 0;
    *y = 2;
    return;
  }
  if (is_jac_equal(R, PQ3)) {
    *x = 1;
    *y = 1;
    return;
  }
  jac_neg(&T, PQ3);
  if (is_jac_equal(R, &T)) {
    *x = 2;
    *y = 2;
    return;
  }
  ADD(&T, PQ3, Q3, curve); // P + 2Q = P - Q
  if (is_jac_equal(R, &T)) {
    *x = 1;
    *y = 2;
    return;
  }
  jac_neg(&T, &T);
  if (is_jac_equal(R, &T)) {
    *x = 2;
    *y = 1;
    return;
  }
  *x = 0;
  *y = 0;
}

static void subtract_dlog_multiple(jac_point_t *R, int dP, int dQ,
                                   const jac_point_t *P, const jac_point_t *Q,
                                   const ec_curve_t *curve) {
  jac_point_t T;
  if (dP == 1) {
    jac_neg(&T, P);
    ADD(R, R, &T, curve);
  } else if (dP == 2) {
    DBL(&T, P, curve);
    jac_neg(&T, &T);
    ADD(R, R, &T, curve);
  }
  if (dQ == 1) {
    jac_neg(&T, Q);
    ADD(R, R, &T, curve);
  } else if (dQ == 2) {
    DBL(&T, Q, curve);
    jac_neg(&T, &T);
    ADD(R, R, &T, curve);
  }
}

static void ec_dlog_3_strategy_rec(int *digitsP, int *digitsQ, jac_point_t *R,
                                   int length, const jac_point_t *Pe3,
                                   const jac_point_t *Qe3,
                                   const jac_point_t *PQe3,
                                   const ec_curve_t *curve) {
  // Invariant: R = sum_{j=0}^{length-1} d_j * Pe3[length-1-j]
  // where Pe3[k] = 3^{f-1-k} * P (global), and we only use Pe3[0..length-1].
  // Pe3[0] is order-3, Pe3[length-1] is order 3^length.
  // d_j is the coefficient for 3^j in the 3-adic expansion.
  if (length == 1) {
    solve_order_3_dlog(&digitsP[0], &digitsQ[0], R, &Pe3[0], &Qe3[0], &PQe3[0],
                       curve);
    return;
  }
  int right_len = length / 2;
  int left_len = length - right_len;

  // Phase 1: Determine lower digits d_0, ..., d_{right_len-1}.
  // TPL left_len times: 3^left * R = sum_{j=0}^{right-1} d_j * Pe3[right-1-j]
  // (terms with j >= right vanish since 3^left * Pe3[length-1-j] =
  //  Pe3[length-1-j-left] for j < right, and = 0 for j >= right)
  jac_point_t R_saved = *R;
  for (int i = 0; i < left_len; i++) {
    TPL(R, R, curve);
  }
  ec_dlog_3_strategy_rec(digitsP, digitsQ, R, right_len, Pe3, Qe3, PQe3, curve);

  // Phase 2: Subtract lower digits from saved R, then find upper digits.
  *R = R_saved;
  for (int j = 0; j < right_len; j++) {
    int dP = digitsP[j];
    int dQ = digitsQ[j];
    if (dP != 0 || dQ != 0) {
      // Subtract d_j * Pe3[length-1-j] (the basis element at position j)
      subtract_dlog_multiple(R, dP, dQ, &Pe3[length - 1 - j],
                             &Qe3[length - 1 - j], curve);
    }
  }
  // Now R = sum_{j=right}^{length-1} d_j * Pe3[length-1-j]
  //       = sum_{k=0}^{left-1} d_{right+k} * Pe3[left-1-k]
  // This has the same form with length=left, using Pe3[0..left-1].
  ec_dlog_3_strategy_rec(digitsP + right_len, digitsQ + right_len, R, left_len,
                         Pe3, Qe3, PQe3, curve);
}

void ec_dlog_3(
    digit_t *scalarP, digit_t *scalarQ, const ec_basis_t *PQ3,
    const ec_point_t *R,
    const ec_curve_t *curve) { // Optimized implementation based on Strategy
  // count_ec_dlog_3++;
  int i;
  digit_t f;
  jac_point_t P, Q, RR, TT;
  jac_point_t Pe3[POWER_OF_3], Qe3[POWER_OF_3], PQe3[POWER_OF_3];
  ec_point_t Rnorm;
  ec_curve_t curvenorm;
  ec_basis_t PQ3norm;

  f = POWER_OF_3;
  memset(scalarP, 0, NWORDS_ORDER * RADIX / 8);
  memset(scalarQ, 0, NWORDS_ORDER * RADIX / 8);

  copy_point(&Rnorm, R);
  copy_point(&PQ3norm.P, &PQ3->P);
  copy_point(&PQ3norm.Q, &PQ3->Q);
  copy_point(&PQ3norm.PmQ, &PQ3->PmQ);
  copy_curve(&curvenorm, curve);
  fp2_t invs5[5];
  fp2_copy(&invs5[0], &Rnorm.z);
  fp2_copy(&invs5[1], &PQ3norm.P.z);
  fp2_copy(&invs5[2], &PQ3norm.Q.z);
  fp2_copy(&invs5[3], &PQ3norm.PmQ.z);
  fp2_copy(&invs5[4], &curvenorm.C);
  fp2_batched_inv(invs5, 5);

  fp2_mul(&Rnorm.x, &Rnorm.x, &invs5[0]);
  fp2_set_one(&Rnorm.z);
  fp2_mul(&PQ3norm.P.x, &PQ3norm.P.x, &invs5[1]);
  fp2_set_one(&PQ3norm.P.z);
  fp2_mul(&PQ3norm.Q.x, &PQ3norm.Q.x, &invs5[2]);
  fp2_set_one(&PQ3norm.Q.z);
  fp2_mul(&PQ3norm.PmQ.x, &PQ3norm.PmQ.x, &invs5[3]);
  fp2_set_one(&PQ3norm.PmQ.z);
  fp2_mul(&curvenorm.A, &curvenorm.A, &invs5[4]);
  fp2_set_one(&curvenorm.C);

  recover_y(&P.y, &PQ3norm.P.x, &curvenorm);
  fp2_copy(&P.x, &PQ3norm.P.x);
  fp2_set_one(&P.z);
  // recover_y(&Q.y, &PQ3norm.Q.x, &curvenorm);
  // fp2_copy(&Q.x, &PQ3norm.Q.x);
  // fp2_set_one(&Q.z);

  // Algorithm of Okeya-Sakurai to recover y.Q in the montgomery model
  fp2_t v1, v2, v3, v4;
  fp2_mul(&v1, &P.x, &PQ3norm.Q.z);
  fp2_add(&v2, &PQ3norm.Q.x, &v1);
  fp2_sub(&v3, &PQ3norm.Q.x, &v1);
  fp2_sqr(&v3, &v3);
  fp2_mul(&v3, &v3, &PQ3norm.PmQ.x);
  fp2_add(&v1, &curvenorm.A, &curvenorm.A);
  fp2_mul(&v1, &v1, &PQ3norm.Q.z);
  fp2_add(&v2, &v2, &v1);
  fp2_mul(&v4, &P.x, &PQ3norm.Q.x);
  fp2_add(&v4, &v4, &PQ3norm.Q.z);
  fp2_mul(&v2, &v2, &v4);
  fp2_mul(&v1, &v1, &PQ3norm.Q.z);
  fp2_sub(&v2, &v2, &v1);
  fp2_mul(&v2, &v2, &PQ3norm.PmQ.z);
  fp2_sub(&Q.y, &v3, &v2);
  fp2_add(&v1, &P.y, &P.y);
  fp2_mul(&v1, &v1, &PQ3norm.Q.z);
  fp2_mul(&v1, &v1, &PQ3norm.PmQ.z);
  fp2_mul(&Q.x, &PQ3norm.Q.x, &v1);
  fp2_mul(&Q.z, &PQ3norm.Q.z, &v1);

  // Transforming to a jacobian coordinate
  fp2_sqr(&v1, &Q.z);
  fp2_mul(&Q.y, &Q.y, &v1);
  fp2_mul(&Q.x, &Q.x, &Q.z);

  recover_y(&RR.y, &Rnorm.x, &curvenorm);
  fp2_copy(&RR.x, &Rnorm.x);
  fp2_set_one(&RR.z);

  jac_neg(&TT, &Q);
  ADD(&TT, &P, &TT, &curvenorm);
  if (!is_jac_xz_equal(&TT, &PQ3norm.PmQ))
    jac_neg(&Q, &Q);

  copy_jac_point(&Pe3[POWER_OF_3 - 1], &P);
  copy_jac_point(&Qe3[POWER_OF_3 - 1], &Q);
  ADD(&PQe3[POWER_OF_3 - 1], &P, &Q, &curvenorm);

  for (i = 0; i < (POWER_OF_3 - 1); i++) {
    TPL(&Pe3[POWER_OF_3 - i - 2], &Pe3[POWER_OF_3 - i - 1], &curvenorm);
    TPL(&Qe3[POWER_OF_3 - i - 2], &Qe3[POWER_OF_3 - i - 1], &curvenorm);
    TPL(&PQe3[POWER_OF_3 - i - 2], &PQe3[POWER_OF_3 - i - 1], &curvenorm);
  }

  int digitsP_arr[POWER_OF_3], digitsQ_arr[POWER_OF_3];
  memset(digitsP_arr, 0, sizeof(digitsP_arr));
  memset(digitsQ_arr, 0, sizeof(digitsQ_arr));

  // clock_t start = clock();
  ec_dlog_3_strategy_rec(digitsP_arr, digitsQ_arr, &RR, (int)f, Pe3, Qe3, PQe3,
                         &curvenorm);
  // clock_t end = clock();
  // printf("ec_dlog_3_strategy_rec: %f ms\n",
  //        (double)(end - start) * 1000 / CLOCKS_PER_SEC);

  memset(scalarP, 0, NWORDS_ORDER * sizeof(digit_t));
  memset(scalarQ, 0, NWORDS_ORDER * sizeof(digit_t));
  for (int j = (int)f - 1; j >= 0; j--) {
    mp_mul_digit_add(scalarP, scalarP, 3, (digit_t)digitsP_arr[j],
                     NWORDS_ORDER);
    mp_mul_digit_add(scalarQ, scalarQ, 3, (digit_t)digitsQ_arr[j],
                     NWORDS_ORDER);
  }

  if (mp_compare(scalarP, THREEpFdiv2, NWORDS_ORDER) == 1 ||
      (mp_compare(scalarQ, THREEpFdiv2, NWORDS_ORDER) == 1 &&
       (mp_is_zero(scalarP, NWORDS_ORDER) == 1))) {
    if (mp_is_zero(scalarP, NWORDS_ORDER) != 1)
      mp_sub(scalarP, THREEpF, scalarP, NWORDS_ORDER);
    if (mp_is_zero(scalarQ, NWORDS_ORDER) != 1)
      mp_sub(scalarQ, THREEpF, scalarQ, NWORDS_ORDER);
  }
}

// WRAPPERS

void ec_dbl(ec_point_t *res, const ec_curve_t *curve, const ec_point_t *P) {
  xDBL(res, P, (ec_point_t const *)curve);
}

void ec_random_point_order_3(ec_point_t *P, const ec_curve_t *curve) {
  fp2_t x, t0, t1, t2;
  ec_point_t P3, A24, A3;

  // Curve coefficient in the form A24 = (A+2C:4C)
  // printf("ec_random_point_order_3\n");
  // fp2_add(&A24.x, &curve->C, &curve->C); // Precomputation of A24=(A+2C:4C)
  // fp2_add(&A24.z, &A24.x, &A24.x);
  // fp2_add(&A24.x, &A24.x, &curve->A);
  // point_print("A24: ", A24);
  // printf("\n");
  copy_point(&A24, &curve->A24);
  // point_print("A24: ", A24);
  // printf("\n");

  // Curve coefficient in the form A3 = (A+2C:A-2C)
  fp2_sub(&A3.z, &A24.x, &A24.z);
  fp2_copy(&A3.x, &A24.x);

  fp2_set_one(&x);
  // Find P
  while (1) {
    fp_add(x.im, x.re, x.im);

    // Check if point is rational
    fp2_sqr(&t0, &curve->C);
    fp2_mul(&t1, &t0, &x);
    fp2_mul(&t2, &curve->A, &curve->C);
    fp2_add(&t1, &t1, &t2);
    fp2_mul(&t1, &t1, &x);
    fp2_add(&t1, &t1, &t0);
    fp2_mul(&t1, &t1, &x);
    if (fp2_is_square(&t1)) {
      fp2_copy(&P->x, &x);
      fp2_set_one(&P->z);
    } else
      continue;

    // Clear non-3 factors from the order
    xMULv2(P, P, p_cofactor_for_3g, (int)P_COFACTOR_FOR_3G_BITLENGTH, &A24);

    // Check if point has order 3^g
    copy_point(&P3, P);
    for (int i = 0; i < POWER_OF_3 - 1; i++)
      TPL_A3(&P3, &P3, &A3);
    if (ec_is_zero(&P3))
      continue;
    else {
      copy_point(P, &P3);
      break;
    }
  }
}

// this is exactly like xDBLv2, except we use the fact that P is normalised
// to gain a multiplication
// Warning: for now we need to assume that A24 is normalised, ie C24=1.
// (maybe add an assert?)
void cubicalDBL(ec_point_t *Q, ec_point_t const *P, ec_point_t const *A24) {
  // A24 = (A+2C:4C)
  fp2_t t0, t1, t2;

  assert(fp2_is_one(&A24->z));
  fp2_add(&t0, &P->x, &P->z);
  fp2_sqr(&t0, &t0);
  fp2_sub(&t1, &P->x, &P->z);
  fp2_sqr(&t1, &t1);
  fp2_sub(&t2, &t0, &t1);
  // fp2_mul(&t1, &t1, &A24->z);
  fp2_mul(&Q->x, &t0, &t1);
  fp2_mul(&t0, &t2, &A24->x);
  fp2_add(&t0, &t0, &t1);
  fp2_mul(&Q->z, &t0, &t2);
}

// this would be exactly like xADD if PQ was 'antinormalised' as (1,z)
void cubicalADD(ec_point_t *R, ec_point_t const *P, ec_point_t const *Q,
                fp2_t const *ixPQ) {
  fp2_t t0, t1, t2, t3;

  fp2_add(&t0, &P->x, &P->z);
  fp2_sub(&t1, &P->x, &P->z);
  fp2_add(&t2, &Q->x, &Q->z);
  fp2_sub(&t3, &Q->x, &Q->z);
  fp2_mul(&t0, &t0, &t3);
  fp2_mul(&t1, &t1, &t2);
  fp2_add(&t2, &t0, &t1);
  fp2_sub(&t3, &t0, &t1);
  fp2_sqr(&R->z, &t3);
  fp2_sqr(&t2, &t2);
  fp2_mul(&R->x, ixPQ, &t2);
}

// given cubical reps of P+Q, Q, P, return P+2Q, 2Q
void biextDBL(ec_point_t *PQQ, ec_point_t *QQ, ec_point_t const *PQ,
              ec_point_t const *Q, fp2_t const *ixP, ec_point_t const *A24) {
  cubicalADD(PQQ, PQ, Q, ixP);
  cubicalDBL(QQ, Q, A24);
}

// iterative biextension doubling
void biext_ladder_2e(uint64_t e, ec_point_t *PnQ, ec_point_t *nQ,
                     ec_point_t const *PQ, ec_point_t const *Q,
                     fp2_t const *ixP, ec_point_t const *A24) {
  copy_point(PnQ, PQ);
  copy_point(nQ, Q);
  for (uint64_t i = 0; i < e; i++) {
    biextDBL(PnQ, nQ, PnQ, nQ, ixP, A24);
  }
}

// compute the monodromy ratio of cubical points [(P+nQ)/P] / [(nQ)/0]
// (not used)
void ratio(fp2_t *r, ec_point_t const *PnQ, ec_point_t const *nQ,
           ec_point_t const *P) {
  // Sanity tests
  assert(ec_is_zero(nQ));
  assert(is_point_equal(PnQ, P));

  fp2_mul(r, &nQ->x, &P->x);
  fp2_inv(r);
  fp2_mul(r, r, &PnQ->x);
}

// Compute the ratio X/Z above as a (X:Z) point to avoid a division
void point_ratio(ec_point_t *R, ec_point_t const *PnQ, ec_point_t const *nQ,
                 ec_point_t const *P) {
  // Sanity tests
  // point_print("PnQ: ", *PnQ);
  // point_print("nQ: ", *nQ);
  // point_print("P: ", *P);
  assert(ec_is_zero(nQ));
  assert(is_point_equal(PnQ, P));

  fp2_mul(&R->x, &nQ->x, &P->x);
  fp2_copy(&R->z, &PnQ->x);
}

// (X(P):Z(P))->x(P)
void x_coord(fp2_t *r, ec_point_t const *P) {
  fp2_copy(r, &P->z);
  fp2_inv(r);
  fp2_mul(r, r, &P->x);
}

// compute the cubical translation of P by a point of 2-torsion T
void translate(ec_point_t *P, ec_point_t const *T) {
  fp2_t t0, t1, t2;
  if (fp2_is_zero(&T->z)) {
    // do nothing
  } else if (fp2_is_zero(&T->x)) {
    fp2_copy(&t0, &P->x);
    fp2_copy(&P->x, &P->z);
    fp2_copy(&P->z, &t0);
  } else {
    fp2_mul(&t0, &T->x, &P->x);
    fp2_mul(&t1, &T->z, &P->z);
    fp2_sub(&t2, &t0, &t1);
    fp2_mul(&t0, &T->z, &P->x);
    fp2_mul(&t1, &T->x, &P->z);
    fp2_sub(&P->z, &t0, &t1);
    fp2_copy(&P->x, &t2);
  }
}

// Compute the monodromy P+2^e Q (in level 1)
// The suffix _i means that we are given 1/x(P) as parameter.
// Warning: to get meaningful result when using the monodromy to compute
// pairings, we need P, Q, PQ, A24 to be normalised
// (this is not strictly necessary, but care need to be taken when they are not
// normalised. Only handle the normalised case for now)
void monodromy_i(ec_point_t *r, uint64_t e, ec_point_t const *PQ,
                 ec_point_t const *Q, ec_point_t const *P, fp2_t const *ixP,
                 ec_point_t const *A24) {
  ec_point_t PnQ, nQ;
  biext_ladder_2e(e - 1, &PnQ, &nQ, PQ, Q, ixP, A24);
  translate(&PnQ, &nQ);
  translate(&nQ, &nQ);
  point_ratio(r, &PnQ, &nQ, P);
}

void monodromy(ec_point_t *r, uint64_t e, ec_point_t const *PQ,
               ec_point_t const *Q, ec_point_t const *P,
               ec_point_t const *A24) {
  fp2_t ixP;
  fp2_copy(&ixP, &P->x);
  fp2_inv(&ixP);
  monodromy_i(r, e, PQ, Q, P, &ixP, A24);
}

// TODO: use only one inversion
// And normalize A24 at the same time (if needed), to save another inversion
void to_cubical(ec_point_t *Q, ec_point_t *P) {
  fp2_t invs[2];
  fp2_copy(&invs[0], &P->z);
  fp2_copy(&invs[1], &Q->z);
  fp2_batched_inv(invs, 2);

  fp2_mul(&P->x, &P->x, &invs[0]);
  fp2_set_one(&P->z);

  fp2_mul(&Q->x, &Q->x, &invs[1]);
  fp2_set_one(&Q->z);
}

// Normalize the points and also store 1/x(P), 1/x(Q)
void to_cubical_i(ec_point_t *P, ec_point_t *Q, fp2_t *ixP, fp2_t *ixQ) {
  /*
  //ec_normalize_point(A24);
  ec_normalize_point(P);
  ec_normalize_point(Q);
  //ec_normalize_point(PQ);
  fp2_copy(ixP, &P->x);
  fp2_inv(ixP);
  fp2_copy(ixQ, &Q->x);
  fp2_inv(ixQ);
  */
  fp2_t t[4];
  fp2_copy(&t[0], &P->x);
  fp2_copy(&t[1], &P->z);
  fp2_copy(&t[2], &Q->x);
  fp2_copy(&t[3], &Q->z);
  fp2_batched_inv(t, 4);
  fp2_mul(ixP, &P->z, &t[0]);
  fp2_mul(ixQ, &Q->z, &t[2]);
  fp2_mul(&P->x, &P->x, &t[1]);
  fp2_mul(&Q->x, &Q->x, &t[3]);
  fp2_set_one(&P->z);
  fp2_set_one(&Q->z);
}

/* (Do we need this?)
void to_cubical_c(ec_point_t* P, ec_point_t* A24, ec_point_t const* P_,
ec_point_t const* A24_) { copy_point(P, P_); copy_point(A24, A24_);
    inline_to_cubical(P, A24);
}
*/

// non reduced Tate pairing, PQ should be P+Q in (X:Z) coordinates
// Assume the cubical points are normalised, and that we have 1/x(P)
// The _n suffix means we assume the points are normalised
void non_reduced_tate_n(fp2_t *r, uint64_t e, ec_point_t *P, ec_point_t *Q,
                        ec_point_t *PQ, fp2_t const *ixP, ec_point_t *A24) {
  ec_point_t R;
  monodromy_i(&R, e, PQ, Q, P, ixP, A24);
  x_coord(r, &R);
}

// Same as above, but first normalise the points
void non_reduced_tate(fp2_t *r, uint64_t e, ec_point_t *P, ec_point_t *Q,
                      ec_point_t *PQ, ec_point_t *A24) {
  // to_cubical(Q, P);
  fp2_t ixP, ixQ;
  to_cubical_i(P, Q, &ixP, &ixQ); // TODO: ixQ not used
  non_reduced_tate_n(r, e, PQ, Q, P, &ixP, A24);
}

// Weil pairing, PQ should be P+Q in (X:Z) coordinates
// We assume the points are normalised correctly
// Do we need a weil_c version?
void weil_n(fp2_t *r, uint32_t e, ec_point_t const *P, ec_point_t const *Q,
            ec_point_t const *PQ, fp2_t const *ixP, fp2_t const *ixQ,
            ec_point_t const *A24) {
  ec_point_t R0, R1;
  monodromy_i(&R0, e, PQ, Q, P, ixP, A24);
  monodromy_i(&R1, e, PQ, P, Q, ixQ, A24);
  // TODO: check if that's the Weil pairing or its inverse
  fp2_mul(r, &R0.x, &R1.z);
  fp2_inv(r);
  fp2_mul(r, r, &R0.z);
  fp2_mul(r, r, &R1.x);
}

// Weil pairing, PQ should be P+Q in (X:Z) coordinates
// Normalise the points and call the code above
// The code will crash (division by 0) if either P or Q is (0:1)
void weil(fp2_t *r, uint32_t e, ec_point_t *P, ec_point_t *Q, ec_point_t *PQ,
          ec_curve_t *E) {
  fp2_t ixP, ixQ;
  to_cubical_i(P, Q, &ixP, &ixQ);
  if (!fp2_is_one(&E->A24.z) || fp2_is_zero(&E->A24.z)) {
    A24_from_AC_and_normalize(&E->A24, E);
  }
  weil_n(r, e, P, Q, PQ, &ixP, &ixQ, &E->A24);
}

int ec_curve_verify_A(const fp2_t *A) {
  /* A Montgomery curve is nonsingular iff A != +/-2 (with C != 0). */
  fp2_t two;
  fp2_set_one(&two);
  fp2_add(&two, &two, &two);
  if (fp2_is_equal(A, &two)) return 0;
  fp2_neg(&two, &two);
  if (fp2_is_equal(A, &two)) return 0;
  return 1;
}

// cyclotomic squaring: (a+bi)^2 = (2a^2 - 1) + 2abi
static inline void fp2_sqr_cyclo(fp2_t *x, const fp2_t *y) {
  fp_t a2, ab;
  fp_sqr(a2, y->re);
  fp_mul(ab, y->re, y->im);
  fp_add(a2, a2, a2);
  fp_t one;
  fp_mont_setone(one);
  fp_t t_re, t_im;
  fp_sub(t_re, a2, one);
  fp_add(t_im, ab, ab);
  fp_copy(x->re, t_re);
  fp_copy(x->im, t_im);
}

// cyclotomic cubing: (a+bi)^3 = a(4a^2 - 3) + b(4a^2 - 1)i
static inline void fp2_cube_cyclo(fp2_t *x, const fp2_t *y) {
  fp_t a2, a4_3, a4_1;
  fp_sqr(a2, y->re);
  fp_add(a4_1, a2, a2);
  fp_add(a4_1, a4_1, a4_1);
  fp_t one, two;
  fp_mont_setone(one);
  fp_add(two, one, one);
  fp_sub(a4_1, a4_1, one);
  fp_sub(a4_3, a4_1, two); // a4_1 - 2 = 4a^2 - 3
  fp_t t_re, t_im;
  fp_mul(t_re, y->re, a4_3);
  fp_mul(t_im, y->im, a4_1);
  fp_copy(x->re, t_re);
  fp_copy(x->im, t_im);
}

// recursive dlog function
static bool fp2_dlog_2e_rec(digit_t *a, long len, fp2_t *pows_f, fp2_t *pows_g,
                            long stacklen) {
  if (len == 0) {
    // *a = 0;
    for (int i = 0; i < NWORDS_ORDER; i++) {
      a[i] = 0;
    }
    return true;
  } else if (len == 1) {
    if (fp2_is_one(&pows_f[stacklen - 1])) {
      // a = 0;
      for (int i = 0; i < NWORDS_ORDER; i++) {
        a[i] = 0;
      }
      for (int i = 0; i < stacklen - 1; ++i) {
        fp2_sqr_cyclo(&pows_g[i], &pows_g[i]); // new_g = g^2
      }
      return true;
    } else if (fp2_is_equal(&pows_f[stacklen - 1], &pows_g[stacklen - 1])) {
      // a = 1;
      a[0] = 1;
      for (int i = 1; i < NWORDS_ORDER; i++) {
        a[i] = 0;
      }
      for (int i = 0; i < stacklen - 1; ++i) {
        fp2_mul(&pows_f[i], &pows_f[i], &pows_g[i]); // new_f = f*g
        fp2_sqr_cyclo(&pows_g[i], &pows_g[i]);       // new_g = g^2
      }
      return true;
    } else {
      return false;
    }
  } else {
    long right = (long)((double)len * 0.5);
    long left = len - right;
    pows_f[stacklen] = pows_f[stacklen - 1];
    pows_g[stacklen] = pows_g[stacklen - 1];
    for (int i = 0; i < left; i++) {
      fp2_sqr_cyclo(&pows_f[stacklen], &pows_f[stacklen]);
      fp2_sqr_cyclo(&pows_g[stacklen], &pows_g[stacklen]);
    }
    // uint32_t dlp1 = 0, dlp2 = 0;
    digit_t dlp1[NWORDS_ORDER], dlp2[NWORDS_ORDER];
    bool ok;
    ok = fp2_dlog_2e_rec(dlp1, right, pows_f, pows_g, stacklen + 1);
    if (!ok)
      return false;
    ok = fp2_dlog_2e_rec(dlp2, left, pows_f, pows_g, stacklen);
    if (!ok)
      return false;
    // a = dlp1 + 2^right * dlp2
    mp_shiftl_any(dlp2, (unsigned int)right, NWORDS_ORDER);
    mp_add(a, dlp2, dlp1, NWORDS_ORDER);

    return true;
  }
}

// compute DLP: compute scal such that f = g^scal with f, 1/g as input
static bool fp2_dlog_2e(digit_t *scal, const fp2_t *f, const fp2_t *g_inverse,
                        int e) {
  long log, len = e;
  for (log = 0; len > 1; len >>= 1)
    log++;
  log += 1;

  fp2_t pows_f[log], pows_g[log];
  pows_f[0] = *f;
  pows_g[0] = *g_inverse;

  for (int i = 0; i < NWORDS_ORDER; i++) {
    scal[i] = 0;
  }

  bool ok = fp2_dlog_2e_rec(scal, e, pows_f, pows_g, 1);
  assert(ok);

  return ok;
}

// Given two bases <P, Q> and basis = <R, S> compute
// x(P - R), x(P - S), x(R - Q), x(S - Q)
static void compute_difference_points(pairing_dlog_params_t *pairing_dlog_data,
                                      ec_curve_t *curve) {
  jac_point_t xyP, xyQ, xyR, xyS, temp;

  // lifting the two basis points, assumes that x(P) and x(R)
  // and the curve itself are normalised to (X : 1)
  lift_basis(&xyP, &xyQ, &pairing_dlog_data->PQ, curve);
  lift_basis(&xyR, &xyS, &pairing_dlog_data->RS, curve);

  // computation of the differences
  // x(P - R)
  jac_neg(&temp, &xyR);
  ADD(&temp, &temp, &xyP, curve);
  jac_to_xz(&pairing_dlog_data->diff.PmR, &temp);

  // x(P - S)
  jac_neg(&temp, &xyS);
  ADD(&temp, &temp, &xyP, curve);
  jac_to_xz(&pairing_dlog_data->diff.PmS, &temp);

  // x(R - Q)
  jac_neg(&temp, &xyQ);
  ADD(&temp, &temp, &xyR, curve);
  jac_to_xz(&pairing_dlog_data->diff.RmQ, &temp);

  // x(S - Q)
  jac_neg(&temp, &xyQ);
  ADD(&temp, &temp, &xyS, curve);
  jac_to_xz(&pairing_dlog_data->diff.SmQ, &temp);
}

// Normalize the bases (P, Q), (R, S) and store their inverse
// and additionally normalise the curve to (A/C : 1)
static void cubical_normalization_dlog(pairing_dlog_params_t *pairing_dlog_data,
                                       ec_curve_t *curve) {
  fp2_t t[12];
  ec_basis_t *PQ = &pairing_dlog_data->PQ;
  ec_basis_t *RS = &pairing_dlog_data->RS;
  fp2_copy(&t[0], &PQ->P.x);
  fp2_copy(&t[1], &PQ->P.z);
  fp2_copy(&t[2], &PQ->Q.x);
  fp2_copy(&t[3], &PQ->Q.z);
  fp2_copy(&t[4], &PQ->PmQ.x);
  fp2_copy(&t[5], &PQ->PmQ.z);
  fp2_copy(&t[6], &RS->P.x);
  fp2_copy(&t[7], &RS->P.z);
  fp2_copy(&t[8], &RS->Q.x);
  fp2_copy(&t[9], &RS->Q.z);
  fp2_copy(&t[10], &curve->C);
  fp2_copy(&t[11], &pairing_dlog_data->A24.z);

  fp2_batched_inv(t, 12);

  fp2_mul(&pairing_dlog_data->ixP, &PQ->P.z, &t[0]);
  fp2_mul(&PQ->P.x, &PQ->P.x, &t[1]);
  fp2_set_one(&PQ->P.z);

  fp2_mul(&pairing_dlog_data->ixQ, &PQ->Q.z, &t[2]);
  fp2_mul(&PQ->Q.x, &PQ->Q.x, &t[3]);
  fp2_set_one(&PQ->Q.z);

  fp2_mul(&PQ->PmQ.x, &PQ->PmQ.x, &t[5]);
  fp2_set_one(&PQ->PmQ.z);

  fp2_mul(&pairing_dlog_data->ixR, &RS->P.z, &t[6]);
  fp2_mul(&RS->P.x, &RS->P.x, &t[7]);
  fp2_set_one(&RS->P.z);

  fp2_mul(&pairing_dlog_data->ixS, &RS->Q.z, &t[8]);
  fp2_mul(&RS->Q.x, &RS->Q.x, &t[9]);
  fp2_set_one(&RS->Q.z);

  fp2_mul(&curve->A, &curve->A, &t[10]);
  fp2_set_one(&curve->C);

  fp2_mul(&pairing_dlog_data->A24.x, &pairing_dlog_data->A24.x, &t[11]);
  fp2_set_one(&pairing_dlog_data->A24.z);
  copy_point(&curve->A24, &pairing_dlog_data->A24);
}

// Inline all the Weil pairing computations needed for ec_dlog_2_weil
static void weil_dlog(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                      pairing_dlog_params_t *pairing_dlog_data) {

  ec_point_t nP, nQ, nR, nS, nPQ, PnQ, nPR, PnR, nPS, PnS, nRQ, RnQ, nSQ, SnQ;

  copy_point(&nP, &pairing_dlog_data->PQ.P);
  copy_point(&nQ, &pairing_dlog_data->PQ.Q);
  copy_point(&nR, &pairing_dlog_data->RS.P);
  copy_point(&nS, &pairing_dlog_data->RS.Q);
  copy_point(&nPQ, &pairing_dlog_data->PQ.PmQ);
  copy_point(&PnQ, &pairing_dlog_data->PQ.PmQ);
  copy_point(&nPR, &pairing_dlog_data->diff.PmR);
  copy_point(&nPS, &pairing_dlog_data->diff.PmS);
  copy_point(&PnR, &pairing_dlog_data->diff.PmR);
  copy_point(&PnS, &pairing_dlog_data->diff.PmS);
  copy_point(&nRQ, &pairing_dlog_data->diff.RmQ);
  copy_point(&nSQ, &pairing_dlog_data->diff.SmQ);
  copy_point(&RnQ, &pairing_dlog_data->diff.RmQ);
  copy_point(&SnQ, &pairing_dlog_data->diff.SmQ);

  for (uint32_t i = 0; i < pairing_dlog_data->e - 1; i++) {
    cubicalADD(&nPQ, &nPQ, &nP, &pairing_dlog_data->ixQ);
    cubicalADD(&nPR, &nPR, &nP, &pairing_dlog_data->ixR);
    cubicalADD(&nPS, &nPS, &nP, &pairing_dlog_data->ixS);
    cubicalDBL(&nP, &nP, &pairing_dlog_data->A24);

    cubicalADD(&PnQ, &PnQ, &nQ, &pairing_dlog_data->ixP);
    cubicalADD(&RnQ, &RnQ, &nQ, &pairing_dlog_data->ixR);
    cubicalADD(&SnQ, &SnQ, &nQ, &pairing_dlog_data->ixS);
    cubicalDBL(&nQ, &nQ, &pairing_dlog_data->A24);

    cubicalADD(&PnR, &PnR, &nR, &pairing_dlog_data->ixP);
    cubicalADD(&nRQ, &nRQ, &nR, &pairing_dlog_data->ixQ);
    cubicalDBL(&nR, &nR, &pairing_dlog_data->A24);

    cubicalADD(&PnS, &PnS, &nS, &pairing_dlog_data->ixP);
    cubicalADD(&nSQ, &nSQ, &nS, &pairing_dlog_data->ixQ);
    cubicalDBL(&nS, &nS, &pairing_dlog_data->A24);
  }

  // weil(&w0,e,&PQ->P,&PQ->Q,&PQ->PmQ,&A24);
  translate(&nPQ, &nP);
  translate(&nPR, &nP);
  translate(&nPS, &nP);
  translate(&PnQ, &nQ);
  translate(&RnQ, &nQ);
  translate(&SnQ, &nQ);
  translate(&PnR, &nR);
  translate(&nRQ, &nR);
  translate(&PnS, &nS);
  translate(&nSQ, &nS);

  translate(&nP, &nP);
  translate(&nQ, &nQ);
  translate(&nR, &nR);
  translate(&nS, &nS);

  // computation of the reference weil pairing
  ec_point_t T0, T1;
  fp2_t w1[5], w2[5];

  // e(P, Q) = w0
  point_ratio(&T0, &nPQ, &nP, &pairing_dlog_data->PQ.Q);
  point_ratio(&T1, &PnQ, &nQ, &pairing_dlog_data->PQ.P);
  // For the first element we need it's inverse for
  // fp2_dlog_2e so we swap w1 and w2 here to save inversions
  fp2_mul(&w2[0], &T0.x, &T1.z);
  fp2_mul(&w1[0], &T1.x, &T0.z);

  // e(P,R) = w0^r2
  point_ratio(&T0, &nPR, &nP, &pairing_dlog_data->RS.P);
  point_ratio(&T1, &PnR, &nR, &pairing_dlog_data->PQ.P);
  fp2_mul(&w1[1], &T0.x, &T1.z);
  fp2_mul(&w2[1], &T1.x, &T0.z);

  // e(R,Q) = w0^r1
  point_ratio(&T0, &nRQ, &nR, &pairing_dlog_data->PQ.Q);
  point_ratio(&T1, &RnQ, &nQ, &pairing_dlog_data->RS.P);
  fp2_mul(&w1[2], &T0.x, &T1.z);
  fp2_mul(&w2[2], &T1.x, &T0.z);

  // e(P,S) = w0^s2
  point_ratio(&T0, &nPS, &nP, &pairing_dlog_data->RS.Q);
  point_ratio(&T1, &PnS, &nS, &pairing_dlog_data->PQ.P);
  fp2_mul(&w1[3], &T0.x, &T1.z);
  fp2_mul(&w2[3], &T1.x, &T0.z);

  // e(S,Q) = w0^s1
  point_ratio(&T0, &nSQ, &nS, &pairing_dlog_data->PQ.Q);
  point_ratio(&T1, &SnQ, &nQ, &pairing_dlog_data->RS.Q);
  fp2_mul(&w1[4], &T0.x, &T1.z);
  fp2_mul(&w2[4], &T1.x, &T0.z);

  fp2_batched_inv(w1, 5);
  for (int i = 0; i < 5; i++) {
    fp2_mul(&w1[i], &w1[i], &w2[i]);
  }

  fp2_dlog_2e(r2, &w1[1], &w1[0], pairing_dlog_data->e);
  fp2_dlog_2e(r1, &w1[2], &w1[0], pairing_dlog_data->e);
  fp2_dlog_2e(s2, &w1[3], &w1[0], pairing_dlog_data->e);
  fp2_dlog_2e(s1, &w1[4], &w1[0], pairing_dlog_data->e);
}

void clear_cofac(fp2_t *r, const fp2_t *a) {
  const int nwords = sizeof(p_cofactor_for_2f) / sizeof(digit_t);
  fp2_pow_vartime(r, a, p_cofactor_for_2f, nwords);
}

static void tate_dlog_partial(digit_t *r1, digit_t *r2, digit_t *s1,
                              digit_t *s2,
                              pairing_dlog_params_t *pairing_dlog_data) {
  uint32_t e_full = POWER_OF_2;
  uint32_t e_diff = e_full - pairing_dlog_data->e;

  ec_point_t nP, nQ, nR, nS, nPQ, PnR, PnS, nRQ, nSQ;

  copy_point(&nP, &pairing_dlog_data->PQ.P);
  copy_point(&nQ, &pairing_dlog_data->PQ.Q);
  copy_point(&nR, &pairing_dlog_data->RS.P);
  copy_point(&nS, &pairing_dlog_data->RS.Q);
  copy_point(&nPQ, &pairing_dlog_data->PQ.PmQ);
  copy_point(&PnR, &pairing_dlog_data->diff.PmR);
  copy_point(&PnS, &pairing_dlog_data->diff.PmS);
  copy_point(&nRQ, &pairing_dlog_data->diff.RmQ);
  copy_point(&nSQ, &pairing_dlog_data->diff.SmQ);

  for (uint32_t i = 0; i < e_full - 1; i++) {
    biextDBL(&nPQ, &nP, &nPQ, &nP, &pairing_dlog_data->ixQ,
             &pairing_dlog_data->A24);
  }

  for (uint32_t i = 0; i < pairing_dlog_data->e - 1; i++) {
    cubicalADD(&PnR, &PnR, &nR, &pairing_dlog_data->ixP);
    biextDBL(&nRQ, &nR, &nRQ, &nR, &pairing_dlog_data->ixQ,
             &pairing_dlog_data->A24);

    cubicalADD(&PnS, &PnS, &nS, &pairing_dlog_data->ixP);
    biextDBL(&nSQ, &nS, &nSQ, &nS, &pairing_dlog_data->ixQ,
             &pairing_dlog_data->A24);
  }

  translate(&nPQ, &nP);
  translate(&PnR, &nR);
  translate(&nRQ, &nR);
  translate(&PnS, &nS);
  translate(&nSQ, &nS);

  translate(&nP, &nP);
  translate(&nQ, &nQ);
  translate(&nR, &nR);
  translate(&nS, &nS);

  // computation of the reference Tate pairing
  ec_point_t T0;
  fp2_t w1[5], w2[5];

  // t(P, Q)^(2^e_diff) = w0
  point_ratio(&T0, &nPQ, &nP, &pairing_dlog_data->PQ.Q);
  fp2_copy(&w1[0], &T0.x);
  fp2_copy(&w2[0], &T0.z);

  // t(R,P) = w0^r2
  point_ratio(&T0, &PnR, &nR, &pairing_dlog_data->PQ.P);
  fp2_copy(&w1[1], &T0.x);
  fp2_copy(&w2[1], &T0.z);

  // t(R,Q) = w0^r1
  point_ratio(&T0, &nRQ, &nR, &pairing_dlog_data->PQ.Q);
  fp2_copy(&w2[2], &T0.x);
  fp2_copy(&w1[2], &T0.z);

  // t(S,P) = w0^s2
  point_ratio(&T0, &PnS, &nS, &pairing_dlog_data->PQ.P);
  fp2_copy(&w1[3], &T0.x);
  fp2_copy(&w2[3], &T0.z);

  // t(S,Q) = w0^s1
  point_ratio(&T0, &nSQ, &nS, &pairing_dlog_data->PQ.Q);
  fp2_copy(&w2[4], &T0.x);
  fp2_copy(&w1[4], &T0.z);

  // batched reduction using projective representation
  for (int i = 0; i < 5; i++) {
    fp2_t frob, tmp;
    fp2_copy(&tmp, &w1[i]);
    // inline frobenius for ^p
    // multiply by inverse to get ^(p-1)
    fp2_frob(&frob, &w1[i]);
    fp2_mul(&w1[i], &w2[i], &frob);

    // repeat for denom
    fp2_frob(&frob, &w2[i]);
    fp2_mul(&w2[i], &tmp, &frob);
  }

  // batched normalization
  fp2_batched_inv(w2, 5);
  for (int i = 0; i < 5; i++) {
    fp2_mul(&w1[i], &w1[i], &w2[i]);
  }

  for (int i = 0; i < 5; i++) {
    clear_cofac(&w1[i], &w1[i]);

    // removes 2^e_diff
    for (uint32_t j = 0; j < e_diff; j++) {
      fp2_sqr(&w1[i], &w1[i]);
    }
  }

  fp2_dlog_2e(r2, &w1[1], &w1[0], pairing_dlog_data->e);
  fp2_dlog_2e(r1, &w1[2], &w1[0], pairing_dlog_data->e);
  fp2_dlog_2e(s2, &w1[3], &w1[0], pairing_dlog_data->e);
  fp2_dlog_2e(s1, &w1[4], &w1[0], pairing_dlog_data->e);
}

void ec_dlog_2_tate(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                    const ec_basis_t *PQ, const ec_basis_t *RS,
                    const ec_curve_t *curve, int e) {
  // count_ec_dlog_2_tate++;
  // active_fp2_counters = &count_ec_dlog_2_tate_fp2;
  // assume PQ is a full torsion basis
  // returns a, b, c, d such that R = [a]P + [b]Q, S = [c]P + [d]Q

#ifndef NDEBUG
  int e_full = POWER_OF_2;
  int e_diff = e_full - e;
#endif
  assert(test_point_order_twof(&PQ->P, curve, e_full));
  assert(test_point_order_twof(&PQ->Q, curve, e_full));

  ec_curve_t local_curve = *curve;

  // precomputing the correct curve data
  if (!fp2_is_one(&local_curve.A24.z) || fp2_is_zero(&local_curve.A24.z)) {
    A24_from_AC(&local_curve.A24, &local_curve);
  }

  pairing_dlog_params_t pairing_dlog_data;
  pairing_dlog_data.e = e;
  pairing_dlog_data.PQ = *PQ;
  pairing_dlog_data.RS = *RS;
  pairing_dlog_data.A24 = local_curve.A24;

  cubical_normalization_dlog(&pairing_dlog_data, &local_curve);
  compute_difference_points(&pairing_dlog_data, &local_curve);
  tate_dlog_partial(r1, r2, s1, s2, &pairing_dlog_data);

#ifndef NDEBUG
  ec_point_t test;
  ec_biscalar_mul(&test, &local_curve, r1, r2, PQ);
  for (int i = 0; i < e_diff; i++) {
    ec_dbl(&test, &local_curve, &test);
  }
  // R = [r1]P + [r2]Q
  assert(ec_is_equal(&test, &RS->P));

  ec_biscalar_mul(&test, &local_curve, s1, s2, PQ);
  for (int i = 0; i < e_diff; i++) {
    ec_dbl(&test, &local_curve, &test);
  }
  // S = [s1]P + [s2]Q
  assert(ec_is_equal(&test, &RS->Q));
#endif
  // active_fp2_counters = NULL;
}

void ec_dlog_2_weil(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                    const ec_basis_t *PQ, const ec_basis_t *RS,
                    ec_curve_t *curve, int e) {
  assert(test_point_order_twof(&PQ->Q, curve, e));

  // precomputing the correct curve data
  if (!fp2_is_one(&curve->A24.z) || fp2_is_zero(&curve->A24.z)) {
    A24_from_AC(&curve->A24, curve);
  }

  pairing_dlog_params_t pairing_dlog_data;
  pairing_dlog_data.e = e;
  pairing_dlog_data.PQ = *PQ;
  pairing_dlog_data.RS = *RS;
  pairing_dlog_data.A24 = curve->A24;

  cubical_normalization_dlog(&pairing_dlog_data, curve);
  compute_difference_points(&pairing_dlog_data, curve);

  weil_dlog(r1, r2, s1, s2, &pairing_dlog_data);

#ifndef NDEBUG
  ec_point_t test;
  ec_biscalar_mul(&test, curve, r1, r2, PQ);
  // R = [r1]P + [r2]Q
  assert(ec_is_equal(&test, &RS->P));
  ec_biscalar_mul(&test, curve, s1, s2, PQ);
  // S = [s1]P + [s2]Q
  assert(ec_is_equal(&test, &RS->Q));
#endif
}

// -----------------------------------------------------------------------
// Variant of ec_dlog_2_weil computing only R = [r1]P + [r2]Q
// (skips all computation involving the second basis point S)
// -----------------------------------------------------------------------

// Normalise P, Q, PmQ, R and the curve for the single-point dlog.
// Uses a batched inversion of 8 Z-coordinates.
// Fields left unused (ixS, RS.Q, diff.PmS, diff.SmQ) are not touched.
static void cubical_normalization_dlog_R(pairing_dlog_params_t *d,
                                         ec_curve_t *curve) {
  fp2_t t[10];
  ec_basis_t *PQ = &d->PQ;
  // RS.P holds R
  fp2_copy(&t[0], &PQ->P.x);
  fp2_copy(&t[1], &PQ->P.z);
  fp2_copy(&t[2], &PQ->Q.x);
  fp2_copy(&t[3], &PQ->Q.z);
  fp2_copy(&t[4], &PQ->PmQ.x);
  fp2_copy(&t[5], &PQ->PmQ.z);
  fp2_copy(&t[6], &d->RS.P.x);
  fp2_copy(&t[7], &d->RS.P.z);
  fp2_copy(&t[8], &curve->C); // include C in the batch (was separate fp2_inv)
  fp2_copy(&t[9], &d->A24.z);

  fp2_batched_inv(t, 10);

  fp2_mul(&d->ixP, &PQ->P.z, &t[0]);
  fp2_mul(&PQ->P.x, &PQ->P.x, &t[1]);
  fp2_set_one(&PQ->P.z);

  fp2_mul(&d->ixQ, &PQ->Q.z, &t[2]);
  fp2_mul(&PQ->Q.x, &PQ->Q.x, &t[3]);
  fp2_set_one(&PQ->Q.z);

  fp2_mul(&PQ->PmQ.x, &PQ->PmQ.x, &t[5]);
  fp2_set_one(&PQ->PmQ.z);

  fp2_mul(&d->ixR, &d->RS.P.z, &t[6]);
  fp2_mul(&d->RS.P.x, &d->RS.P.x, &t[7]);
  fp2_set_one(&d->RS.P.z);

  // Normalise curve A/C -> A using the batched inverse of C (t[8])
  fp2_mul(&curve->A, &curve->A, &t[8]);
  fp2_set_one(&curve->C);

  fp2_mul(&d->A24.x, &d->A24.x, &t[9]);
  fp2_set_one(&d->A24.z);
  copy_point(&curve->A24, &d->A24);
}

// Compute x(P - R) and x(R - Q) only.
static void compute_difference_points_R(pairing_dlog_params_t *d,
                                        ec_curve_t *curve) {
  jac_point_t xyP, xyQ, xyR, temp;

  // Lift individual points directly – avoids the need for a valid PmQ entry
  // in the RS basis (which we do not have for the single-point variant).
  lift_basis(&xyP, &xyQ, &d->PQ, curve);
  lift_point(&xyR, &d->RS.P, curve);

  // x(P - R)
  jac_neg(&temp, &xyR);
  ADD(&temp, &temp, &xyP, curve);
  jac_to_xz(&d->diff.PmR, &temp);

  // x(R - Q)
  jac_neg(&temp, &xyQ);
  ADD(&temp, &temp, &xyR, curve);
  jac_to_xz(&d->diff.RmQ, &temp);
}

// Weil dlog for a single point R: compute r1, r2 such that R = [r1]P + [r2]Q.
// Uses only the three pairings e(P,Q), e(P,R), e(R,Q).
static void weil_dlog_R(digit_t *r1, digit_t *r2, pairing_dlog_params_t *d) {
  ec_point_t nP, nQ, nR, nPQ, PnQ, nPR, PnR, nRQ, RnQ;

  copy_point(&nP, &d->PQ.P);
  copy_point(&nQ, &d->PQ.Q);
  copy_point(&nR, &d->RS.P);
  copy_point(&nPQ, &d->PQ.PmQ);
  copy_point(&PnQ, &d->PQ.PmQ);
  copy_point(&nPR, &d->diff.PmR);
  copy_point(&PnR, &d->diff.PmR);
  copy_point(&nRQ, &d->diff.RmQ);
  copy_point(&RnQ, &d->diff.RmQ);

  for (uint32_t i = 0; i < d->e - 1; i++) {
    cubicalADD(&nPQ, &nPQ, &nP, &d->ixQ);
    cubicalADD(&nPR, &nPR, &nP, &d->ixR);
    cubicalDBL(&nP, &nP, &d->A24);

    cubicalADD(&PnQ, &PnQ, &nQ, &d->ixP);
    cubicalADD(&RnQ, &RnQ, &nQ, &d->ixR);
    cubicalDBL(&nQ, &nQ, &d->A24);

    cubicalADD(&PnR, &PnR, &nR, &d->ixP);
    cubicalADD(&nRQ, &nRQ, &nR, &d->ixQ);
    cubicalDBL(&nR, &nR, &d->A24);
  }

  translate(&nPQ, &nP);
  translate(&nPR, &nP);
  translate(&PnQ, &nQ);
  translate(&RnQ, &nQ);
  translate(&PnR, &nR);
  translate(&nRQ, &nR);

  translate(&nP, &nP);
  translate(&nQ, &nQ);
  translate(&nR, &nR);

  ec_point_t T0, T1;
  fp2_t w1[3], w2[3];

  // e(P, Q) = w0  (stored inverted in w1[0] for fp2_dlog_2e)
  point_ratio(&T0, &nPQ, &nP, &d->PQ.Q);
  point_ratio(&T1, &PnQ, &nQ, &d->PQ.P);
  fp2_mul(&w2[0], &T0.x, &T1.z);
  fp2_mul(&w1[0], &T1.x, &T0.z);

  // e(P, R) = w0^r2
  point_ratio(&T0, &nPR, &nP, &d->RS.P);
  point_ratio(&T1, &PnR, &nR, &d->PQ.P);
  fp2_mul(&w1[1], &T0.x, &T1.z);
  fp2_mul(&w2[1], &T1.x, &T0.z);

  // e(R, Q) = w0^r1
  point_ratio(&T0, &nRQ, &nR, &d->PQ.Q);
  point_ratio(&T1, &RnQ, &nQ, &d->RS.P);
  fp2_mul(&w1[2], &T0.x, &T1.z);
  fp2_mul(&w2[2], &T1.x, &T0.z);

  fp2_batched_inv(w1, 3);
  for (int i = 0; i < 3; i++) {
    fp2_mul(&w1[i], &w1[i], &w2[i]);
  }

  fp2_dlog_2e(r2, &w1[1], &w1[0], d->e);
  fp2_dlog_2e(r1, &w1[2], &w1[0], d->e);
}

// Public API ---------------------------------------------------------------
void ec_dlog_2_weil_R(digit_t *scalarP, digit_t *scalarQ, const ec_basis_t *PQ2,
                      const ec_point_t *R, ec_curve_t *curve, int e) {
  assert(test_point_order_twof(&PQ2->Q, curve, e));

  if (!fp2_is_one(&curve->A24.z) || fp2_is_zero(&curve->A24.z)) {
    A24_from_AC(&curve->A24, curve);
  }

  pairing_dlog_params_t d;
  memset(&d, 0, sizeof(d));
  d.e = e;
  d.PQ = *PQ2;
  d.RS.P = *R; // only R is used; RS.Q is intentionally left zeroed
  d.A24 = curve->A24;

  // Build a dummy RS.Q so that lift_basis inside compute_difference_points_R
  // does not crash (lift_basis lifts both points; the result for RS.Q is
  // discarded immediately).
  ec_set_zero(&d.RS.Q);
  // PmQ field of RS is not needed, zero it out
  ec_set_zero(&d.RS.PmQ);

  cubical_normalization_dlog_R(&d, curve);
  compute_difference_points_R(&d, curve);

  weil_dlog_R(scalarP, scalarQ, &d);

#ifndef NDEBUG
  ec_point_t test;
  ec_biscalar_mul(&test, curve, scalarP, scalarQ, PQ2);
  assert(ec_is_equal(&test, R));
#endif
}

// -----------------------------------------------------------------------
// Single-point Tate variant: ec_dlog_2_tate_R
// Computes r1, r2 such that R = [r1]P + [r2]Q using the Tate pairing.
// PQ2 must be a FULL E[2^f]-torsion basis; R must have order dividing 2^e.
// -----------------------------------------------------------------------

static void tate_dlog_partial_R(digit_t *r1, digit_t *r2,
                                pairing_dlog_params_t *d) {
  uint32_t e_full = POWER_OF_2;
  uint32_t e_diff = e_full - d->e;

  ec_point_t nP, nQ, nR, nPQ, PnR, nRQ;

  copy_point(&nP, &d->PQ.P);
  copy_point(&nQ, &d->PQ.Q);
  copy_point(&nR, &d->RS.P);
  copy_point(&nPQ, &d->PQ.PmQ);
  copy_point(&PnR, &d->diff.PmR);
  copy_point(&nRQ, &d->diff.RmQ);

  // Full-order ladder for t(P, Q)^(2^e_diff)
  for (uint32_t i = 0; i < e_full - 1; i++) {
    biextDBL(&nPQ, &nP, &nPQ, &nP, &d->ixQ, &d->A24);
  }

  // Short-order ladder for t(R, P) and t(R, Q)
  for (uint32_t i = 0; i < d->e - 1; i++) {
    cubicalADD(&PnR, &PnR, &nR, &d->ixP);
    biextDBL(&nRQ, &nR, &nRQ, &nR, &d->ixQ, &d->A24);
  }

  translate(&nPQ, &nP);
  translate(&PnR, &nR);
  translate(&nRQ, &nR);
  translate(&nP, &nP);
  translate(&nQ, &nQ);
  translate(&nR, &nR);

  ec_point_t T0;
  fp2_t w1[3], w2[3];

  // t(P, Q)^(2^e_diff)  -- reference pairing
  point_ratio(&T0, &nPQ, &nP, &d->PQ.Q);
  fp2_copy(&w1[0], &T0.x);
  fp2_copy(&w2[0], &T0.z);

  // t(R, P) = ref^r2
  point_ratio(&T0, &PnR, &nR, &d->PQ.P);
  fp2_copy(&w1[1], &T0.x);
  fp2_copy(&w2[1], &T0.z);

  // t(R, Q) = ref^r1   (note: sign flip – denom is x, num is z)
  point_ratio(&T0, &nRQ, &nR, &d->PQ.Q);
  fp2_copy(&w2[2], &T0.x);
  fp2_copy(&w1[2], &T0.z);

  // Frobenius reduction: replace (X:Z) by frob(X)*Z : X*frob(Z)
  for (int i = 0; i < 3; i++) {
    fp2_t frob, tmp;
    fp2_copy(&tmp, &w1[i]);
    fp2_frob(&frob, &w1[i]);
    fp2_mul(&w1[i], &w2[i], &frob);
    fp2_frob(&frob, &w2[i]);
    fp2_mul(&w2[i], &tmp, &frob);
  }

  // Normalise
  fp2_batched_inv(w2, 3);
  for (int i = 0; i < 3; i++) {
    fp2_mul(&w1[i], &w1[i], &w2[i]);
  }

  // Clear 3-cofactor then remaining 2^e_diff factor
  for (int i = 0; i < 3; i++) {
    clear_cofac(&w1[i], &w1[i]);
    for (uint32_t j = 0; j < e_diff; j++) {
      fp2_sqr(&w1[i], &w1[i]);
    }
  }

  fp2_dlog_2e(r2, &w1[1], &w1[0], d->e);
  fp2_dlog_2e(r1, &w1[2], &w1[0], d->e);
}

void ec_dlog_2_tate_R(digit_t *scalarP, digit_t *scalarQ, const ec_basis_t *PQ2,
                      const ec_point_t *R, ec_curve_t *curve, int e) {
  // count_ec_dlog_2_tate_R++;
  // active_fp2_counters = &count_ec_dlog_2_tate_R_fp2;
  if (!fp2_is_one(&curve->A24.z) || fp2_is_zero(&curve->A24.z)) {
    A24_from_AC(&curve->A24, curve);
  }

  pairing_dlog_params_t d;
  memset(&d, 0, sizeof(d));
  d.e = e;
  d.PQ = *PQ2;
  d.RS.P = *R;
  d.A24 = curve->A24;

  ec_set_zero(&d.RS.Q);
  ec_set_zero(&d.RS.PmQ);

  cubical_normalization_dlog_R(&d, curve);
  compute_difference_points_R(&d, curve);

  tate_dlog_partial_R(scalarP, scalarQ, &d);

#ifndef NDEBUG
  ec_point_t test;
  ec_biscalar_mul(&test, curve, scalarP, scalarQ, PQ2);
  assert(ec_is_equal(&test, R));
#endif
  // active_fp2_counters = NULL;
}

// -------------------------------------------------------------
// 3-power Weil DLP implementation using Cubical Ladder (Weil_pairing_odd)

struct cubical_ladder_result {
  fp2_t Xn1P;
  fp2_t Zn1PQ;
};

static void cubical_ladder_odd(struct cubical_ladder_result *res,
                               const digit_t *n_minus_1, int n_bits,
                               fp2_t const *XPQ, fp2_t const *XPQinv,
                               fp2_t const *XP, fp2_t const *XPinv,
                               fp2_t const *XQinv, ec_point_t const *A24) {
  ec_point_t nP, n1P, n1PQ;

  nP.x = *XP;
  fp2_set_one(&nP.z);

  n1P.x = *XP;
  fp2_set_one(&n1P.z);
  cubicalDBL(&n1P, &n1P, A24);

  n1PQ.x = *XPQ;
  fp2_set_one(&n1PQ.z);

  ec_point_t P_pt;
  P_pt.x = *XP;
  fp2_set_one(&P_pt.z);
  cubicalADD(&n1PQ, &n1PQ, &P_pt, XQinv);

  for (int i = n_bits - 2; i >= 0; i--) {
    int word_idx = i / RADIX;
    int bit_idx = i % RADIX;
    int bit = (n_minus_1[word_idx] >> bit_idx) & 1;

    if (bit == 0) {
      cubicalADD(&n1PQ, &n1PQ, &nP, XPQinv);
      cubicalADD(&n1P, &n1P, &nP, XPinv);
      cubicalDBL(&nP, &nP, A24);
    } else {
      cubicalADD(&nP, &n1P, &nP, XPinv);
      cubicalADD(&n1PQ, &n1PQ, &n1P, XQinv);
      cubicalDBL(&n1P, &n1P, A24);
    }
  }
  res->Xn1P = n1P.x;
  res->Zn1PQ = n1PQ.z;
}

static void weil_pairing_odd(fp2_t *lamPd_lamQn, fp2_t *lamPn_lamQd,
                             fp2_t const *XP, fp2_t const *XPinv,
                             fp2_t const *XQ, fp2_t const *XQinv,
                             fp2_t const *XPQ, fp2_t const *XPQinv,
                             const digit_t *n_minus_1, int n_bits,
                             ec_point_t const *A24) {
  struct cubical_ladder_result res1, res2;
  cubical_ladder_odd(&res1, n_minus_1, n_bits, XPQ, XPQinv, XP, XPinv, XQinv,
                     A24);
  cubical_ladder_odd(&res2, n_minus_1, n_bits, XPQ, XPQinv, XQ, XQinv, XPinv,
                     A24);

  fp2_mul(lamPd_lamQn, &res1.Xn1P, &res2.Zn1PQ);
  fp2_mul(lamPn_lamQd, &res1.Zn1PQ, &res2.Xn1P);
}

static bool fp2_dlog_3e_rec(digit_t *a, long len, fp2_t *pows_f, fp2_t *pows_g,
                            long stacklen) {
  if (len == 0) {
    for (int i = 0; i < NWORDS_ORDER; i++)
      a[i] = 0;
    return true;
  } else if (len == 1) {
    if (fp2_is_one(&pows_f[stacklen - 1])) {
      for (int i = 0; i < NWORDS_ORDER; i++)
        a[i] = 0;
      for (int i = 0; i < stacklen - 1; ++i) {
        fp2_cube_cyclo(&pows_g[i], &pows_g[i]);
      }
      return true;
    } else {
      fp2_t g_inv_sq;
      fp2_sqr(&g_inv_sq, &pows_g[stacklen - 1]);
      if (fp2_is_equal(&pows_f[stacklen - 1], &g_inv_sq)) {
        // f = (g^{-1})^2 = g^1 => a = 1
        a[0] = 1;
        for (int i = 1; i < NWORDS_ORDER; i++)
          a[i] = 0;
        for (int i = 0; i < stacklen - 1; ++i) {
          fp2_mul(&pows_f[i], &pows_f[i], &pows_g[i]);
          fp2_cube_cyclo(&pows_g[i], &pows_g[i]);
        }
        return true;
      } else if (fp2_is_equal(&pows_f[stacklen - 1], &pows_g[stacklen - 1])) {
        // f = g^{-1} = g^2 => a = 2
        a[0] = 2;
        for (int i = 1; i < NWORDS_ORDER; i++)
          a[i] = 0;
        for (int i = 0; i < stacklen - 1; ++i) {
          fp2_t g_inv_sq_i;
          fp2_sqr_cyclo(&g_inv_sq_i, &pows_g[i]);
          fp2_mul(&pows_f[i], &pows_f[i], &g_inv_sq_i);
          fp2_cube_cyclo(&pows_g[i], &pows_g[i]);
        }
        return true;
      }
      return false;
    }
  } else {
    long right = (long)((double)len * 0.5);
    long left = len - right;
    pows_f[stacklen] = pows_f[stacklen - 1];
    pows_g[stacklen] = pows_g[stacklen - 1];
    for (int i = 0; i < left; i++) {
      fp2_cube_cyclo(&pows_f[stacklen], &pows_f[stacklen]);
      fp2_cube_cyclo(&pows_g[stacklen], &pows_g[stacklen]);
    }
    digit_t dlp1[NWORDS_ORDER], dlp2[NWORDS_ORDER];
    bool ok;
    ok = fp2_dlog_3e_rec(dlp1, right, pows_f, pows_g, stacklen + 1);
    if (!ok)
      return false;
    ok = fp2_dlog_3e_rec(dlp2, left, pows_f, pows_g, stacklen);
    if (!ok)
      return false;

    for (int i = 0; i < right; i++) {
      mp_mul_digit_add(dlp2, dlp2, 3, 0, NWORDS_ORDER);
    }
    mp_add(a, dlp2, dlp1, NWORDS_ORDER);
    return true;
  }
}

static bool fp2_dlog_3e(digit_t *scal, const fp2_t *f, const fp2_t *g_inverse,
                        int e) {
  long log, len = e;
  for (log = 0; len > 1; len >>= 1)
    log++;
  log += 1;
  fp2_t pows_f[log], pows_g[log];
  pows_f[0] = *f;
  pows_g[0] = *g_inverse;
  for (int i = 0; i < NWORDS_ORDER; i++)
    scal[i] = 0;
  bool ok = fp2_dlog_3e_rec(scal, e, pows_f, pows_g, 1);
  return ok;
}

static void
compute_difference_points_3(pairing_dlog_params_3_t *pairing_dlog_data,
                            ec_curve_t *curve) {
  jac_point_t xyP, xyQ, xyR, xyS, temp;
  lift_basis(&xyP, &xyQ, &pairing_dlog_data->PQ, curve);
  lift_basis(&xyR, &xyS, &pairing_dlog_data->RS, curve);
  jac_neg(&temp, &xyR);
  ADD(&temp, &temp, &xyP, curve);
  jac_to_xz(&pairing_dlog_data->diff.PmR, &temp);
  jac_neg(&temp, &xyS);
  ADD(&temp, &temp, &xyP, curve);
  jac_to_xz(&pairing_dlog_data->diff.PmS, &temp);
  jac_neg(&temp, &xyQ);
  ADD(&temp, &temp, &xyR, curve);
  jac_to_xz(&pairing_dlog_data->diff.RmQ, &temp);
  jac_neg(&temp, &xyQ);
  ADD(&temp, &temp, &xyS, curve);
  jac_to_xz(&pairing_dlog_data->diff.SmQ, &temp);
}

static void normalize_params_3(pairing_dlog_params_3_t *d, ec_curve_t *curve) {
  fp2_t t[8];
  t[0] = d->PQ.P.z;
  t[1] = d->PQ.Q.z;
  t[2] = d->PQ.PmQ.z;
  t[3] = d->RS.P.z;
  t[4] = d->RS.Q.z;
  t[5] = d->RS.PmQ.z;
  t[6] = curve->C;
  t[7] = curve->A24.z;

  fp2_batched_inv(t, 8);

  // Normalize basis PQ
  fp2_mul(&d->PQ.P.x, &d->PQ.P.x, &t[0]);
  fp2_set_one(&d->PQ.P.z);
  fp2_mul(&d->PQ.Q.x, &d->PQ.Q.x, &t[1]);
  fp2_set_one(&d->PQ.Q.z);
  fp2_mul(&d->PQ.PmQ.x, &d->PQ.PmQ.x, &t[2]);
  fp2_set_one(&d->PQ.PmQ.z);

  // Normalize basis RS
  fp2_mul(&d->RS.P.x, &d->RS.P.x, &t[3]);
  fp2_set_one(&d->RS.P.z);
  fp2_mul(&d->RS.Q.x, &d->RS.Q.x, &t[4]);
  fp2_set_one(&d->RS.Q.z);
  fp2_mul(&d->RS.PmQ.x, &d->RS.PmQ.x, &t[5]);
  fp2_set_one(&d->RS.PmQ.z);

  // Normalize Curve
  fp2_mul(&curve->A, &curve->A, &t[6]);
  fp2_set_one(&curve->C);

  fp2_mul(&curve->A24.x, &curve->A24.x, &t[7]);
  fp2_set_one(&curve->A24.z);

  // Compute basis inversion constants
  fp2_t invs[4];
  invs[0] = d->PQ.P.x;
  invs[1] = d->PQ.Q.x;
  invs[2] = d->RS.P.x;
  invs[3] = d->RS.Q.x;
  fp2_batched_inv(invs, 4);
  d->ixP = invs[0];
  d->ixQ = invs[1];
  d->ixR = invs[2];
  d->ixS = invs[3];
}

static int compute_n_minus_1(digit_t *n_minus_1, int e) {
  for (int i = 0; i < NWORDS_ORDER; i++)
    n_minus_1[i] = 0;
  n_minus_1[0] = 1;
  for (int i = 0; i < e; i++) {
    mp_mul_digit_add(n_minus_1, n_minus_1, 3, 0, NWORDS_ORDER);
  }
  digit_t one_arr[NWORDS_ORDER] = {0};
  one_arr[0] = 1;
  mp_sub(n_minus_1, n_minus_1, one_arr, NWORDS_ORDER);

  int n_bits = 0;
  for (int i = NWORDS_ORDER - 1; i >= 0; i--) {
    if (n_minus_1[i] != 0) {
      digit_t w = n_minus_1[i];
      int b = 0;
      while (w) {
        b++;
        w >>= 1;
      }
      n_bits = i * RADIX + b;
      break;
    }
  }
  return n_bits;
}

void ec_dlog_3_weil(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                    const ec_basis_t *PQ, const ec_basis_t *RS,
                    const ec_curve_t *curve, int e) {
  ec_curve_t local_curve = *curve;
  if (!fp2_is_one(&local_curve.A24.z) || fp2_is_zero(&local_curve.A24.z)) {
    A24_from_AC(&local_curve.A24, &local_curve);
  }

  pairing_dlog_params_3_t d;
  d.e = e;
  d.PQ = *PQ;
  d.RS = *RS;

  normalize_params_3(&d, &local_curve);
  compute_difference_points_3(&d, &local_curve);
  // Re-normalize diff points to Z=1 after computation
  fp2_t t_diff[4];
  t_diff[0] = d.diff.PmR.z;
  t_diff[1] = d.diff.PmS.z;
  t_diff[2] = d.diff.RmQ.z;
  t_diff[3] = d.diff.SmQ.z;
  fp2_batched_inv(t_diff, 4);
  fp2_mul(&d.diff.PmR.x, &d.diff.PmR.x, &t_diff[0]);
  fp2_set_one(&d.diff.PmR.z);
  fp2_mul(&d.diff.PmS.x, &d.diff.PmS.x, &t_diff[1]);
  fp2_set_one(&d.diff.PmS.z);
  fp2_mul(&d.diff.RmQ.x, &d.diff.RmQ.x, &t_diff[2]);
  fp2_set_one(&d.diff.RmQ.z);
  fp2_mul(&d.diff.SmQ.x, &d.diff.SmQ.x, &t_diff[3]);
  fp2_set_one(&d.diff.SmQ.z);

  digit_t n_minus_1[NWORDS_ORDER];
  int n_bits = compute_n_minus_1(n_minus_1, e);

  fp2_t num[5], den[5];
  fp2_t ixPmQ, ixPmR, ixPmS, ixRmQ, ixSmQ;
  fp2_t inv_diff[5];
  inv_diff[0] = d.PQ.PmQ.x;
  inv_diff[1] = d.diff.PmR.x;
  inv_diff[2] = d.diff.PmS.x;
  inv_diff[3] = d.diff.RmQ.x;
  inv_diff[4] = d.diff.SmQ.x;
  fp2_batched_inv(inv_diff, 5);
  ixPmQ = inv_diff[0];
  ixPmR = inv_diff[1];
  ixPmS = inv_diff[2];
  ixRmQ = inv_diff[3];
  ixSmQ = inv_diff[4];

  weil_pairing_odd(&num[0], &den[0], &d.PQ.P.x, &d.ixP, &d.PQ.Q.x, &d.ixQ,
                   &d.PQ.PmQ.x, &ixPmQ, n_minus_1, n_bits, &local_curve.A24);
  weil_pairing_odd(&num[1], &den[1], &d.PQ.P.x, &d.ixP, &d.RS.P.x, &d.ixR,
                   &d.diff.PmR.x, &ixPmR, n_minus_1, n_bits, &local_curve.A24);
  weil_pairing_odd(&num[2], &den[2], &d.RS.P.x, &d.ixR, &d.PQ.Q.x, &d.ixQ,
                   &d.diff.RmQ.x, &ixRmQ, n_minus_1, n_bits, &local_curve.A24);
  weil_pairing_odd(&num[3], &den[3], &d.PQ.P.x, &d.ixP, &d.RS.Q.x, &d.ixS,
                   &d.diff.PmS.x, &ixPmS, n_minus_1, n_bits, &local_curve.A24);
  weil_pairing_odd(&num[4], &den[4], &d.RS.Q.x, &d.ixS, &d.PQ.Q.x, &d.ixQ,
                   &d.diff.SmQ.x, &ixSmQ, n_minus_1, n_bits, &local_curve.A24);

  // Invert the first pairing element to serve as the base inverse
  fp2_t tmp;
  fp2_copy(&tmp, &num[0]);
  fp2_copy(&num[0], &den[0]);
  fp2_copy(&den[0], &tmp);

  fp2_batched_inv(den, 5);

  fp2_t w[5];
  for (int i = 0; i < 5; i++) {
    fp2_mul(&w[i], &num[i], &den[i]);
  }

  fp2_dlog_3e(r2, &w[1], &w[0], e);
  fp2_dlog_3e(r1, &w[2], &w[0], e);
  fp2_dlog_3e(s2, &w[3], &w[0], e);
  fp2_dlog_3e(s1, &w[4], &w[0], e);
}

// -----------------------------------------------------------------------
// Single-point variant: ec_dlog_3_weil_R
// Computes r1, r2 such that R = [r1]P + [r2]Q.
// -----------------------------------------------------------------------

static void normalize_params_3_R(pairing_dlog_params_3_t *d,
                                 ec_curve_t *curve) {
  fp2_t t[6];
  t[0] = d->PQ.P.z;
  t[1] = d->PQ.Q.z;
  t[2] = d->PQ.PmQ.z;
  t[3] = d->RS.P.z;
  t[4] = curve->C;
  t[5] = curve->A24.z;

  fp2_batched_inv(t, 6);

  // Normalize basis PQ
  fp2_mul(&d->PQ.P.x, &d->PQ.P.x, &t[0]);
  fp2_set_one(&d->PQ.P.z);
  fp2_mul(&d->PQ.Q.x, &d->PQ.Q.x, &t[1]);
  fp2_set_one(&d->PQ.Q.z);
  fp2_mul(&d->PQ.PmQ.x, &d->PQ.PmQ.x, &t[2]);
  fp2_set_one(&d->PQ.PmQ.z);

  // Normalize point R (in RS.P)
  fp2_mul(&d->RS.P.x, &d->RS.P.x, &t[3]);
  fp2_set_one(&d->RS.P.z);

  // Normalize Curve
  fp2_mul(&curve->A, &curve->A, &t[4]);
  fp2_set_one(&curve->C);

  fp2_mul(&curve->A24.x, &curve->A24.x, &t[5]);
  fp2_set_one(&curve->A24.z);

  // Compute inversion constants
  fp2_t invs[3];
  invs[0] = d->PQ.P.x;
  invs[1] = d->PQ.Q.x;
  invs[2] = d->RS.P.x;
  fp2_batched_inv(invs, 3);
  d->ixP = invs[0];
  d->ixQ = invs[1];
  d->ixR = invs[2]; // x(R) inverse
}

static void compute_difference_points_3_R(pairing_dlog_params_3_t *d,
                                          ec_curve_t *curve) {
  jac_point_t xyP, xyQ, xyR, temp;
  lift_basis(&xyP, &xyQ, &d->PQ, curve);
  lift_point(&xyR, &d->RS.P, curve);
  jac_neg(&temp, &xyR);
  ADD(&temp, &temp, &xyP, curve);
  jac_to_xz(&d->diff.PmR, &temp);
  jac_neg(&temp, &xyQ);
  ADD(&temp, &temp, &xyR, curve);
  jac_to_xz(&d->diff.RmQ, &temp);
}

void ec_dlog_3_weil_R(digit_t *r1, digit_t *r2, const ec_basis_t *PQ3,
                      const ec_point_t *R, const ec_curve_t *curve, int e) {
  ec_curve_t local_curve = *curve;
  if (!fp2_is_one(&local_curve.A24.z) || fp2_is_zero(&local_curve.A24.z)) {
    A24_from_AC(&local_curve.A24, &local_curve);
  }

  pairing_dlog_params_3_t d;
  d.e = e;
  d.PQ = *PQ3;
  d.RS.P = *R;

  normalize_params_3_R(&d, &local_curve);
  compute_difference_points_3_R(&d, &local_curve);

  // Re-normalize diff points
  fp2_t t_diff[2];
  t_diff[0] = d.diff.PmR.z;
  t_diff[1] = d.diff.RmQ.z;
  fp2_batched_inv(t_diff, 2);
  fp2_mul(&d.diff.PmR.x, &d.diff.PmR.x, &t_diff[0]);
  fp2_set_one(&d.diff.PmR.z);
  fp2_mul(&d.diff.RmQ.x, &d.diff.RmQ.x, &t_diff[1]);
  fp2_set_one(&d.diff.RmQ.z);

  digit_t n_minus_1[NWORDS_ORDER];
  int n_bits = compute_n_minus_1(n_minus_1, e);

  fp2_t num[3], den[3];
  fp2_t ixPmQ, ixPmR, ixRmQ;
  fp2_t inv_diff[3];
  inv_diff[0] = d.PQ.PmQ.x;
  inv_diff[1] = d.diff.PmR.x;
  inv_diff[2] = d.diff.RmQ.x;
  fp2_batched_inv(inv_diff, 3);
  ixPmQ = inv_diff[0];
  ixPmR = inv_diff[1];
  ixRmQ = inv_diff[2];

  // e(P,Q)^2 (base)
  weil_pairing_odd(&num[0], &den[0], &d.PQ.P.x, &d.ixP, &d.PQ.Q.x, &d.ixQ,
                   &d.PQ.PmQ.x, &ixPmQ, n_minus_1, n_bits, &local_curve.A24);
  // e(P,R)^2 = base^r2
  weil_pairing_odd(&num[1], &den[1], &d.PQ.P.x, &d.ixP, &d.RS.P.x, &d.ixR,
                   &d.diff.PmR.x, &ixPmR, n_minus_1, n_bits, &local_curve.A24);
  // e(R,Q)^2 = base^r1
  weil_pairing_odd(&num[2], &den[2], &d.RS.P.x, &d.ixR, &d.PQ.Q.x, &d.ixQ,
                   &d.diff.RmQ.x, &ixRmQ, n_minus_1, n_bits, &local_curve.A24);

  // Invert the first pairing element to serve as the base inverse
  fp2_t tmp;
  fp2_copy(&tmp, &num[0]);
  fp2_copy(&num[0], &den[0]);
  fp2_copy(&den[0], &tmp);

  fp2_batched_inv(den, 3);

  fp2_t w[3];
  for (int i = 0; i < 3; i++) {
    fp2_mul(&w[i], &num[i], &den[i]);
  }

  fp2_dlog_3e(r2, &w[1], &w[0], e);
  fp2_dlog_3e(r1, &w[2], &w[0], e);

#ifndef NDEBUG
  ec_point_t test;
  ec_biscalar_mul(&test, &local_curve, r1, r2, &d.PQ);
  assert(ec_is_equal(&test, &d.RS.P));
#endif
}

static void tate_pairing_odd(fp2_t *num, fp2_t *den, fp2_t const *XP,
                             fp2_t const *XPinv, fp2_t const *XQ,
                             fp2_t const *XQinv, fp2_t const *XPQ,
                             fp2_t const *XPQinv, const digit_t *n_minus_1,
                             int n_bits, ec_point_t const *A24) {
  struct cubical_ladder_result res;
  cubical_ladder_odd(&res, n_minus_1, n_bits, XPQ, XPQinv, XP, XPinv, XQinv,
                     A24);
  fp2_copy(num, &res.Xn1P);
  fp2_copy(den, &res.Zn1PQ);
}

// Apply the (p-1) * 2^POWER_OF_2 * 3^(POWER_OF_3 - e) exponentiation
// to an element already in mu_{p+1}, i.e. u^(p-1) has already been computed.
// Input: u = x^(p-1) for some x in F_{p^2}*
// Output: r = x^((p-1) * 2^POWER_OF_2 * 3^(POWER_OF_3 - e))  in mu_{3^e}
static void apply_tate_exp_3(fp2_t *r, const fp2_t *u, int e) {
  fp2_t v;
  fp2_copy(&v, u);

  // 2^POWER_OF_2 exponentiation (clears the 2-part of |mu_{p+1}|)
  for (int i = 0; i < POWER_OF_2; i++) {
    fp2_sqr(&v, &v);
  }

  // 3^(POWER_OF_3 - e) exponentiation (clears the excess 3-cofactor)
  int diff = POWER_OF_3 - e;
  for (int i = 0; i < diff; i++) {
    fp2_t sq;
    fp2_sqr(&sq, &v);
    fp2_mul(&v, &sq, &v);
  }

  fp2_copy(r, &v);
}

void ec_dlog_3_tate(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                    const ec_basis_t *PQ, const ec_basis_t *RS,
                    const ec_curve_t *curve, int e) {
  // count_ec_dlog_3_tate++;
  // active_fp2_counters = &count_ec_dlog_3_tate_fp2;
  ec_curve_t local_curve = *curve;
  if (!fp2_is_one(&local_curve.A24.z) || fp2_is_zero(&local_curve.A24.z)) {
    A24_from_AC(&local_curve.A24, &local_curve);
  }

  pairing_dlog_params_3_t d;
  d.e = e;
  d.PQ = *PQ;
  d.RS = *RS;

  normalize_params_3(&d, &local_curve);
  compute_difference_points_3(&d, &local_curve);

  // Re-normalize diff points to Z=1 after computation
  fp2_t t_diff[4];
  t_diff[0] = d.diff.PmR.z;
  t_diff[1] = d.diff.PmS.z;
  t_diff[2] = d.diff.RmQ.z;
  t_diff[3] = d.diff.SmQ.z;
  fp2_batched_inv(t_diff, 4);
  fp2_mul(&d.diff.PmR.x, &d.diff.PmR.x, &t_diff[0]);
  fp2_set_one(&d.diff.PmR.z);
  fp2_mul(&d.diff.PmS.x, &d.diff.PmS.x, &t_diff[1]);
  fp2_set_one(&d.diff.PmS.z);
  fp2_mul(&d.diff.RmQ.x, &d.diff.RmQ.x, &t_diff[2]);
  fp2_set_one(&d.diff.RmQ.z);
  fp2_mul(&d.diff.SmQ.x, &d.diff.SmQ.x, &t_diff[3]);
  fp2_set_one(&d.diff.SmQ.z);

  digit_t n_minus_1[NWORDS_ORDER];
  int n_bits = compute_n_minus_1(n_minus_1, e);

  fp2_t num[5], den[5];
  fp2_t ixPmQ, ixPmR, ixPmS, ixRmQ, ixSmQ;
  fp2_t inv_diff[5];
  inv_diff[0] = d.PQ.PmQ.x;
  inv_diff[1] = d.diff.PmR.x;
  inv_diff[2] = d.diff.PmS.x;
  inv_diff[3] = d.diff.RmQ.x;
  inv_diff[4] = d.diff.SmQ.x;
  fp2_batched_inv(inv_diff, 5);
  ixPmQ = inv_diff[0];
  ixPmR = inv_diff[1];
  ixPmS = inv_diff[2];
  ixRmQ = inv_diff[3];
  ixSmQ = inv_diff[4];

  tate_pairing_odd(&num[0], &den[0], &d.PQ.P.x, &d.ixP, &d.PQ.Q.x, &d.ixQ,
                   &d.PQ.PmQ.x, &ixPmQ, n_minus_1, n_bits, &local_curve.A24);
  tate_pairing_odd(&num[1], &den[1], &d.PQ.P.x, &d.ixP, &d.RS.P.x, &d.ixR,
                   &d.diff.PmR.x, &ixPmR, n_minus_1, n_bits, &local_curve.A24);
  tate_pairing_odd(&num[2], &den[2], &d.RS.P.x, &d.ixR, &d.PQ.Q.x, &d.ixQ,
                   &d.diff.RmQ.x, &ixRmQ, n_minus_1, n_bits, &local_curve.A24);
  tate_pairing_odd(&num[3], &den[3], &d.PQ.P.x, &d.ixP, &d.RS.Q.x, &d.ixS,
                   &d.diff.PmS.x, &ixPmS, n_minus_1, n_bits, &local_curve.A24);
  tate_pairing_odd(&num[4], &den[4], &d.RS.Q.x, &d.ixS, &d.PQ.Q.x, &d.ixQ,
                   &d.diff.SmQ.x, &ixSmQ, n_minus_1, n_bits, &local_curve.A24);

  // Compute u_i^(p-1) = frob(num_i)*den_i / (frob(den_i)*num_i) for all i.
  // Batch-invert the 5 denominators plus a placeholder slot for base_inv (6
  // total).
  fp2_t numer[5], denom[6]; // denom[5] will hold w[0] for base_inv
  for (int i = 0; i < 5; i++) {
    fp2_t fn, fd;
    fp2_frob(&fn, &num[i]);           // fn = frob(num_i)
    fp2_frob(&fd, &den[i]);           // fd = frob(den_i)
    fp2_mul(&numer[i], &fn, &den[i]); // numer_i = frob(num_i)*den_i
    fp2_mul(&denom[i], &fd, &num[i]); // denom_i = frob(den_i)*num_i
  }

  // Apply exponentiation to get u_i^(p-1) then project into mu_{3^e}
  fp2_t w[5];
  fp2_batched_inv(denom, 5); // invert the 5 denominators
  for (int i = 0; i < 5; i++) {
    fp2_t u_pm1;
    fp2_mul(&u_pm1, &numer[i], &denom[i]); // u_i^(p-1)
    apply_tate_exp_3(&w[i], &u_pm1, e);
  }

  // Invert w[0] to serve as the base inverse for fp2_dlog_3e
  // Since w[0] is in the cyclotomic subgroup, its inverse is its conjugate.
  fp2_t base_inv;
  fp2_copy(&base_inv, &w[0]);
  fp_neg(base_inv.im, base_inv.im);

  fp2_dlog_3e(r2, &w[1], &base_inv, e);
  fp2_dlog_3e(r1, &w[2], &base_inv, e);
  fp2_dlog_3e(s2, &w[3], &base_inv, e);
  fp2_dlog_3e(s1, &w[4], &base_inv, e);

#ifndef NDEBUG
  ec_point_t test;
  ec_biscalar_mul(&test, &local_curve, r1, r2, &d.PQ);
  assert(ec_is_equal(&test, &d.RS.P));
  ec_biscalar_mul(&test, &local_curve, s1, s2, &d.PQ);
  assert(ec_is_equal(&test, &d.RS.Q));
#endif
  // active_fp2_counters = NULL;
}

void ec_dlog_3_tate_R(digit_t *r1, digit_t *r2, const ec_basis_t *PQ3,
                      const ec_point_t *R, const ec_curve_t *curve, int e) {
  // count_ec_dlog_3_tate_R++;
  // active_fp2_counters = &count_ec_dlog_3_tate_R_fp2;
  ec_curve_t local_curve = *curve;
  if (!fp2_is_one(&local_curve.A24.z) || fp2_is_zero(&local_curve.A24.z)) {
    A24_from_AC(&local_curve.A24, &local_curve);
  }

  pairing_dlog_params_3_t d;
  d.e = e;
  d.PQ = *PQ3;
  d.RS.P = *R;

  normalize_params_3_R(&d, &local_curve);
  compute_difference_points_3_R(&d, &local_curve);

  // Re-normalize diff points
  fp2_t t_diff[2];
  t_diff[0] = d.diff.PmR.z;
  t_diff[1] = d.diff.RmQ.z;
  fp2_batched_inv(t_diff, 2);
  fp2_mul(&d.diff.PmR.x, &d.diff.PmR.x, &t_diff[0]);
  fp2_set_one(&d.diff.PmR.z);
  fp2_mul(&d.diff.RmQ.x, &d.diff.RmQ.x, &t_diff[1]);
  fp2_set_one(&d.diff.RmQ.z);

  digit_t n_minus_1[NWORDS_ORDER];
  int n_bits = compute_n_minus_1(n_minus_1, e);

  fp2_t num[3], den[3];
  fp2_t ixPmQ, ixPmR, ixRmQ;
  fp2_t inv_diff[3];
  inv_diff[0] = d.PQ.PmQ.x;
  inv_diff[1] = d.diff.PmR.x;
  inv_diff[2] = d.diff.RmQ.x;
  fp2_batched_inv(inv_diff, 3);
  ixPmQ = inv_diff[0];
  ixPmR = inv_diff[1];
  ixRmQ = inv_diff[2];

  tate_pairing_odd(&num[0], &den[0], &d.PQ.P.x, &d.ixP, &d.PQ.Q.x, &d.ixQ,
                   &d.PQ.PmQ.x, &ixPmQ, n_minus_1, n_bits, &local_curve.A24);
  tate_pairing_odd(&num[1], &den[1], &d.PQ.P.x, &d.ixP, &d.RS.P.x, &d.ixR,
                   &d.diff.PmR.x, &ixPmR, n_minus_1, n_bits, &local_curve.A24);
  tate_pairing_odd(&num[2], &den[2], &d.RS.P.x, &d.ixR, &d.PQ.Q.x, &d.ixQ,
                   &d.diff.RmQ.x, &ixRmQ, n_minus_1, n_bits, &local_curve.A24);

  // Compute u_i^(p-1) = frob(num_i)*den_i / (frob(den_i)*num_i) for all i.
  fp2_t numer[3], denom[3];
  for (int i = 0; i < 3; i++) {
    fp2_t fn, fd;
    fp2_frob(&fn, &num[i]);           // fn = frob(num_i)
    fp2_frob(&fd, &den[i]);           // fd = frob(den_i)
    fp2_mul(&numer[i], &fn, &den[i]); // numer_i = frob(num_i)*den_i
    fp2_mul(&denom[i], &fd, &num[i]); // denom_i = frob(den_i)*num_i
  }

  fp2_t w[3];
  fp2_batched_inv(denom, 3); // invert the 3 denominators
  for (int i = 0; i < 3; i++) {
    fp2_t u_pm1;
    fp2_mul(&u_pm1, &numer[i], &denom[i]); // u_i^(p-1)
    apply_tate_exp_3(&w[i], &u_pm1, e);
  }

  fp2_t base_inv;
  fp2_copy(&base_inv, &w[0]);
  fp_neg(base_inv.im, base_inv.im);

  fp2_dlog_3e(r2, &w[1], &base_inv, e);
  fp2_dlog_3e(r1, &w[2], &base_inv, e);

#ifndef NDEBUG
  ec_point_t test;
  ec_biscalar_mul(&test, &local_curve, r1, r2, &d.PQ);
  assert(ec_is_equal(&test, &d.RS.P));
#endif
  // active_fp2_counters = NULL;
}
