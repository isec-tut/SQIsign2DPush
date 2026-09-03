#include <curve_extras.h>
#include <fp.h>
#include <sqisign2dpush.h>
#include <time.h>

#include <inttypes.h>

static void fp_print(char *name, fp_t const a) {
  fp_t b;
  fp_set(b, 1);
  fp_mul(b, b, a);
  printf("%s0x", name);
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016llx", (unsigned long long)b[i]);
}

static void fp2_print(char *name, fp2_t const a) {
  fp2_t b;
  fp2_set(&b, 1);
  fp2_mul(&b, &b, &a);
  // fp2_frommont(&b, &a);
  printf("%s0x", name);
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016llx", (unsigned long long)b.re[i]);
  printf(" + i*0x");
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016llx", (unsigned long long)b.im[i]);
}
static void point_print(char *name, ec_point_t P) {
  fp2_t a;
  if (fp2_is_zero(&P.z)) {
    printf("%s = INF\n", name);
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

void ec_point_init(
    ec_point_t *P) { // Initialize point as identity element (1:0)
  fp2_set_one(&(P->x));
  fp2_set_zero(&(P->z));
}

void secret_key_init(secret_key_t *sk) {
  quat_left_ideal_init(&(sk->secret_ideal_two));
  quat_alg_elem_init(&(sk->two_to_three_transporter));
  // ibz_mat_2x2_init(&(sk->mat_BAcan_to_BA0_two));
  ec_point_init(&(sk->phi_sk2_three.P));
  ec_point_init(&(sk->phi_sk2_three.Q));
  ec_point_init(&(sk->phi_sk2_three.PmQ));
  ibz_mat_2x2_init(&(sk->mat_BAcan_to_BA0_three));
}

void secret_key_finalize(secret_key_t *sk) {
  quat_left_ideal_finalize(&(sk->secret_ideal_two));
  quat_alg_elem_finalize(&(sk->two_to_three_transporter));
  // ibz_mat_2x2_finalize(&(sk->mat_BAcan_to_BA0_two));
  // ec_point_finalize(&(sk->phi_sk2_three.P));
  // ec_point_finalize(&(sk->phi_sk2_three.Q));
  // ec_point_finalize(&(sk->phi_sk2_three.PmQ));
  ibz_mat_2x2_finalize(&(sk->mat_BAcan_to_BA0_three));
}

// Given an x-coordinate, determines if this is a valid
// point on the curve. Assumes C=1.
static uint32_t is_on_curve(const fp2_t *x, const ec_curve_t *curve) {
  assert(fp2_is_one(&curve->C));
  fp2_t t0, one;
  fp2_set_one(&one);

  fp2_add(&t0, x, &curve->A); // x + (A/C)
  fp2_mul(&t0, &t0, x);       // x^2 + (A/C)*x
  fp2_add(&t0, &t0, &one);    // x^2 + (A/C)*x + 1
  fp2_mul(&t0, &t0, x);       // x^3 + (A/C)*x^2 + x

  return fp2_is_square(&t0);
}

void ec_normalize_curve_and_A24(
    ec_curve_t
        *E) { // Neither the curve or A24 are guaranteed to be normalized.
              // First we normalize (A/C : 1) and conditionally compute
  if (!fp2_is_one(&E->C)) {
    ec_normalize_curve(E);
  }

  // Now compute A24 = ((A + 2) / 4 : 1)
  fp2_t one, four;
  fp2_set_one(&one);
  fp2_add(&four, &one, &one);
  fp2_add(&four, &four, &four);

  fp2_add(&E->A24.x, &E->A, &one);     // re(A24.x) = re(A) + 2
  fp2_add(&E->A24.x, &E->A24.x, &one); // re(A24.x) = re(A) + 2

  fp2_inv(&four);
  fp2_mul(&E->A24.x, &E->A24.x, &four); // (A + 2) / 4
  fp2_set_one(&E->A24.z);
}

void xDBL_A24(
    ec_point_t *Q, const ec_point_t *P,
    const ec_point_t
        *A24) { // Doubling of a Montgomery point in projective coordinates
                // (X:Z). Input: projective Montgomery x-coordinates P =
                // (XP:ZP), where xP=XP/ZP, and
                //        the Montgomery curve constants A24 = (A+2C:4C) (or A24
                //        = (A+2C/4C:1) if normalized).
                // Output: projective Montgomery x-coordinates Q <- 2*P =
                // (XQ:ZQ) such that x(2P)=XQ/ZQ.
  fp2_t t0, t1, t2;

  fp2_add(&t0, &P->x, &P->z);
  fp2_sqr(&t0, &t0);
  fp2_sub(&t1, &P->x, &P->z);
  fp2_sqr(&t1, &t1);
  fp2_sub(&t2, &t0, &t1);
  // if (!A24_normalized)
  //     fp2_mul(&t1, &t1, &A24->z);
  fp2_mul(&Q->x, &t0, &t1);
  fp2_mul(&t0, &t2, &A24->x);
  fp2_add(&t0, &t0, &t1);
  fp2_mul(&Q->z, &t0, &t2);
}

// Helper function which given a point of order k*2^n with n maximal
// and k odd, computes a point of order 2^f
static inline void
clear_cofactor_for_maximal_even_order(ec_point_t *P, ec_curve_t *curve, int f) {
  // clear out the odd cofactor to get a point of order 2^n
  xMULv2(P, P, p_cofactor_for_2f, P_COFACTOR_FOR_2F_BITLENGTH, &curve->A24);

  // clear the power of two to get a point of order 2^f
  for (int i = 0; i < TORSION_PLUS_EVEN_POWER - f; i++) {
    xDBL_A24(P, P, &curve->A24);
  }
}

// Helper function which finds an NQR -1 / (1 + i*b) for entangled basis
// generation
static uint8_t find_nqr_factor(fp2_t *x, ec_curve_t *curve,
                               const uint8_t start) {
  // factor = -1/(1 + i*b) for b in Fp will be NQR whenever 1 + b^2 is NQR
  // in Fp, so we find one of these and then invert (1 + i*b). We store b
  // as a u8 hint to save time in verification.

  // We return the hint as a u8, but use (uint16_t)n to give 2^16 - 1
  // to make failure cryptographically negligible, with a fallback when
  // n > 128 is required.
  uint8_t hint;
  uint32_t found = 0;
  uint16_t n = start;

  bool qr_b = 1;
  fp_t b, tmp;
  fp2_t z, t0, t1;

  do {
    while (qr_b) {
      // find b with 1 + b^2 a non-quadratic residue
      fp_set(tmp, (uint32_t)n * n + 1);
      fp_tomont(tmp, tmp);
      qr_b = fp_is_square(tmp);
      n++; // keeps track of b = n - 1
    }

    // for Px := -A/(1 + i*b) to be on the curve
    // is equivalent to A^2*(z-1) - z^2 NQR for z = 1 + i*b
    // thus prevents unnecessary inversion pre-check

    // t0 = z - 1 = i*b
    // t1 = z = 1 + i*b;
    fp_set(b, (uint32_t)n - 1);
    fp_tomont(b, b);
    fp2_set_zero(&t0);
    fp2_set_one(&z);
    fp_copy(z.im, b);
    fp_copy(t0.im, b);

    // A^2*(z-1) - z^2
    fp2_sqr(&t1, &curve->A);
    fp2_mul(&t0, &t0, &t1); // A^2 * (z - 1)
    fp2_sqr(&t1, &z);
    fp2_sub(&t0, &t0, &t1); // A^2 * (z - 1) - z^2
    found = !fp2_is_square(&t0);

    qr_b = 1;
  } while (!found);

  // set Px to -A/(1 + i*b)
  fp2_copy(x, &z);
  fp2_inv(x);
  fp2_mul(x, x, &curve->A);
  fp2_neg(x, x);

  /*
   * With very low probability n will not fit in 7 bits.
   * We set hint = 0 which signals failure and the need
   * to generate a value on the fly during verification
   */
  hint = n <= 128 ? n - 1 : 0;
  if (hint == 0) {
    printf("hint is 0\n");
  }
  return hint;
}

// Helper function which finds a point x(P) = n * A
static uint8_t find_nA_x_coord(fp2_t *x, ec_curve_t *curve,
                               const uint8_t start) {
  assert(!fp2_is_square(&curve->A)); // Only to be called when A is a NQR

  // when A is NQR we allow x(P) to be a multiple n*A of A
  uint8_t n = start;
  fp2_t tmp;
  if (n == 1) {
    fp2_copy(x, &curve->A);
  } else {
    printf("find_nA_x_coord: n = %d\n", n);
    fp2_set(&tmp, n);
    fp2_tomont(&tmp, &tmp);
    fp2_mul(x, &curve->A, &tmp);
  }

  while (!is_on_curve(x, curve)) {
    fp2_add(x, x, &curve->A);
    n++;
  }

  /*
   * With very low probability (1/2^128), n will not fit in 7 bits.
   * In this case, we set hint = 0 which signals failure and the need
   * to generate a value on the fly during verification
   */
  uint8_t hint = n < 128 ? n : 0;
  if (hint == 0) {
    printf("hint is 0\n");
  }
  return hint;
}

void xDBL_E0(ec_point_t *Q,
             const ec_point_t *P) { // Doubling of a Montgomery point in
                                    // projective coordinates (X : Z)
  // on the curve E0 with (A:C) = (0:1). Input: projective
  // Montgomery x-coordinates P = (XP:ZP), where xP=XP/ZP, and
  // Montgomery curve constants (A:C) = (0:1). Output: projective
  // Montgomery x-coordinates Q <- 2*P = (XQ:ZQ) such that x(2P)=XQ/ZQ.
  fp2_t t0, t1, t2;

  fp2_add(&t0, &P->x, &P->z);
  fp2_sqr(&t0, &t0);
  fp2_sub(&t1, &P->x, &P->z);
  fp2_sqr(&t1, &t1);
  fp2_sub(&t2, &t0, &t1);
  fp2_add(&t1, &t1, &t1);
  fp2_mul(&Q->x, &t0, &t1);
  fp2_add(&Q->z, &t1, &t2);
  fp2_mul(&Q->z, &Q->z, &t2);
}

// The entangled basis generation does not allow A = 0
// so we simply return the one we have already precomputed
static void ec_basis_E0_2f(ec_basis_t *PQ2, ec_curve_t *curve, int f) {
  assert(fp2_is_zero(&curve->A));
  ec_point_t P, Q;

  // Set P, Q to precomputed (X : 1) values
  fp2_copy(&P.x, &BASIS_EVEN.P.x);
  fp2_copy(&Q.x, &BASIS_EVEN.Q.x);
  fp2_set_one(&P.z);
  fp2_set_one(&Q.z);

  // clear the power of two to get a point of order 2^f
  for (int i = 0; i < TORSION_PLUS_EVEN_POWER - f; i++) {
    xDBL_E0(&P, &P);
    xDBL_E0(&Q, &Q);
  }

  // Set P, Q in the basis and compute x(P - Q)
  copy_point(&PQ2->P, &P);
  copy_point(&PQ2->Q, &Q);
  difference_point(&PQ2->PmQ, &P, &Q, curve);
}

// Computes a basis E[2^f] = <P, Q> where the point Q is above (0 : 0)
// and stores hints as an array for faster recomputation at a later point
uint8_t ec_curve_to_basis_2f_to_hint(ec_basis_t *PQ2, ec_curve_t *curve,
                                     int f) {
  // Normalise (A/C : 1) and ((A + 2)/4 : 1)
  ec_normalize_curve_and_A24(curve);

  if (fp2_is_zero(&curve->A)) {
    ec_basis_E0_2f(PQ2, curve, f);
    return 0;
  }

  uint8_t hint;
  bool hint_A = fp2_is_square(&curve->A);

  // Compute the points P, Q
  ec_point_t P, Q;

  if (!hint_A) {
    // when A is NQR we allow x(P) to be a multiple n*A of A
    // printf("A is NQR\n");
    hint = find_nA_x_coord(&P.x, curve, 1);
  } else {
    // when A is QR we instead have to find (1 + b^2) a NQR
    // such that x(P) = -A / (1 + i*b)
    // printf("A is QR\n");
    hint = find_nqr_factor(&P.x, curve, 1);
  }

  fp2_set_one(&P.z);
  fp2_add(&Q.x, &curve->A, &P.x);
  fp2_neg(&Q.x, &Q.x);
  fp2_set_one(&Q.z);

  // assert(ec_is_on_curve(curve, &P));
  // assert(ec_is_on_curve(curve, &Q));

  // clear out the odd cofactor to get a point of order 2^f
  clear_cofactor_for_maximal_even_order(&P, curve, f);
  clear_cofactor_for_maximal_even_order(&Q, curve, f);

  // assert(ec_is_on_curve(curve, &P));
  // assert(ec_is_on_curve(curve, &Q));

  // compute PmQ, set PmQ to Q to ensure Q above (0,0)
  difference_point(&PQ2->Q, &P, &Q, curve);
  copy_point(&PQ2->P, &P);
  copy_point(&PQ2->PmQ, &Q);

  // Finally, we compress hint_A and hint into a single bytes.
  // We choose to set the LSB of hint to hint_A
  assert(hint < 128); // We expect hint to be 7-bits in size
  return (hint << 1) | hint_A;
}

void protocols_keygen(public_key_t *pk, secret_key_t *sk,
                      keygen_timings_t *timings) {

  quat_left_ideal_t lideal_odd;
  quat_alg_elem_t gamma;
  ec_isog_even_t two_isogeny_first_half, two_isogeny_second_half;
  ec_isog_odd_t phi_first_half, phi_second_half;
  ec_point_t list_points[3];
  ec_basis_t B_can_two, B_can_three, B_0_two, B_0_three;

  quat_left_ideal_init(&lideal_odd);
  quat_alg_elem_init(&gamma);

  // Initialize sub-step timing accumulators
  if (timings) {
    timings->ms_eval_even = 0.f;
    timings->ms_eval_three = 0.f;
    timings->total_isog_length_even = 0UL;
    timings->total_isog_length_three = 0UL;
    timings->ms_keygen_isog = 0.f;
    timings->ms_doublepath = 0.f;
    timings->ms_basis_two_hint = 0.f;
    timings->ms_change_basis_two = 0.f;
    timings->ms_basis_three_hint = 0.f;
    timings->ms_change_basis_three = 0.f;
    timings->ms_dp_represent_integer = 0.f;
    timings->ms_dp_lideal_create = 0.f;
    timings->ms_dp_quat_to_isog_two = 0.f;
    timings->ms_dp_quat_to_kernel_three = 0.f;
    timings->ms_dp_quat_to_isog_three = 0.f;
    timings->ms_dp_quat_to_kernel_two = 0.f;
    timings->ms_dp_eval_even1 = 0.f;
    timings->ms_dp_isog_init_three = 0.f;
    timings->ms_dp_biscalar1 = 0.f;
    timings->ms_dp_complete_three_basis = 0.f;
    timings->ms_dp_curve_to_basis_2 = 0.f;
    timings->ms_dp_eval_three1 = 0.f;
    timings->ms_dp_eval_three2 = 0.f;
    timings->ms_dp_biscalar2 = 0.f;
    timings->ms_dp_isog_init_two = 0.f;
    timings->ms_dp_complete_two_basis = 0.f;
    timings->ms_dp_curve_to_basis_3 = 0.f;
    timings->ms_dp_eval_even2 = 0.f;
    timings->ms_dp_dlog_3_tate_R = 0.f;
    timings->ms_dp_biscalar3 = 0.f;
    timings->ms_dp_eval_three3 = 0.f;
    timings->ms_dp_dlog_2_tate_R = 0.f;
    timings->ms_dp_biscalar4 = 0.f;
    timings->ms_dp_eval_even3 = 0.f;
  }

  // ---- doublepath ----
  clock_t t_start = clock();
  doublepath(&gamma, &(sk->secret_ideal_two), &lideal_odd, &B_0_three, &B_0_two,
             &(sk->curve), 0, timings);
  clock_t t_end = clock();
  if (timings)
    timings->ms_doublepath =
        (float)(t_end - t_start) * 1000.f / (float)CLOCKS_PER_SEC;

#ifndef NDEBUG
  assert(sqisign2dpush_test_point_order_twof(&(B_0_two.P), &(sk->curve)));
  assert(sqisign2dpush_test_point_order_twof(&(B_0_two.Q), &(sk->curve)));
  assert(sqisign2dpush_test_point_order_twof(&(B_0_two.PmQ), &(sk->curve)));

  assert(sqisign2dpush_test_point_order_threef(&(B_0_three.P), &(sk->curve)));
  assert(sqisign2dpush_test_point_order_threef(&(B_0_three.Q), &(sk->curve)));
  assert(sqisign2dpush_test_point_order_threef(&(B_0_three.PmQ), &(sk->curve)));
#endif

  // sk->two_to_three_transporter = conj(gamma)/power_of_2
  quat_alg_conj(&(sk->two_to_three_transporter), &gamma);
  ibz_mul(&(sk->two_to_three_transporter.denom),
          &(sk->two_to_three_transporter.denom), &(sk->secret_ideal_two.norm));

#ifndef NDEBUG
  {
    quat_left_ideal_t lideal_odd_again;
    quat_left_ideal_init(&lideal_odd_again);

    quat_lideal_mul(&lideal_odd_again, &(sk->secret_ideal_two),
                    &(sk->two_to_three_transporter), &QUATALG_PINFTY, 0);

    assert(quat_lideal_equals(&lideal_odd_again, &lideal_odd, &QUATALG_PINFTY));

    quat_left_ideal_finalize(&lideal_odd_again);
  }
#endif

  copy_curve(&(pk->curve), &(sk->curve));

  // ---- ec_curve_to_basis_2f_to_hint ----
  // t_start = clock();
  // pk->hint_pk_even = ec_curve_to_basis_2f_to_hint(&B_can_two, &(pk->curve),
  //                                                 TORSION_PLUS_EVEN_POWER);
  // t_end = clock();
  // if (timings)
  //   timings->ms_basis_two_hint =
  //       (float)(t_end - t_start) * 1000.f / (float)CLOCKS_PER_SEC;

// #ifndef NDEBUG
//   assert(sqisign2dpush_test_point_order_twof(&(B_can_two.P), &(sk->curve)));
//   assert(sqisign2dpush_test_point_order_twof(&(B_can_two.Q), &(sk->curve)));
//   assert(sqisign2dpush_test_point_order_twof(&(B_can_two.PmQ), &(sk->curve)));
// #endif

  copy_point(&(sk->phi_sk2_three.P), &B_0_two.P);
  copy_point(&(sk->phi_sk2_three.Q), &B_0_two.Q);
  copy_point(&(sk->phi_sk2_three.PmQ), &B_0_two.PmQ);

  // ---- change_of_basis_matrix_two ----
  // t_start = clock();

  // change_of_basis_matrix_two(&(sk->mat_BAcan_to_BA0_two), &B_can_two, &B_0_two,
  //                            &(sk->curve));
  // t_end = clock();
  // if (timings)
  //   timings->ms_change_basis_two =
  //       (float)(t_end - t_start) * 1000.f / (float)CLOCKS_PER_SEC;

  // ---- ec_curve_to_basis_3f_to_hint ----
  t_start = clock();
  pk->hint_pk_three = ec_curve_to_basis_3f_to_hint(&B_can_three, &(pk->curve));
  t_end = clock();
  if (timings)
    timings->ms_basis_three_hint =
        (float)(t_end - t_start) * 1000.f / (float)CLOCKS_PER_SEC;

#ifndef NDEBUG
  assert(sqisign2dpush_test_point_order_threef(&(B_can_three.P), &(sk->curve)));
  assert(sqisign2dpush_test_point_order_threef(&(B_can_three.Q), &(sk->curve)));
  assert(
      sqisign2dpush_test_point_order_threef(&(B_can_three.PmQ), &(sk->curve)));
#endif

  // ---- change_of_basis_matrix_three ----
  t_start = clock();
  change_of_basis_matrix_three(&(sk->mat_BAcan_to_BA0_three), &B_can_three,
                               &B_0_three, &(sk->curve));

  t_end = clock();
  if (timings)
    timings->ms_change_basis_three =
        (float)(t_end - t_start) * 1000.f / (float)CLOCKS_PER_SEC;

  quat_left_ideal_finalize(&lideal_odd);
  quat_alg_elem_finalize(&gamma);
  return;
}
