#include <encoded_sizes.h>
#include <curve_extras.h>
#include <ec.h>
#include <fips202.h>
#include <fp2.h>
#include <hd.h>
#include <isog.h>
#include <sqisign2dpush.h>
#include <stdio.h>
#include <string.h>
#include <tedwards.h>
#include <time.h>
#include <toolbox.h>

// extern uint64_t theta_measure_JacDBL;
// extern uint64_t theta_measure_ThetaDBL;
// extern uint64_t theta_measure_gluingCodomain;
// extern uint64_t theta_measure_gluingEvalpts;
// extern uint64_t theta_measure_gluingEvalStratpts;
// extern uint64_t theta_measure_ThetaIsogCompute;
// extern uint64_t theta_measure_ThetaIsogEvalpts;
// extern uint64_t theta_measure_ThetaIsogEvalStratpts;
// extern uint64_t theta_measure_SplittingCompute;

#define EXPONENT_TWO TORSION_PLUS_EVEN_POWER
#define EXPONENT_THREE TORSION_PLUS_ODD_POWERS[0]

#define SIGN_TIME_CATEGORY(category, ...)                                      \
  do {                                                                         \
    if (timings) {                                                             \
      clock_t _ts = clock();                                                   \
      __VA_ARGS__;                                                             \
      timings->category +=                                                     \
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;             \
    } else {                                                                   \
      __VA_ARGS__;                                                             \
    }                                                                          \
  } while (0)

const clock_t time_isogenies_odd = 0;
const clock_t time_sample_response = 0;
const clock_t time_change_of_basis_matrix = 0;

// void ec_point_init(
//     ec_point_t *P) { // Initialize point as identity element (1:0)
//   fp2_set_one(&(P->x));
//   fp2_set_zero(&(P->z));
// }

void secret_sig_init(signature_t *sig) {
  // ibz_init(&(sig->challenge));
  for (int i = 0; i < NWORDS_ORDER; i++) {
    sig->challenge[i] = 0;
  }
  // sig->n1 = -1;
  ibz_mat_2x2_init(&(sig->mat_rsp));
}

void secret_sig_finalize(signature_t *sig) {
  // ibz_finalize(&(sig->challenge));
  ibz_mat_2x2_finalize(&(sig->mat_rsp));
}

static void ibz_vec_2_print2(char *name, const ibz_vec_2_t *vec) {
  printf("%s", name);
  for (int i = 0; i < 2; i++) {
    ibz_printf("%Zd ", &((*vec)[i]));
  }
  ibz_printf("\n");
}

static void ibz_vec_4_print2(char *name, const ibz_vec_4_t *vec) {
  printf("%s", name);
  for (int i = 0; i < 4; i++) {
    ibz_printf("%Zd ", &((*vec)[i]));
  }
  ibz_printf("\n");
}

// static void ibz_mat_2x2_print(const ibz_mat_2x2_t *mat) {
//   printf("matrix: ");
//   for (int i = 0; i < 2; i++) {
//     for (int j = 0; j < 2; j++) {
//       ibz_printf("%Zd ", &((*mat)[i][j]));
//     }
//     printf("\n        ");
//   }
//   printf("\n");
// }

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

void print_signature(const signature_t *sig) {
  ibz_t challenge;
  ibz_init(&challenge);
  ibz_copy_digit_array(&challenge, sig->challenge);
  // fp2_t j;
  // ec_j_inv(&j, &sig->E_aux);
  // fp2_print("j_Eaux = ", j);
  curve_print("E_aux = ", sig->E_aux);
  printf("\n");
  ibz_printf("challenge = %Zd\n", &challenge);
  printf("\n");
  // ibz_mat_2x2_print(&sig->mat_sigma_phichall);
  ibz_printf("M_rsp[00] = %Zd, ", &((sig->mat_rsp)[0][0]));
  printf("\n");
  ibz_printf("M_rsp[01] = %Zd, ", &((sig->mat_rsp)[0][1]));
  printf("\n");
  ibz_printf("M_rsp[10] = %Zd, ", &((sig->mat_rsp)[1][0]));
  printf("\n");
  ibz_printf("M_rsp[11] = %Zd", &((sig->mat_rsp)[1][1]));
  printf("\n");
  printf("n1 = %d\n", sig->n1);
  printf("hint_aux = %d\n", sig->hint_aux);
  printf("hint_chall = %d\n", sig->hint_chall);
  ibz_finalize(&challenge);
}

void print_public_key(const public_key_t *pk) {
  fp2_t j;
  ec_j_inv(&j, &pk->curve);
  fp2_print("j_EA = ", j);
  printf("\n");
  // printf("hint_pk_even = %d\n", pk->hint_pk_even);
  printf("hint_pk_three = %d\n", pk->hint_pk_three);
}

void print_secret_key(const secret_key_t *sk) {
  curve_print("E_pk = ", sk->curve);
  printf("\n");
  printf("secret_ideal_two:\n");
  quat_left_ideal_print(&sk->secret_ideal_two);
  printf("two_to_three_transporter:\n");
  quat_alg_elem_print(&sk->two_to_three_transporter);
  // printf("mat_BAcan_to_BA0_two:\n");
  // ibz_mat_2x2_print(&sk->mat_BAcan_to_BA0_two);
  printf("mat_BAcan_to_BA0_three:\n");
  ibz_mat_2x2_print(&sk->mat_BAcan_to_BA0_three);
  // point_print("phi_sk_even.P = ", sk->phi_sk_even.P);
  // printf("\n");
  // point_print("phi_sk_even.Q = ", sk->phi_sk_even.Q);
  // printf("\n");
  // point_print("phi_sk_even.PmQ = ", sk->phi_sk_even.PmQ);
  // printf("\n");
  // point_print("phi_sk_three.P = ", sk->phi_sk_three.P);
  // printf("\n");
  // point_print("phi_sk_three.Q = ", sk->phi_sk_three.Q);
  // printf("\n");
  // point_print("phi_sk_three.PmQ = ", sk->phi_sk_three.PmQ);
  // printf("\n");
}

int point_order_2f(const ec_point_t *P, const ec_curve_t *E, int t) {
  ec_point_t test;
  copy_point(&test, P);
  if (fp2_is_zero(&test.z))
    return 0;
  for (int i = 0; i < t - 1; i++) {
    ec_dbl(&test, E, &test);
  }
  if (fp2_is_zero(&test.z))
    return 0;
  ec_dbl(&test, E, &test);
  return (fp2_is_zero(&test.z));
}

int point_order_3f(const ec_point_t *P, const ec_curve_t *E, int t) {
  ec_point_t test;
  copy_point(&test, P);
  digit_t three[NWORDS_ORDER] = {0};
  three[0] = 3;
  if (fp2_is_zero(&test.z))
    return 0;
  for (int i = 0; i < t - 1; i++) {
    ec_mul(&test, E, three, &test);
  }
  if (fp2_is_zero(&test.z))
    return 0;
  ec_mul(&test, E, three, &test);
  return (fp2_is_zero(&test.z));
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

// void ec_normalize_curve_and_A24(
//     ec_curve_t
//         *E) { // Neither the curve or A24 are guaranteed to be normalized.
//               // First we normalize (A/C : 1) and conditionally compute
//   if (!fp2_is_one(&E->C)) {
//     ec_normalize_curve(E);
//   }

//   // Now compute A24 = ((A + 2) / 4 : 1)
//   fp2_t one, four;
//   fp2_set_one(&one);
//   fp2_add(&four, &one, &one);
//   fp2_add(&four, &four, &four);

//   fp2_add(&E->A24.x, &E->A, &one);     // re(A24.x) = re(A) + 2
//   fp2_add(&E->A24.x, &E->A24.x, &one); // re(A24.x) = re(A) + 2

//   fp2_inv(&four);
//   fp2_mul(&E->A24.x, &E->A24.x, &four); // (A + 2) / 4
//   fp2_set_one(&E->A24.z);
// }

// void xDBL_A24(
//     ec_point_t *Q, const ec_point_t *P,
//     const ec_point_t
//         *A24) { // Doubling of a Montgomery point in projective coordinates
//                 // (X:Z). Input: projective Montgomery x-coordinates P =
//                 // (XP:ZP), where xP=XP/ZP, and
//                 //        the Montgomery curve constants A24 = (A+2C:4C) (or
//                 A24
//                 //        = (A+2C/4C:1) if normalized).
//                 // Output: projective Montgomery x-coordinates Q <- 2*P =
//                 // (XQ:ZQ) such that x(2P)=XQ/ZQ.
//   fp2_t t0, t1, t2;

//   fp2_add(&t0, &P->x, &P->z);
//   fp2_sqr(&t0, &t0);
//   fp2_sub(&t1, &P->x, &P->z);
//   fp2_sqr(&t1, &t1);
//   fp2_sub(&t2, &t0, &t1);
//   // if (!A24_normalized)
//   //     fp2_mul(&t1, &t1, &A24->z);
//   fp2_mul(&Q->x, &t0, &t1);
//   fp2_mul(&t0, &t2, &A24->x);
//   fp2_add(&t0, &t0, &t1);
//   fp2_mul(&Q->z, &t0, &t2);
// }

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
  fp2_t z, t0, t1, A_sqr;
  fp2_sqr(&A_sqr, &curve->A);

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
    fp2_mul(&t0, &t0, &A_sqr); // A^2 * (z - 1)
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
  return hint;
}

// void xDBL_E0(ec_point_t *Q,
//              const ec_point_t *P) { // Doubling of a Montgomery point in
//                                     // projective coordinates (X : Z)
//   // on the curve E0 with (A:C) = (0:1). Input: projective
//   // Montgomery x-coordinates P = (XP:ZP), where xP=XP/ZP, and
//   // Montgomery curve constants (A:C) = (0:1). Output: projective
//   // Montgomery x-coordinates Q <- 2*P = (XQ:ZQ) such that x(2P)=XQ/ZQ.
//   fp2_t t0, t1, t2;

//   fp2_add(&t0, &P->x, &P->z);
//   fp2_sqr(&t0, &t0);
//   fp2_sub(&t1, &P->x, &P->z);
//   fp2_sqr(&t1, &t1);
//   fp2_sub(&t2, &t0, &t1);
//   fp2_add(&t1, &t1, &t1);
//   fp2_mul(&Q->x, &t0, &t1);
//   fp2_add(&Q->z, &t1, &t2);
//   fp2_mul(&Q->z, &Q->z, &t2);
// }

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

// // Computes a basis E[2^f] = <P, Q> where the point Q is above (0 : 0)
// // and stores hints as an array for faster recomputation at a later point
// uint8_t ec_curve_to_basis_2f_to_hint(ec_basis_t *PQ2, ec_curve_t *curve,
//                                      int f) {
//   // Normalise (A/C : 1) and ((A + 2)/4 : 1)
//   ec_normalize_curve_and_A24(curve);

//   if (fp2_is_zero(&curve->A)) {
//     ec_basis_E0_2f(PQ2, curve, f);
//     return 0;
//   }

//   uint8_t hint;
//   bool hint_A = fp2_is_square(&curve->A);

//   // Compute the points P, Q
//   ec_point_t P, Q;

//   if (!hint_A) {
//     // when A is NQR we allow x(P) to be a multiple n*A of A
//     hint = find_nA_x_coord(&P.x, curve, 1);
//   } else {
//     // when A is QR we instead have to find (1 + b^2) a NQR
//     // such that x(P) = -A / (1 + i*b)
//     hint = find_nqr_factor(&P.x, curve, 1);
//   }

//   fp2_set_one(&P.z);
//   fp2_add(&Q.x, &curve->A, &P.x);
//   fp2_neg(&Q.x, &Q.x);
//   fp2_set_one(&Q.z);

//   // assert(ec_is_on_curve(curve, &P));
//   // assert(ec_is_on_curve(curve, &Q));

//   // clear out the odd cofactor to get a point of order 2^f
//   clear_cofactor_for_maximal_even_order(&P, curve, f);
//   clear_cofactor_for_maximal_even_order(&Q, curve, f);

//   // assert(ec_is_on_curve(curve, &P));
//   // assert(ec_is_on_curve(curve, &Q));

//   // compute PmQ, set PmQ to Q to ensure Q above (0,0)
//   difference_point(&PQ2->Q, &P, &Q, curve);
//   copy_point(&PQ2->P, &P);
//   copy_point(&PQ2->PmQ, &Q);

//   // Finally, we compress hint_A and hint into a single bytes.
//   // We choose to set the LSB of hint to hint_A
//   assert(hint < 128); // We expect hint to be 7-bits in size
//   return (hint << 1) | hint_A;
// }

// Computes a basis E[2^f] = <P, Q> where the point Q is above (0 : 0)
// given the hints as an array for faster basis computation
int ec_curve_to_basis_2f_from_hint(ec_basis_t *PQ2, ec_curve_t *curve, int f,
                                   const uint8_t hint) {
  // Normalise (A/C : 1) and ((A + 2)/4 : 1)
  ec_normalize_curve_and_A24(curve);

  if (fp2_is_zero(&curve->A)) {
    ec_basis_E0_2f(PQ2, curve, f);
    return 1;
  }

  // The LSB of hint encodes whether A is a QR
  // The remaining 7-bits are used to find a valid x(P)
  bool hint_A = hint & 1;
  uint8_t hint_P = hint >> 1;

  // Compute the points P, Q
  ec_point_t P, Q;
  fp_t fp_tmp;
  fp2_t fp2_tmp;

  if (!hint_P) {
    // When hint_P = 0 it means we did not find a point in 128 attempts
    // this is very rare and we almost never expect to need this fallback
    // In either case, we can start with b = 128 to skip testing the known
    // values which will not work
    if (!hint_A) {
      find_nA_x_coord(&P.x, curve, 128);
    } else {
      find_nqr_factor(&P.x, curve, 128);
    }
  } else {
    // Otherwise we use the hint to directly find x(P) based on hint_A
    if (!hint_A) {
      // when A is NQR, we have found n such that x(P) = n*A
      fp2_set(&fp2_tmp, hint_P);
      fp2_tomont(&fp2_tmp, &fp2_tmp);
      fp2_mul(&P.x, &curve->A, &fp2_tmp);
    } else {
      // when A is QR we have found b such that (1 + b^2) is a NQR in
      // Fp, so we must compute x(P) = -A / (1 + i*b)
      fp_set(P.x.re, 1);
      fp_tomont(P.x.re, P.x.re);
      fp_set(P.x.im, hint_P);
      fp_tomont(P.x.im, P.x.im);
      fp2_inv(&P.x);
      fp2_mul(&P.x, &P.x, &curve->A);
      fp2_neg(&P.x, &P.x);
    }
  }
  fp2_set_one(&P.z);

#ifndef NDEBUG
  int passed = 1;
  passed = is_on_curve(&P.x, curve);
  passed &= !fp2_is_square(&P.x);

  if (!passed)
    return 0;
#endif

  // set xQ to -xP - A
  fp2_add(&Q.x, &curve->A, &P.x);
  fp2_neg(&Q.x, &Q.x);
  fp2_set_one(&Q.z);

  // clear out the odd cofactor to get a point of order 2^f
  clear_cofactor_for_maximal_even_order(&P, curve, f);
  clear_cofactor_for_maximal_even_order(&Q, curve, f);

  // compute PmQ, set PmQ to Q to ensure Q above (0,0)
  difference_point(&PQ2->Q, &P, &Q, curve);
  copy_point(&PQ2->P, &P);
  copy_point(&PQ2->PmQ, &Q);

#ifndef NDEBUG
  passed &= point_order_2f(&PQ2->P, curve, f);
  passed &= point_order_2f(&PQ2->Q, curve, f);
  passed &= point_order_2f(&PQ2->PmQ, curve, f);

  if (!passed)
    return 0;
#endif

  return 1;
}

int commit(ec_curve_t *E_com, ec_basis_t *basis_even_com,
           quat_left_ideal_t *lideal_commit_three,
           ec_point_t *kernel_iso_three1, ec_point_t *kernel_iso_three2,
           ec_curve_t *E_com_mid, ec_basis_t *basis_two_mid, int verbose,
           sign_timings_t *timings) {

  quat_alg_elem_t gamma;
  quat_left_ideal_t lideal_even;
  ec_isog_even_t two_isogeny_first_half, two_isogeny_second_half;
  ec_isog_odd_t phi_first_half, phi_second_half;
  ec_point_t list_points[3];

  quat_alg_elem_init(&gamma);
  quat_left_ideal_init(&lideal_even);
  // doublepath(&gamma, &lideal_even, lideal_commit_three,
  //            NULL, // not used ?
  //            basis_even_com, E_com,
  //            verbose); // used only for image of BASIS_EVEN
  int ret =
      fastcommit(&gamma, &lideal_even, lideal_commit_three, basis_even_com,
                 E_com, kernel_iso_three1, kernel_iso_three2, E_com_mid,
                 basis_two_mid, verbose, timings); //
                                                   // used
                                                   // only
                                                   // for
                                                   // image
                                                   // of
                                                   // BASIS_EVEN

  // #ifndef NDEBUG
  //     ec_curve_t E_test;
  //     copy_curve(&E_test, &CURVE_E0);
  //     copy_point(list_points + 0, &BASIS_EVEN.P);
  //     copy_point(list_points + 1, &BASIS_EVEN.Q);
  //     copy_point(list_points + 2, &BASIS_EVEN.PmQ);
  //     TAC("ec_eval_odd in");
  //     ec_eval_odd(&E_test, &phi_first_half, list_points, 3);
  //     ec_eval_odd(&E_test, &phi_second_half, list_points, 3);
  //     TAC("ec_eval_odd out");

  //     assert(ec_is_equal(&(basis_even_com->P), list_points + 0));
  //     assert(ec_is_equal(&(basis_even_com->Q), list_points + 1));
  //     assert(ec_is_equal(&(basis_even_com->PmQ), list_points + 2));

  //     fp2_t j_R,j_L;
  //     ec_j_inv(&j_R, &E_test);
  //     ec_j_inv(&j_L, E_com);
  //     assert(fp2_is_equal(&j_R,&j_L));
  // #endif

  quat_alg_elem_finalize(&gamma);
  quat_left_ideal_finalize(&lideal_even);
  return ret;
}

void isog_init_two_f(ec_isog_even_t *isog, const ec_curve_t *curve,
                     const ec_point_t *ker, int length) {
  copy_curve(&(isog->curve), curve);
  copy_point(&(isog->kernel), ker);
#ifdef DEBUG
  assert(point_order_2f(ker, curve, length));
#endif
  isog->length = length;
  return;
}

void isog_init_3(ec_isog_odd_t *isog, const ec_curve_t *curve,
                 const ec_point_t *ker) {
  copy_curve(&(isog->curve), curve);
  copy_point(&(isog->ker_plus), ker);
  ec_set_zero(&(isog->ker_minus));
#ifdef DEBUG
  assert(point_order_3f(ker, curve, 1));
#endif

#define NUMPP                                                                  \
  (sizeof(TORSION_ODD_PRIMEPOWERS) / sizeof(*TORSION_ODD_PRIMEPOWERS))
  for (size_t i = 0; i < NUMPP; ++i) {
    (isog->degree)[i] = 0;
    if (TORSION_ODD_PRIMES[i] == 3) {
      (isog->degree)[i] = 1;
    }
  }
  return;
}

int composed_rand_isog(ec_curve_t *Em__, ec_basis_t *RSm__, ibz_t *d_coprime6,
                       ec_point_t *kernel_phicom1, ibz_t *e_ibz,
                       ibz_t *delta_ibz, ibz_t *pow2, ibz_t *pow3,
                       sign_timings_t *timings) {
  clock_t t_cri_setup = clock();
  int e = (int)ibz_get(e_ibz);
  int delta = (int)ibz_get(delta_ibz);

  ibz_t n_alpha;
  quat_alg_elem_t alpha;
  ibz_init(&n_alpha);
  quat_alg_elem_init(&alpha);

  // n_alpha = d_coprime6*(2^e - d_coprime6)*2^delta*3^EXPONENT_THREE
  ibz_t term2, three_pow;
  ibz_init(&term2);
  ibz_init(&three_pow);

  ibz_set(&term2, 1);
  ibz_mul_2exp(&term2, &term2, (uint64_t)e);
  ibz_sub(&term2, &term2, d_coprime6);

  ibz_mul(&n_alpha, d_coprime6, &term2);
  ibz_mul_2exp(&n_alpha, &n_alpha, (uint64_t)delta);

  ibz_mul(&n_alpha, &n_alpha, pow3);

  quat_alg_elem_t alpha_conj;
  quat_alg_elem_init(&alpha_conj);
  ibz_vec_4_t coeffs;
  ibz_vec_4_init(&coeffs);
  ibz_mat_2x2_t M_alpha, M_alpha_conj;
  ibz_mat_2x2_init(&M_alpha);
  ibz_mat_2x2_init(&M_alpha_conj);
  ibz_t inv_scalar;
  ibz_init(&inv_scalar);
  ec_curve_t E1;
  ec_point_t list_points[3];
  ec_basis_t end_alpha_basis;

  if (timings) {
    timings->ms_sign_quat +=
        (float)(clock() - t_cri_setup) * 1000.f / (float)CLOCKS_PER_SEC;
  }

  copy_point(&end_alpha_basis.P, &BASIS_EVEN.P);
  copy_point(&end_alpha_basis.Q, &BASIS_EVEN.Q);
  copy_point(&end_alpha_basis.PmQ, &BASIS_EVEN.PmQ);

  ec_basis_t basis_even;
  copy_point(&basis_even.P, &BASIS_EVEN.P);
  copy_point(&basis_even.Q, &BASIS_EVEN.Q);
  copy_point(&basis_even.PmQ, &BASIS_EVEN.PmQ);

  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog, {
    for (int i = 0; i < delta; i++) {
      ec_dbl(&basis_even.P, &CURVE_E0, &basis_even.P);
      ec_dbl(&basis_even.Q, &CURVE_E0, &basis_even.Q);
      ec_dbl(&basis_even.PmQ, &CURVE_E0, &basis_even.PmQ);
    }
  });

#ifdef DEBUG
  assert(point_order_2f(&basis_even.P, &CURVE_E0, e));
  assert(point_order_2f(&basis_even.Q, &CURVE_E0, e));
  assert(point_order_2f(&basis_even.PmQ, &CURVE_E0, e));
#endif

  digit_t scalar_d[NWORDS_ORDER];
  ibz_to_digit_array(scalar_d, d_coprime6);

  // ec_mul(&basis_even.P, &CURVE_E0, scalar_d, &basis_even.P);
  // ec_mul(&basis_even.Q, &CURVE_E0, scalar_d, &basis_even.Q);
  // ec_mul(&basis_even.PmQ, &CURVE_E0, scalar_d, &basis_even.PmQ);

  // assert(point_order_2f(&basis_even.P, &CURVE_E0, e));
  // assert(point_order_2f(&basis_even.Q, &CURVE_E0, e));
  // assert(point_order_2f(&basis_even.PmQ, &CURVE_E0, e));

  // inv_pow3_d = (3^e2*d_coprime6)^-1 mod 2^e1
  int res_inv;
  SIGN_TIME_CATEGORY(ms_sign_quat, {
    ibz_mul(&inv_scalar, pow3, d_coprime6);
    res_inv = ibz_invmod(&inv_scalar, &inv_scalar, pow2);
  });
  assert(res_inv);

  ec_isog_odd_t phi3;
  ec_curve_t E0__;
  ec_basis_t RS;

  int ok = 0;

  int found;
  SIGN_TIME_CATEGORY(ms_sign_quat,
                  found = represent_integer(&alpha, &n_alpha, &QUATALG_PINFTY));
  assert(found);

  SIGN_TIME_CATEGORY(ms_sign_quat, {
    quat_alg_conj(&alpha_conj, &alpha);
    from_1ijk_to_O0basis(&coeffs, &alpha_conj);
    matrix_of_endomorphism_even(&M_alpha, &alpha);
  });

  SIGN_TIME_CATEGORY(ms_sign_quat, {
    // M_alpha = M_alpha * inv_scalar * inv_d mod 2^e1
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        ibz_mul(&M_alpha[i][j], &M_alpha[i][j], &inv_scalar);
        // ibz_mul(&M_alpha[i][j], &M_alpha[i][j], &inv_d);
        ibz_mod(&M_alpha[i][j], &M_alpha[i][j], pow2);
      }
    }
  });

  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                  matrix_application_even_basis(&end_alpha_basis, &CURVE_E0,
                                                &M_alpha, EXPONENT_TWO));

  copy_point(list_points + 0, &end_alpha_basis.P);
  copy_point(list_points + 1, &end_alpha_basis.Q);
  copy_point(list_points + 2, &end_alpha_basis.PmQ);

  // Logic for K3 (3^e2-isogeny kernel)
  ec_point_t K3; // K3 = E0[3^e2] \cap ker(\hat{alpha})
  {
    clock_t kernel_start = timings ? clock() : 0;
    ibz_t a, b, tmp, tmp2;
    ibz_init(&a);
    ibz_init(&b);
    ibz_init(&tmp);
    ibz_init(&tmp2);

    ibz_mat_2x2_t M;
    ibz_mat_2x2_init(&M);

    for (unsigned i = 0; i < 2; ++i) {
      ibz_add(&M[i][i], &M[i][i], &coeffs[0]);
      for (unsigned j = 0; j < 2; ++j) {
        ibz_mul(&tmp, &ACTION_GEN2[i][j], &coeffs[1]);
        ibz_add(&M[i][j], &M[i][j], &tmp);
        ibz_mul(&tmp, &ACTION_GEN3[i][j], &coeffs[2]);
        ibz_add(&M[i][j], &M[i][j], &tmp);
        ibz_mul(&tmp, &ACTION_GEN4[i][j], &coeffs[3]);
        ibz_add(&M[i][j], &M[i][j], &tmp);
      }
    }
    for (int i = 0; i < 2; i++)
      for (int j = 0; j < 2; j++)
        ibz_mod(&M[i][j], &M[i][j], pow3);

    // int col = 0;
    ibz_mod(&tmp, &M[0][0], &ibz_const_three);
    ibz_mod(&tmp2, &M[0][1], &ibz_const_three);
    if (!ibz_is_zero(&tmp) || !ibz_is_zero(&tmp2)) {
      ibz_copy(&a, &M[0][1]);
      ibz_copy(&b, &M[0][0]);
      ibz_neg(&b, &b);
      ibz_mod(&b, &b, pow3);
    } else {
      ibz_copy(&a, &M[1][1]);
      ibz_copy(&b, &M[1][0]);
      ibz_neg(&b, &b);
      ibz_mod(&b, &b, pow3);
    }

    // ibz_mod(&tmp, &a, &ibz_const_three);
    // if (!ibz_is_zero(&tmp)) {
    //   ibz_t inv;
    //   ibz_init(&inv);
    //   ibz_invmod(&inv, &a, pow3);
    //   ibz_mul(&b, &b, &inv);
    //   ibz_mod(&b, &b, pow3);
    //   ibz_set(&a, 1);
    //   ibz_finalize(&inv);
    // } else {
    //   ibz_t inv;
    //   ibz_init(&inv);
    //   ibz_invmod(&inv, &b, pow3);
    //   ibz_mul(&a, &a, &inv);
    //   ibz_mod(&a, &a, pow3);
    //   ibz_set(&b, 1);
    //   ibz_finalize(&inv);
    // }

    if (timings)
      timings->ms_sign_quat +=
          (float)(clock() - kernel_start) * 1000.f / CLOCKS_PER_SEC;
    SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
        ec_biscalar_mul_ibz(&K3, &CURVE_E0, &a, &b, &BASIS_THREE));
    ibz_finalize(&a);
    ibz_finalize(&b);
    ibz_finalize(&tmp);
    ibz_finalize(&tmp2);
    ibz_mat_2x2_finalize(&M);
  }

  // Logic for K2 (2^delta-isogeny kernel)
  if (delta) {
    ibz_t norm2;
    ibz_init(&norm2);
    ibz_set(&norm2, 1);
    ibz_mul_2exp(&norm2, &norm2, (uint64_t)delta);

    ec_point_t K2; // K2 = E0[2^delta] \cap ker(\hat{alpha})
    {
      clock_t kernel_start = timings ? clock() : 0;
      ibz_t a, b, tmp, tmp2;
      ibz_init(&a);
      ibz_init(&b);
      ibz_init(&tmp);
      ibz_init(&tmp2);

      ibz_mat_2x2_t M;
      ibz_mat_2x2_init(&M);

      for (unsigned i = 0; i < 2; ++i) {
        ibz_add(&M[i][i], &M[i][i], &coeffs[0]);
        for (unsigned j = 0; j < 2; ++j) {
          ibz_mul(&tmp, &ACTION_GEN2[i][j], &coeffs[1]);
          ibz_add(&M[i][j], &M[i][j], &tmp);
          ibz_mul(&tmp, &ACTION_GEN3[i][j], &coeffs[2]);
          ibz_add(&M[i][j], &M[i][j], &tmp);
          ibz_mul(&tmp, &ACTION_GEN4[i][j], &coeffs[3]);
          ibz_add(&M[i][j], &M[i][j], &tmp);
        }
      }
      for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
          ibz_mod(&M[i][j], &M[i][j], &norm2);

      ibz_mod(&tmp, &M[0][0], &ibz_const_two);
      ibz_mod(&tmp2, &M[0][1], &ibz_const_two);
      if (!ibz_is_zero(&tmp) || !ibz_is_zero(&tmp2)) {
        ibz_copy(&a, &M[0][1]);
        ibz_copy(&b, &M[0][0]);
        ibz_neg(&b, &b);
        ibz_mod(&b, &b, &norm2);
      } else {
        ibz_copy(&a, &M[1][1]);
        ibz_copy(&b, &M[1][0]);
        ibz_neg(&b, &b);
        ibz_mod(&b, &b, &norm2);
      }

      // ibz_mod(&tmp, &a, &ibz_const_two);
      // if (!ibz_is_zero(&tmp)) {
      //   ibz_t inv;
      //   ibz_init(&inv);
      //   ibz_invmod(&inv, &a, &norm2);
      //   ibz_mul(&b, &b, &inv);
      //   ibz_mod(&b, &b, &norm2);
      //   ibz_set(&a, 1);
      //   ibz_finalize(&inv);
      // } else {
      //   ibz_t inv;
      //   ibz_init(&inv);
      //   ibz_invmod(&inv, &b, &norm2);
      //   ibz_mul(&a, &a, &inv);
      //   ibz_mod(&a, &a, &norm2);
      //   ibz_set(&b, 1);
      //   ibz_finalize(&inv);
      // }

      if (timings)
        timings->ms_sign_quat +=
            (float)(clock() - kernel_start) * 1000.f / CLOCKS_PER_SEC;
      SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
          ec_biscalar_mul_ibz(&K2, &CURVE_E0, &a, &b, &BASIS_EVEN));

      ibz_finalize(&a);
      ibz_finalize(&b);
      ibz_finalize(&tmp);
      ibz_finalize(&tmp2);
      ibz_mat_2x2_finalize(&M);
      ibz_finalize(&norm2);
    }
    SIGN_TIME_CATEGORY(ms_sign_ec_non_isog, {
      for (int i = 0; i < (int)EXPONENT_TWO - delta; i++) {
        ec_dbl(&K2, &CURVE_E0, &K2);
      }
    });

    // Perform isogenies
    ec_isog_even_t phi2;
#ifdef DEBUG
    assert(point_order_2f(&K2, &CURVE_E0, delta));
#endif

    isog_init_two_f(&phi2, &CURVE_E0, &K2, delta);

    ec_point_t pts[4];
#ifdef DEBUG
    assert(point_order_3f(&K3, &CURVE_E0, EXPONENT_THREE));
#endif
    copy_point(pts + 0, &K3);
    copy_point(pts + 1, list_points + 0);
    copy_point(pts + 2, list_points + 1);
    copy_point(pts + 3, list_points + 2);

    clock_t _t_eval_even = clock();
    ec_eval_even(&E1, &phi2, pts, 4);

    if (timings) {
      float _dt = (float)(clock() - _t_eval_even) * 1000.f /
                  (float)CLOCKS_PER_SEC;
      timings->ms_sign_isog += _dt;
    }

    copy_point(&K3, pts + 0);
    copy_point(list_points + 0, pts + 1);
    copy_point(list_points + 1, pts + 2);
    copy_point(list_points + 2, pts + 3);

#ifdef DEBUG
    assert(point_order_3f(&K3, &E1, EXPONENT_THREE));
#endif
  } else {
    copy_curve(&E1, &CURVE_E0);
  }

  isog_init_three(&phi3, &E1, &K3, EXPONENT_THREE);

  {
    clock_t _t_isog = clock();
    ec_eval_three(&E0__, &phi3, list_points, 3);
    if (timings) {
      float _dt =
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_sign_isog += _dt;
    }
  }

  if (timings) {
  }

  copy_point(&RS.P, &list_points[0]);
  copy_point(&RS.Q, &list_points[1]);
  copy_point(&RS.PmQ, &list_points[2]);

#ifdef DEBUG
  assert(point_order_2f(&RS.P, &E0__, e));
  assert(point_order_2f(&RS.Q, &E0__, e));
  assert(point_order_2f(&RS.PmQ, &E0__, e));
#endif

  ec_basis_t B_can_three;
  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                  ec_curve_to_basis_3(&B_can_three, &E0__));

  theta_couple_curve_t E00__;
  E00__.E1 = CURVE_E0;
  E00__.E2 = E0__;
  // if (!fp2_is_one(&(E00__.E1.C))) {
  //   printf("normalize E00__.E1\n");
  //   ec_normalize_curve(&(E00__.E1));
  // }
  // if (!fp2_is_one(&(E00__.E2.C))) {
  //   // printf("normalize E00__.E2\n");
  //   ec_normalize_curve(&(E00__.E2));
  // }

  theta_kernel_couple_points_t dim_two_ker;
  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                  copy_bases_to_kernel(&dim_two_ker, &basis_even, &RS));

  clock_t t_cri_theta_setup = clock();
  bool extra_torsion = false;
  theta_couple_curve_t codomain;
  ec_curve_init(&codomain.E1);
  ec_curve_init(&codomain.E2);
  theta_couple_point_t coupleP1, coupleP2, coupleQ2, couplePmQ2;

  copy_point(&coupleP1.P1, kernel_phicom1);
  copy_point(&coupleP2.P2, &B_can_three.P);
  copy_point(&coupleQ2.P2, &B_can_three.Q);
  copy_point(&couplePmQ2.P2, &B_can_three.PmQ);
  ec_point_init(&coupleP1.P2);
  ec_point_init(&coupleP2.P1);
  ec_point_init(&coupleQ2.P1);
  ec_point_init(&couplePmQ2.P1);

  theta_couple_point_t pushed_points[4];
  pushed_points[0] = coupleP1;
  pushed_points[1] = coupleP2;
  pushed_points[2] = coupleQ2;
  pushed_points[3] = couplePmQ2;

  unsigned length = (unsigned)ibz_get(e_ibz);
  if (timings) {
    timings->ms_sign_ec_non_isog +=
        (float)(clock() - t_cri_theta_setup) * 1000.f /
        (float)CLOCKS_PER_SEC;
  }

  int ret;
  {
    clock_t _t_isog = clock();
    ret = theta_chain_compute_and_eval(
        length, &E00__, &dim_two_ker, extra_torsion, &codomain, pushed_points,
        sizeof(pushed_points) / sizeof(*pushed_points));
    if (timings) {
      float _dt =
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_sign_isog += _dt;
    }
  }


  if (!ret) {
    printf("theta_chain_compute_and_eval failed\n");
    return 0;
  }

  ec_point_t K0_, K0__;
  ec_basis_t image_B_can_three;

  copy_point(&K0_, &pushed_points[0].P1);
  copy_point(&image_B_can_three.P, &pushed_points[1].P1);
  copy_point(&image_B_can_three.Q, &pushed_points[2].P1);
  copy_point(&image_B_can_three.PmQ, &pushed_points[3].P1);

  digit_t u[NWORDS_ORDER] = {0}, v[NWORDS_ORDER] = {0};
  ibz_t u_ibz, v_ibz;
  ibz_init(&u_ibz);
  ibz_init(&v_ibz);

  // ec_dlog_3(u, v, &image_B_can_three, &K0_, &codomain.E1);
  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                  ec_dlog_3_tate_R(u, v, &image_B_can_three, &K0_,
                                   &codomain.E1, EXPONENT_THREE));
  ibz_copy_digit_array(&u_ibz, u);
  ibz_copy_digit_array(&v_ibz, v);

  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                  ec_biscalar_mul_ibz(&K0__, &E0__, &u_ibz, &v_ibz,
                                      &B_can_three));
#ifdef DEBUG
  assert(point_order_3f(&K0__, &E0__, EXPONENT_THREE));
#endif

  // digit_t digit_term2[NWORDS_ORDER] = {0};
  // ibz_to_digits(digit_term2, &term2);
  // ec_mul(&K0__, &E0__, digit_term2, &K0__);

  ec_isog_odd_t phi0__;
  isog_init_three(&phi0__, &E0__, &K0__, EXPONENT_THREE);

  ec_point_t pts[3];
  copy_point(pts + 0, &RS.P);
  copy_point(pts + 1, &RS.Q);
  copy_point(pts + 2, &RS.PmQ);

  {
    clock_t _t_isog = clock();
    ec_eval_three(Em__, &phi0__, pts, 3);
    if (timings) {
      float _dt =
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_sign_isog += _dt;
    }
  }
  if (timings) {
  }

  copy_point(&RSm__->P, &pts[0]);
  copy_point(&RSm__->Q, &pts[1]);
  copy_point(&RSm__->PmQ, &pts[2]);

  // Cleanup
  ibz_finalize(&u_ibz);
  ibz_finalize(&v_ibz);
  ibz_finalize(&n_alpha);
  quat_alg_elem_finalize(&alpha);
  ibz_finalize(&term2);
  ibz_finalize(&three_pow);
  quat_alg_elem_finalize(&alpha_conj);
  ibz_finalize(&inv_scalar);
  ibz_mat_2x2_finalize(&M_alpha);
  ibz_mat_2x2_finalize(&M_alpha_conj);
  ibz_vec_4_finalize(&coeffs);
  return 1;
}

int pushrandisog(ec_curve_t *E_aux, ec_basis_t *image, ibz_t *d_coprime6,
                 ec_point_t *kernel_phicom1, ec_point_t *kernel_phicom2,
                 ec_curve_t *E_com_mid, ec_basis_t *Bcom0_mid, int n,
                 ibz_t *pow2, ibz_t *pow3, sign_timings_t *timings) {

  clock_t t_pri_setup = clock();
  ibz_t gcd6, val6;
  ibz_init(&gcd6);
  ibz_init(&val6);
  ibz_set(&val6, 6);
  ibz_gcd(&gcd6, d_coprime6, &val6);
  assert(ibz_is_one(&gcd6));

  if (timings)
    timings->ms_sign_quat +=
        (float)(clock() - t_pri_setup) * 1000.f / CLOCKS_PER_SEC;
  t_pri_setup = timings ? clock() : 0;

  ec_isog_odd_t phi_0;
  ec_curve_t E_m;
  ec_point_t list_points[3];
  ec_basis_t PQ_m_even, PQ_m;

#ifdef DEBUG
  assert(point_order_3f(kernel_phicom1, &CURVE_E0, EXPONENT_THREE));
#endif

  copy_curve(&E_m, E_com_mid);
  copy_point(list_points + 0, &(Bcom0_mid->P));
  copy_point(list_points + 1, &(Bcom0_mid->Q));
  copy_point(list_points + 2, &(Bcom0_mid->PmQ));

#ifdef DEBUG
  assert(point_order_2f(list_points + 0, &E_m, EXPONENT_TWO));
  assert(point_order_2f(list_points + 1, &E_m, EXPONENT_TWO));
  assert(point_order_2f(list_points + 2, &E_m, EXPONENT_TWO));
#endif

  copy_point(&PQ_m_even.P, list_points + 0);
  copy_point(&PQ_m_even.Q, list_points + 1);
  copy_point(&PQ_m_even.PmQ, list_points + 2);

  copy_point(&PQ_m.P, list_points + 0);
  copy_point(&PQ_m.Q, list_points + 1);
  copy_point(&PQ_m.PmQ, list_points + 2);
  if (timings) {
    timings->ms_sign_ec_non_isog +=
        (float)(clock() - t_pri_setup) * 1000.f / (float)CLOCKS_PER_SEC;
  }

#ifdef DEBUG
  assert(point_order_3f(kernel_phicom2, &E_m, EXPONENT_THREE));
#endif

  clock_t t_pri_choose_e = clock();
  int e;
  int e1_ = (int)EXPONENT_TWO - n; // e1 - n
  // int e2 = e1 - 1;
  ibz_t N_e1, N_e2;
  ibz_init(&N_e1);
  ibz_init(&N_e2);

  ibz_set(&N_e2, 1);
  ibz_mul_2exp(&N_e2, &N_e2, e1_ - 1);
  // ibz_mul_2exp(&N_e1, &N_e2, 1);
  ibz_mul(&N_e1, &ibz_const_two, &N_e2);
  ibz_sub(&N_e1, &N_e1, d_coprime6);
  ibz_sub(&N_e2, &N_e2, d_coprime6);

  // we can choose e1' = e1 - n1 or e1' = e1 - n1 - 1
  ibz_t gcd1, gcd2;
  ibz_init(&gcd1);
  ibz_init(&gcd2);
  ibz_gcd(&gcd1, &N_e1, &ibz_const_three);
  ibz_gcd(&gcd2, &N_e2, &ibz_const_three);
  assert(ibz_is_one(&gcd1) || ibz_is_one(&gcd2));
  if (ibz_is_one(&gcd1)) {
    e = e1_;
  } else {
    e = e1_ - 1;
  }
  ibz_finalize(&gcd1);
  ibz_finalize(&gcd2);

  int delta = (int)EXPONENT_TWO - e;

  ibz_t ibz_e, ibz_delta;
  ibz_init(&ibz_e);
  ibz_init(&ibz_delta);
  ibz_set(&ibz_e, e);
  ibz_set(&ibz_delta, delta);
  if (timings) {
    timings->ms_sign_quat +=
        (float)(clock() - t_pri_choose_e) * 1000.f /
        (float)CLOCKS_PER_SEC;
  }

  ec_curve_t Em__;
  ec_basis_t RSm__;

  int ret;
  // The callee records its own category timings.
  ret = composed_rand_isog(&Em__, &RSm__, d_coprime6,
                                           kernel_phicom1, &ibz_e, &ibz_delta,
                                           pow2, pow3, timings);
  if (!ret) {
    return 0;
  }
  theta_couple_curve_t Emm__;
  Emm__.E1 = E_m;
  Emm__.E2 = Em__;
  // if (!fp2_is_one(&(Emm__.E1.C))) {
  //   ec_normalize_curve(&(Emm__.E1));
  // }
  // if (!fp2_is_one(&(Emm__.E2.C))) {
  //   ec_normalize_curve(&(Emm__.E2));
  // }

  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog, {
    for (int i = 0; i < delta; i++) {
      ec_dbl(&PQ_m_even.P, &E_m, &PQ_m_even.P);
      ec_dbl(&PQ_m_even.Q, &E_m, &PQ_m_even.Q);
      ec_dbl(&PQ_m_even.PmQ, &E_m, &PQ_m_even.PmQ);
    }
  });

  theta_kernel_couple_points_t dim_two_ker;
  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                  copy_bases_to_kernel(&dim_two_ker, &PQ_m_even, &RSm__));

  clock_t t_pri_theta_setup = clock();
  bool extra_torsion = false;
  theta_couple_curve_t codomain;
  ec_curve_init(&codomain.E1);
  ec_curve_init(&codomain.E2);
  theta_couple_point_t coupleP1, coupleQ1, couplePmQ1, coupleK1;
  copy_point(&coupleP1.P1, &PQ_m.P);
  copy_point(&coupleQ1.P1, &PQ_m.Q);
  copy_point(&couplePmQ1.P1, &PQ_m.PmQ);
  copy_point(&coupleK1.P1, kernel_phicom2);
  ec_point_init(&coupleP1.P2);
  ec_point_init(&coupleQ1.P2);
  ec_point_init(&couplePmQ1.P2);
  ec_point_init(&coupleK1.P2);

  theta_couple_point_t pushed_points[4];
  pushed_points[0] = coupleP1;
  pushed_points[1] = coupleQ1;
  pushed_points[2] = couplePmQ1;
  pushed_points[3] = coupleK1;

  unsigned length = (unsigned)ibz_get(&ibz_e);
  if (timings) {
    timings->ms_sign_ec_non_isog +=
        (float)(clock() - t_pri_theta_setup) * 1000.f /
        (float)CLOCKS_PER_SEC;
  }

  {
    clock_t _t_isog = clock();
    ret = theta_chain_compute_and_eval(
        length, &Emm__, &dim_two_ker, extra_torsion, &codomain, pushed_points,
        sizeof(pushed_points) / sizeof(*pushed_points));
    if (timings) {
      float _dt =
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_sign_isog += _dt;
    }
  }

  if (!ret) {
    printf("theta_chain_compute_and_eval failed\n");
    return 0;
  }

  fp2_t w0a, w1a, w2a;

#ifdef DEBUG
  assert(point_order_2f(&PQ_m.P, &domain_E1_tmp, EXPONENT_TWO));
  assert(point_order_2f(&PQ_m.Q, &domain_E1_tmp, EXPONENT_TWO));
  assert(point_order_2f(&PQ_m.PmQ, &domain_E1_tmp, EXPONENT_TWO));
  assert(point_order_2f(&pushed_points[0].P1, &codomain_E1_tmp, EXPONENT_TWO));
  assert(point_order_2f(&pushed_points[1].P1, &codomain_E1_tmp, EXPONENT_TWO));
  assert(point_order_2f(&pushed_points[2].P1, &codomain_E1_tmp, EXPONENT_TWO));
  assert(point_order_2f(&pushed_points[0].P2, &codomain_E2_tmp, EXPONENT_TWO));
  assert(point_order_2f(&pushed_points[1].P2, &codomain_E2_tmp, EXPONENT_TWO));
  assert(point_order_2f(&pushed_points[2].P2, &codomain_E2_tmp, EXPONENT_TWO));
#endif

  digit_t digit_d[NWORDS_ORDER] = {0};
  fp2_t test_powa;
  int res1, res2;
  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog, {
    weil(&w0a, EXPONENT_TWO, &PQ_m.P, &PQ_m.Q, &PQ_m.PmQ, &Emm__.E1);
    weil(&w1a, EXPONENT_TWO, &pushed_points[0].P1, &pushed_points[1].P1,
         &pushed_points[2].P1, &codomain.E1);
    weil(&w2a, EXPONENT_TWO, &pushed_points[0].P2, &pushed_points[1].P2,
         &pushed_points[2].P2, &codomain.E2);

    ibz_to_digits(digit_d, d_coprime6);
    fp2_pow_vartime(&test_powa, &w0a, digit_d, NWORDS_ORDER);

    res1 = fp2_is_equal(&test_powa, &w1a);
    res2 = fp2_is_equal(&test_powa, &w2a);
  });

  assert(res1 || res2);

  ec_point_t push_pts[3];
  ec_point_t kernel_psim_;
  ec_curve_t Em_;

  SIGN_TIME_CATEGORY(ms_sign_ec_non_isog, {
    if (res1) {
      copy_curve(&Em_, &codomain.E1);
      copy_point(&push_pts[0], &pushed_points[0].P1);
      copy_point(&push_pts[1], &pushed_points[1].P1);
      copy_point(&push_pts[2], &pushed_points[2].P1);
      copy_point(&kernel_psim_, &pushed_points[3].P1);
    } else {
      copy_curve(&Em_, &codomain.E2);
      copy_point(&push_pts[0], &pushed_points[0].P2);
      copy_point(&push_pts[1], &pushed_points[1].P2);
      copy_point(&push_pts[2], &pushed_points[2].P2);
      copy_point(&kernel_psim_, &pushed_points[3].P2);
    }
  });

  ec_isog_odd_t psim_;
  isog_init_three(&psim_, &Em_, &kernel_psim_, EXPONENT_THREE);

  {
    clock_t _t_isog = clock();
    ec_eval_three(E_aux, &psim_, push_pts, 3);
    if (timings) {
      float _dt =
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_sign_isog += _dt;
    }
  }
  if (timings) {
  }

  copy_point(&image->P, &push_pts[0]);
  copy_point(&image->Q, &push_pts[1]);
  copy_point(&image->PmQ, &push_pts[2]);

  ibz_finalize(&ibz_e);
  ibz_finalize(&ibz_delta);
  ibz_finalize(&N_e1);
  ibz_finalize(&N_e2);
  ibz_finalize(&gcd6);
  ibz_finalize(&val6);
  return 1;
}

void quat_lideal_conjugate_lattice(quat_lattice_t *lat,
                                   const quat_left_ideal_t *lideal) {
  ibz_mat_4x4_copy(&(lat->basis), &(lideal->lattice.basis));
  ibz_copy(&(lat->denom), &(lideal->lattice.denom));

  for (int row = 1; row < 4; ++row) {
    for (int col = 0; col < 4; ++col) {
      ibz_neg(&(lat->basis[row][col]), &(lat->basis[row][col]));
    }
  }

  return;
}

int is_good_norm_2dpush(ibz_t *N) {
  ibz_t pow2;
  ibz_init(&pow2);
  int res = 0;

  ibz_set(&pow2, 1);
  ibz_mul_2exp(&pow2, &pow2, EXPONENT_TWO);

  // Check 0 < N < 2^TORSION_PLUS_EVEN_POWER
  bool check1 = ibz_cmp(N, &ibz_const_zero) > 0;
  bool check2 = ibz_cmp(N, &pow2) < 0;
  // printf("check1 = %d, check2 = %d\n", check1, check2);
  if (check1 && check2) {
    ibz_t gcd;
    ibz_init(&gcd);
    ibz_gcd(&gcd, &ibz_const_three, N);
    if (ibz_is_one(&gcd)) {
      res = 1;
    } else {
      res = 0;
    }
    ibz_finalize(&gcd);
  }

  ibz_finalize(&pow2);
  return res;
}

int is_good_2dpush(quat_alg_elem_t *x, ibz_t const *lattice_content) {
  ibq_t N_q;
  ibz_t N, tmp, pow2;
  ibq_init(&N_q);
  ibz_init(&N);
  ibz_init(&tmp);
  int res = 0;

  quat_alg_norm(&N_q, x, &QUATALG_PINFTY);
  ibq_to_ibz(&N, &N_q);

  assert(ibz_divides(&N, lattice_content));

  ibz_div(&N, &tmp, &N, lattice_content);

  res = is_good_norm_2dpush(&N);

#ifndef NDEBUG
  ibz_init(&pow2);
  ibz_set(&pow2, 1);
  ibz_mul_2exp(&pow2, &pow2, EXPONENT_TWO);
  int res2 = 0;
  if (ibz_cmp(&N, &ibz_const_zero) > 0 && ibz_cmp(&N, &pow2) < 0) {
    ibz_t gcd;
    ibz_init(&gcd);
    ibz_gcd(&gcd, &ibz_const_three, &N);
    if (ibz_is_one(&gcd)) {
      res2 = 1;
    } else {
      res2 = 0;
    }
    ibz_finalize(&gcd);
  }

  assert(res == res2);
  ibz_finalize(&pow2);
#endif

  ibz_finalize(&N);
  ibq_finalize(&N_q);
  ibz_finalize(&tmp);
  return res;
}

void norm_from_2_times_gram(ibz_t *norm, ibz_mat_4x4_t *gram,
                            ibz_vec_4_t *vec) {
  quat_qf_eval(norm, gram, vec);
  assert(ibz_is_even(norm));
  ibz_div_2exp(norm, norm, 1);
}

// /*
//  * The Julia implementation of element_for_response enumerates the vectors in
//  * the LLL-reduced lattice with a Fincke-Pohst style search.  The following
//  * small helpers are the C equivalent of the Rational{BigInt} operations used
//  * by that implementation.
//  */
// static void response_floor_rational(ibz_t *result, const ibq_t *value) {
//   ibz_t numerator, denominator, remainder;
//   ibz_init(&numerator);
//   ibz_init(&denominator);
//   ibz_init(&remainder);
//   ibq_num(&numerator, value);
//   ibq_denom(&denominator, value);
//   ibz_div_floor(result, &remainder, &numerator, &denominator);
//   ibz_finalize(&numerator);
//   ibz_finalize(&denominator);
//   ibz_finalize(&remainder);
// }

// static void response_ceil_rational(ibz_t *result, const ibq_t *value) {
//   ibq_t neg_value;
//   ibz_t floor_neg;
//   ibq_init(&neg_value);
//   ibz_init(&floor_neg);
//   ibq_neg(&neg_value, value);
//   response_floor_rational(&floor_neg, &neg_value);
//   ibz_neg(result, &floor_neg);
//   ibq_finalize(&neg_value);
//   ibz_finalize(&floor_neg);
// }

// static void response_set_q_from_ibz(ibq_t *result, const ibz_t *value) {
//   ibz_t one;
//   ibz_init(&one);
//   ibz_set(&one, 1);
//   ibq_set(result, value, &one);
//   ibz_finalize(&one);
// }

// static void response_set_q_zero(ibq_t *result) {
//   response_set_q_from_ibz(result, &ibz_const_zero);
// }

// /* Set the bounds for one coordinate in the Julia enumeration. */
// static void response_set_coordinate_bounds(ibz_t *lower, ibz_t *upper,
//                                            const ibq_t *s, const ibq_t *u,
//                                            const ibq_t *qii) {
//   ibz_t denominator_u, denominator_u_squared, one, tmp_int, z_int;
//   ibq_t tmp, denominator_q, z, z_plus_u, z_minus_u;

//   ibz_init(&denominator_u);
//   ibz_init(&denominator_u_squared);
//   ibz_init(&one);
//   ibz_init(&tmp_int);
//   ibz_init(&z_int);
//   ibq_init(&tmp);
//   ibq_init(&denominator_q);
//   ibq_init(&z);
//   ibq_init(&z_plus_u);
//   ibq_init(&z_minus_u);

//   ibq_denom(&denominator_u, u);
//   ibz_mul(&denominator_u_squared, &denominator_u, &denominator_u);
//   ibz_set(&one, 1);
//   ibq_set(&denominator_q, &denominator_u_squared, &one);

//   /* Julia uses div(S*denominator(U)^2, q[i,i]) here. */
//   ibq_mul(&tmp, s, &denominator_q);
//   ibq_div(&tmp, &tmp, qii);
//   response_floor_rational(&tmp_int, &tmp);
//   if (ibz_cmp(&tmp_int, &ibz_const_zero) < 0) {
//     ibz_set(&tmp_int, 0);
//   }
//   ibz_sqrt_floor(&z_int, &tmp_int);
//   ibq_set(&z, &z_int, &denominator_u);

//   ibq_sub(&z_minus_u, &z, u);
//   response_floor_rational(upper, &z_minus_u);

//   ibq_add(&z_plus_u, &z, u);
//   response_ceil_rational(lower, &z_plus_u);
//   ibz_sub(lower, lower, &one);

//   ibz_finalize(&denominator_u);
//   ibz_finalize(&denominator_u_squared);
//   ibz_finalize(&one);
//   ibz_finalize(&tmp_int);
//   ibz_finalize(&z_int);
//   ibq_finalize(&tmp);
//   ibq_finalize(&denominator_q);
//   ibq_finalize(&z);
//   ibq_finalize(&z_plus_u);
//   ibq_finalize(&z_minus_u);
// }

// /* Construct the triangular quadratic form used by Julia's
//  * make_quadratic_form_coeffs.  gram evaluates twice the normalized norm, so
//  * C is gram/2 here. */
// static void response_make_quadratic_form(ibq_t q[4][4],
//                                          const ibz_mat_4x4_t *gram) {
//   ibq_t c[4][4];
//   ibq_t term, sum;
//   ibz_t two;

//   ibz_init(&two);
//   ibz_set(&two, 2);
//   ibq_init(&term);
//   ibq_init(&sum);
//   for (int i = 0; i < 4; i++) {
//     for (int j = 0; j < 4; j++) {
//       ibq_init(&c[i][j]);
//       ibq_init(&q[i][j]);
//       ibq_set(&c[i][j], &(*gram)[i][j], &two);
//     }
//   }

//   for (int i = 0; i < 4; i++) {
//     response_set_q_zero(&sum);
//     for (int k = 0; k < i; k++) {
//       ibq_mul(&term, &q[k][k], &q[k][i]);
//       ibq_mul(&term, &term, &q[k][i]);
//       ibq_add(&sum, &sum, &term);
//     }
//     ibq_sub(&q[i][i], &c[i][i], &sum);

//     for (int j = i + 1; j < 4; j++) {
//       response_set_q_zero(&sum);
//       for (int k = 0; k < i; k++) {
//         ibq_mul(&term, &q[k][k], &q[k][i]);
//         ibq_mul(&term, &term, &q[k][j]);
//         ibq_add(&sum, &sum, &term);
//       }
//       ibq_sub(&term, &c[i][j], &sum);
//       ibq_div(&q[i][j], &term, &q[i][i]);
//     }
//   }

//   for (int i = 0; i < 4; i++) {
//     for (int j = 0; j < 4; j++) {
//       ibq_finalize(&c[i][j]);
//     }
//   }
//   ibq_finalize(&term);
//   ibq_finalize(&sum);
//   ibz_finalize(&two);
// }

// static void response_make_primitive(ibz_vec_4_t *primitive,
//                                     const ibz_vec_4_t *vector) {
//   ibz_t gcd, absolute, remainder;
//   ibz_init(&gcd);
//   ibz_init(&absolute);
//   ibz_init(&remainder);
//   ibz_set(&gcd, 0);

//   for (int i = 0; i < 4; i++) {
//     ibz_abs(&absolute, &(*vector)[i]);
//     if (ibz_is_zero(&gcd)) {
//       ibz_copy(&gcd, &absolute);
//     } else {
//       ibz_gcd(&gcd, &gcd, &absolute);
//     }
//   }

//   for (int i = 0; i < 4; i++) {
//     ibz_div(&(*primitive)[i], &remainder, &(*vector)[i], &gcd);
//   }

//   ibz_finalize(&gcd);
//   ibz_finalize(&absolute);
//   ibz_finalize(&remainder);
// }

// int sample_response(quat_alg_elem_t *x, const quat_lattice_t *lattice,
//                     ibz_t const *lattice_content, int verbose,
//                     sign_timings_t *timings) {
//   ibz_mat_4x4_t lll;
//   ibz_t denom_gram, norm, norm_bound;

//   ibz_mat_4x4_init(&lll);
//   ibz_init(&denom_gram);
//   ibz_init(&norm);
//   ibz_init(&norm_bound);

//   ibz_pow(&norm_bound, &ibz_const_two, EXPONENT_TWO);

//   // Upper bound on log(determinant)
//   int logdet = 0;
//   for (int i = 0; i < 4; i++) {
//     int max = 0;
//     for (int j = 0; j < 4; j++) {
//       int s = ibz_bitsize(&lattice->basis[i][j]);
//       max = s > max ? s : max;
//     }
//     logdet += max;
//   }

//   int err = quat_lattice_lll(&lll, lattice, &(QUATALG_PINFTY.p), 2 * logdet);
//   assert(!err);
//   // The shortest vector found by lll is a candidate

//   int found = 0;

//   ibz_mat_4x4_t prod, gram;
//   ibz_mat_4x4_init(&prod);
//   ibz_mat_4x4_init(&gram);

//   ibz_mat_4x4_transpose(&prod, &lll);
//   ibz_mat_4x4_mul(&prod, &prod, &(QUATALG_PINFTY.gram));
//   ibz_mat_4x4_mul(&gram, &prod, &lll);

//   ibz_copy(&denom_gram, &(lattice->denom));
//   ibz_mul(&denom_gram, &denom_gram, &(lattice->denom));
//   ibz_mul(&denom_gram, &denom_gram, lattice_content);

//   assert(ibz_is_even(&denom_gram));
//   ibz_div_2exp(&denom_gram, &denom_gram, 1);

//   int divides = ibz_mat_4x4_scalar_div(&gram, &denom_gram, &gram);
//   assert(divides);

//   ibz_copy(&(x->denom), &(lattice->denom));

//   ibz_vec_4_t vec, primitive;
//   ibz_vec_4_init(&vec);
//   ibz_vec_4_init(&primitive);

//   ibq_t q[4][4], s[4], u[4];
//   ibz_t lower[4], upper[4], one;
//   for (int i = 0; i < 4; i++) {
//     ibq_init(&s[i]);
//     ibq_init(&u[i]);
//     ibz_init(&lower[i]);
//     ibz_init(&upper[i]);
//     response_set_q_zero(&s[i]);
//     response_set_q_zero(&u[i]);
//     ibz_set(&vec[i], 0);
//   }
//   ibz_init(&one);
//   ibz_set(&one, 1);
//   response_make_quadratic_form(q, &gram);
//   response_set_q_from_ibz(&s[3], &norm_bound);
//   response_set_coordinate_bounds(&lower[3], &upper[3], &s[3], &u[3],
//                                  &q[3][3]);

//   /* This is the same exhaustive search as element_for_response in Julia. */
//   int level = 3;
//   int exhausted = 0;
//   while (!found && !exhausted) {
//     ibz_add(&vec[level], &vec[level], &one);
//     while (ibz_cmp(&vec[level], &upper[level]) > 0) {
//       if (level == 3) {
//         exhausted = 1;
//         break;
//       }
//       level++;
//       ibz_add(&vec[level], &vec[level], &one);
//     }
//     if (exhausted) {
//       break;
//     }

//     if (level > 0) {
//       ibq_t shifted, shifted_squared, contribution;
//       ibq_init(&shifted);
//       ibq_init(&shifted_squared);
//       ibq_init(&contribution);
//       response_set_q_from_ibz(&shifted, &vec[level]);
//       ibq_add(&shifted, &shifted, &u[level]);
//       ibq_mul(&shifted_squared, &shifted, &shifted);
//       ibq_mul(&contribution, &q[level][level], &shifted_squared);
//       ibq_sub(&s[level - 1], &s[level], &contribution);
//       level--;
//       response_set_q_zero(&u[level]);
//       for (int j = level + 1; j < 4; j++) {
//         response_set_q_from_ibz(&shifted, &vec[j]);
//         ibq_mul(&contribution, &q[level][j], &shifted);
//         ibq_add(&u[level], &u[level], &contribution);
//       }
//       response_set_coordinate_bounds(&lower[level], &upper[level],
//                                      &s[level], &u[level], &q[level][level]);
//       ibz_copy(&vec[level], &lower[level]);
//       ibz_sub(&vec[level], &vec[level], &one);
//       ibq_finalize(&shifted);
//       ibq_finalize(&shifted_squared);
//       ibq_finalize(&contribution);
//     } else {
//       int nonzero = !ibz_is_zero(&vec[0]) || !ibz_is_zero(&vec[1]) ||
//                     !ibz_is_zero(&vec[2]) || !ibz_is_zero(&vec[3]);
//       if (nonzero) {
//         response_make_primitive(&primitive, &vec);
//         norm_from_2_times_gram(&norm, &gram, &primitive);
//         if (is_good_norm_2dpush(&norm)) {
//           ibz_mat_4x4_eval(&(x->coord), &lll, &primitive);
//           found = 1;
//         }
//       }
//     }
//   }

//   for (int i = 0; i < 4; i++) {
//     ibq_finalize(&s[i]);
//     ibq_finalize(&u[i]);
//     ibz_finalize(&lower[i]);
//     ibz_finalize(&upper[i]);
//   }
//   for (int i = 0; i < 4; i++) {
//     for (int j = 0; j < 4; j++) {
//       ibq_finalize(&q[i][j]);
//     }
//   }
//   ibz_finalize(&one);
// #ifndef NDEBUG
//   printf("good sample found by deterministic lattice enumeration\n");
// #endif
//   // assert(quat_lattice_contains(NULL, lattice, x, &QUATALG_PINFTY));

//   ibz_finalize(&denom_gram);
//   ibz_finalize(&norm);
//   ibz_mat_4x4_finalize(&prod);
//   ibz_mat_4x4_finalize(&gram);
//   ibz_mat_4x4_finalize(&lll);
//   ibz_vec_4_finalize(&vec);
//   ibz_vec_4_finalize(&primitive);
//   ibz_finalize(&norm_bound);
//   return found;
// }

int sample_response(quat_alg_elem_t *x, const quat_lattice_t *lattice,
                    ibz_t const *lattice_content, int verbose,
                    sign_timings_t *timings) {
  ibz_mat_4x4_t lll;
  ibz_t denom_gram, norm, norm_bound;

  ibz_mat_4x4_init(&lll);
  ibz_init(&denom_gram);
  ibz_init(&norm);
  ibz_init(&norm_bound);

  ibz_pow(&norm_bound, &ibz_const_two, EXPONENT_TWO);

  // Upper bound on log(determinant)
  int logdet = 0;
  for (int i = 0; i < 4; i++) {
    int max = 0;
    for (int j = 0; j < 4; j++) {
      int s = ibz_bitsize(&lattice->basis[i][j]);
      max = s > max ? s : max;
    }
    logdet += max;
  }

  int err = quat_lattice_lll(&lll, lattice, &(QUATALG_PINFTY.p), 2 * logdet);
  assert(!err);
  // The shortest vector found by lll is a candidate

  int found;

  ibz_mat_4x4_t prod, gram;
  ibz_mat_4x4_init(&prod);
  ibz_mat_4x4_init(&gram);

  ibz_mat_4x4_transpose(&prod, &lll);
  ibz_mat_4x4_mul(&prod, &prod, &(QUATALG_PINFTY.gram));
  ibz_mat_4x4_mul(&gram, &prod, &lll);

  ibz_copy(&denom_gram, &(lattice->denom));
  ibz_mul(&denom_gram, &denom_gram, &(lattice->denom));
  ibz_mul(&denom_gram, &denom_gram, lattice_content);

  assert(ibz_is_even(&denom_gram));
  ibz_div_2exp(&denom_gram, &denom_gram, 1);

  int divides = ibz_mat_4x4_scalar_div(&gram, &denom_gram, &gram);
  assert(divides);

  ibz_copy(&(x->denom), &(lattice->denom));

  ibz_vec_4_t vec;
  ibz_vec_4_init(&vec);

  found = 0;
  int cnt = 0;

  // TODO make these clean constants
  int m = 10;
  while (!found &&
         cnt < 2 * (2 * m + 1) * (2 * m + 1) * (2 * m + 1) * (2 * m + 1)) {
    cnt++;
    for (int i = 0; i < 4; i++) {
      ibz_rand_interval_minm_m(&vec[i], m);
    }
    norm_from_2_times_gram(&norm, &gram, &vec);
    // ibz_printf("norm = %Zd\n", &norm);
    // now we test if the norm is good
    // there are two constraints : the norm must be smaller than some bound
    // and the element must be primitive in O0 (this ensures that there is no
    // backtracking)
    found =
        is_good_norm_2dpush(&norm) && (ibz_cmp(&vec[0], &ibz_const_zero) != 0 ||
                                       ibz_cmp(&vec[1], &ibz_const_zero) != 0 ||
                                       ibz_cmp(&vec[2], &ibz_const_zero) != 0 ||
                                       ibz_cmp(&vec[3], &ibz_const_zero) != 0);
    if (found) {
      ibz_mat_4x4_eval(&(x->coord), &lll, &vec);
      // assert(quat_lattice_contains(NULL, lattice, x, &QUATALG_PINFTY));
      if (!quat_alg_is_primitive(x, &MAXORD_O0, &QUATALG_PINFTY)) {
        found = 0;
      }
    }
  }
#ifndef NDEBUG
  printf("good sample found after %d attempts\n", cnt);
#endif
  // assert(quat_lattice_contains(NULL, lattice, x, &QUATALG_PINFTY));

  ibz_finalize(&denom_gram);
  ibz_finalize(&norm);
  ibz_mat_4x4_finalize(&prod);
  ibz_mat_4x4_finalize(&gram);
  ibz_mat_4x4_finalize(&lll);
  ibz_vec_4_finalize(&vec);
  ibz_finalize(&norm_bound);
  return found;
}

// TODO(code): this is also used in verification, move to a common location when
// verification is implemented
void hash_to_challenge(ibz_vec_2_t *scalars, const ec_curve_t *curve,
                       const unsigned char *message, const public_key_t *pk,
                       size_t length) {
  ibz_t pow3;
  ibz_init(&pow3);
  ibz_pow(&pow3, &ibz_const_three, EXPONENT_THREE);

  unsigned char *buf = malloc(2 * FP2_ENCODED_BYTES + length);
  {
    fp2_t j1, j2;
    ec_j_inv(&j1, curve);
    ec_j_inv(&j2, &pk->curve);
    fp_encode_legacy_hash(buf, j1.re);
    fp_encode_legacy_hash(buf + FP_ENCODED_BYTES, j1.im);
    fp_encode_legacy_hash(buf + FP2_ENCODED_BYTES, j2.re);
    fp_encode_legacy_hash(buf + FP2_ENCODED_BYTES + FP_ENCODED_BYTES, j2.im);
    memcpy(buf + 2 * FP2_ENCODED_BYTES, message, length);
  }

  // TODO(security) omit some vectors, notably (a,1) with gcd(a,6)!=1 but
  // also things like (2,3)?
  {
    digit_t digits[NWORDS_ORDER] = {0};

    // FIXME should use SHAKE128 for smaller parameter sets?
    sha3_256((void *)digits, buf, 2 * FP2_ENCODED_BYTES + length);

    for (int i = 2; i < HASH_ITERATIONS; i++) {
      sha3_256((void *)digits, (void *)digits, sizeof(digits));
    }

    ibz_set(&(*scalars)[1], 1); // FIXME
    ibz_copy_digit_array(&(*scalars)[1], digits);
    ibz_mod(&(*scalars)[1], &(*scalars)[1], &pow3); // hash % 3^e2
  }

  ibz_set(&((*scalars)[0]), 1);

  ibz_finalize(&pow3);
  free(buf);
}

int protocols_sign(signature_t *sig, const public_key_t *pk,
                   const secret_key_t *sk, const unsigned char *m, size_t l,
                   int verbose, sign_timings_t *timings) {

  clock_t t = tic();

  // Initialize timing accumulators
  if (timings) {
    timings->ms_sign_isog = 0.f;
    timings->ms_sign_ec_non_isog = 0.f;
    timings->ms_sign_quat = 0.f;
    timings->ms_commit = 0.f;
    timings->ms_challenge = 0.f;
    timings->ms_response = 0.f;
  }

  ibz_t lattice_content;
  ec_curve_t E_com, E_com_mid;
  ec_basis_t Bcom0_mid, Bcom0, Bcom_can; // basis of 2^n-torsion
  ec_point_t kernel_isocom_three1, kernel_isocom_three2;
  ibz_vec_2_t vec, vec_can, vec_zero;
  quat_left_ideal_t lideal_tmp;
  quat_left_ideal_t lideal_commit_three, lideal_chall_three;
  quat_left_ideal_t lideal_chall3_secret2, lideal_chall3_secret3;
  quat_lattice_t lattice_hom_chall_to_com, lat_commit;
  quat_alg_elem_t resp_quat, resp_quat_2dpush, resp_quat_2dpush_conj;
  quat_alg_elem_t elem_tmp;
  ibz_mat_2x2_t mat_alpha0, mat_Bcom0_to_Bcom;
  ibz_mat_2x2_t mat_sigma_phichall_BA_to_Bcomcan,
      mat_sigma_phichall_BA0_to_Bcom0;
  ibz_mat_2x2_t mat_BAcan_to_BA0_two, mat_BAcan_to_BA0_three;
  ibz_t degree_com_isogeny, tmp;

  ibz_init(&degree_com_isogeny);
  ibz_init(&tmp);
  ibz_init(&lattice_content);

  ibz_mat_2x2_init(&mat_alpha0);
  ibz_mat_2x2_init(&mat_Bcom0_to_Bcom);
  ibz_mat_2x2_init(&mat_sigma_phichall_BA_to_Bcomcan);
  ibz_mat_2x2_init(&mat_sigma_phichall_BA0_to_Bcom0);
  ibz_mat_2x2_init(&mat_BAcan_to_BA0_two);
  ibz_mat_2x2_init(&mat_BAcan_to_BA0_three);

  quat_alg_elem_init(&resp_quat);
  quat_alg_elem_init(&resp_quat_2dpush);
  quat_alg_elem_init(&resp_quat_2dpush_conj);
  quat_alg_elem_init(&elem_tmp);
  quat_lattice_init(&lattice_hom_chall_to_com);
  quat_lattice_init(&lat_commit);
  quat_left_ideal_init(&lideal_tmp);
  quat_left_ideal_init(&lideal_commit_three);
  quat_left_ideal_init(&lideal_chall_three);
  quat_left_ideal_init(&lideal_chall3_secret2);
  quat_left_ideal_init(&lideal_chall3_secret3);

  ibz_vec_2_init(&vec);
  ibz_vec_2_init(&vec_can);

  ibz_t pow2, pow3;
  ibz_init(&pow2);
  ibz_init(&pow3);

  clock_t t_start, t_end, t_step;

  ibz_pow(&pow2, &ibz_const_two, EXPONENT_TWO);     // pow2 = 2^e1
  ibz_pow(&pow3, &ibz_const_three, EXPONENT_THREE); // pow3 = 3^e2

  // protocols_sign() returns 0 on success and 1 on failure.  The helper
  // functions used below use the opposite convention (1 on success).  The
  // outer retry loop follows the Julia implementation and keeps retrying
  // until a valid response is found.
  int ret = 0;

  ec_basis_t Bpk_can2, Bpk_can3_fix, Bchl;
  ec_point_t kernel_chl, pts[3];
  // int ok = ec_curve_to_basis_2f_from_hint(
  //     &Bpk_can2, &pk->curve, TORSION_PLUS_EVEN_POWER, pk->hint_pk_even);
  // assert(ok);

  if (timings)
    t_start = clock();
  int ok = ec_curve_to_basis_3f_from_hint(&Bpk_can3_fix, &pk->curve,
                                      pk->hint_pk_three);
  assert(ok);
  if (timings) {
    t_end = clock();
    timings->ms_sign_ec_non_isog +=
        (float)(t_end - t_start) * 1000.f / (float)CLOCKS_PER_SEC;
  }

  // Bpk_can2.P = phi_sk2(P0_EVEN)
  // Bpk_can2.Q = phi_sk2(Q0_EVEN)
  copy_point(&Bpk_can2.P, &(sk->phi_sk2_three.P));
  copy_point(&Bpk_can2.Q, &(sk->phi_sk2_three.Q));
  copy_point(&Bpk_can2.PmQ, &(sk->phi_sk2_three.PmQ));

  while (!ret) {
    // ---- commit ----
    t_start = clock();
    ret =
        commit(&E_com, &Bcom0, &lideal_commit_three, &kernel_isocom_three1,
               &kernel_isocom_three2, &E_com_mid, &Bcom0_mid, verbose, timings);
    if (timings){
      t_end = clock();
      timings->ms_commit +=
          (float)(t_end - t_start) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    // Do not derive a challenge from an unsuccessful commitment.  In
    // particular, E_com and the commitment ideal may not contain valid
    // outputs when commit() reports failure.
    if (!ret)
      continue;

    // ---- challenge ----
    t_step = clock();
    hash_to_challenge(&vec_can, &E_com, m, pk, l);
    if (timings)
      timings->ms_challenge +=
          (float)(clock() - t_step) * 1000.f / (float)CLOCKS_PER_SEC;
    ibz_to_digit_array(sig->challenge, &vec_can[1]);

    // ---- response ----
    t_step = clock();
    // the kernel of the challenge isogeny is generated by vec_can[0]*B[0] +
    // vec_can[1]*B[1] where B is the canonical basis of the 3^e2 torsion of Epk
    SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                    ec_biscalar_mul_ibz(&kernel_chl, &pk->curve, &vec_can[0],
                                        &vec_can[1], &Bpk_can3_fix));
#ifdef DEBUG
    assert(point_order_3f(&kernel_chl, &pk->curve, EXPONENT_THREE));
#endif

    ec_curve_t E_chl;
    ec_isog_odd_t phi_chl;

    isog_init_three(&phi_chl, &pk->curve, &kernel_chl, EXPONENT_THREE);
    copy_point(pts + 0, &Bpk_can2.P);
    copy_point(pts + 1, &Bpk_can2.Q);
    copy_point(pts + 2, &Bpk_can2.PmQ);

    {
      clock_t _t_isog = clock();
      ec_eval_three(&E_chl, &phi_chl, pts, 3);
      if (timings) {
        float _dt =
            (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
        timings->ms_sign_isog += _dt;
      }
    }
    // Bchl.P = phi_chl(phi_sk(P))
    // Bchl.Q = phi_chl(phi_sk(Q))
    copy_point(&Bchl.P, pts + 0);
    copy_point(&Bchl.Q, pts + 1);
    copy_point(&Bchl.PmQ, pts + 2);

    SIGN_TIME_CATEGORY(ms_sign_quat, {
      ibz_2x2_inv_mod(&mat_BAcan_to_BA0_three, &(sk->mat_BAcan_to_BA0_three),
                    &pow3);
      ibz_mat_2x2_eval(&vec, &(sk->mat_BAcan_to_BA0_three), &vec_can);
    });
    // the kernel of the challenge isogeny is generated by vec[0]*B0[0] +
    // vec[1]*B0[1] where B0 is the image through secret isogeny of the
    // canonical basis E0

    // t = tic();
    SIGN_TIME_CATEGORY(ms_sign_quat,
                    id2iso_kernel_dlogs_to_ideal_three(&lideal_chall_three,
                                                       &vec));
#ifdef DEBUG
    assert(ibz_cmp(&lideal_chall_three.norm, &TORSION_PLUS_3POWER) == 0);
#endif
    // TODO(optimization): only 3-torsion is used. Can optimise

    SIGN_TIME_CATEGORY(ms_sign_quat,
                    quat_lideal_inter(&lideal_chall3_secret2,
                                      &lideal_chall_three,
                                      &(sk->secret_ideal_two),
                                      &QUATALG_PINFTY));
    SIGN_TIME_CATEGORY(ms_sign_quat, {
      int direct_mul_ok = quat_lideal_mul_direct(
          &lideal_chall3_secret3, &lideal_chall3_secret2,
          &(sk->two_to_three_transporter), &QUATALG_PINFTY);
      assert(direct_mul_ok);
    });

    // Careful: want to intersect lideal_chall3_secret3 and
    // dual(lideal_commit_three) Both have norm a power of three, so trouble!!
    // First replace lideal_chall3_secret3 with an equivalent ideal of norm
    // coprime to 3; compute intersection with that, then sample in there, and
    // transport the result back to the wanted intersection

    SIGN_TIME_CATEGORY(ms_sign_quat,
                    quat_lideal_generator_coprime(
                        &elem_tmp, &lideal_chall3_secret3, &ibz_const_one,
                        &QUATALG_PINFTY, 0));
    SIGN_TIME_CATEGORY(ms_sign_quat, {
      quat_alg_conj(&elem_tmp, &elem_tmp);
      ibz_mul(&(elem_tmp.denom), &(elem_tmp.denom),
              &(lideal_chall3_secret3.norm));
    });

    SIGN_TIME_CATEGORY(ms_sign_quat, {
      int direct_mul_ok = quat_lideal_mul_direct(
          &lideal_tmp, &lideal_chall3_secret3, &elem_tmp, &QUATALG_PINFTY);
      assert(direct_mul_ok);
    });
    int test;
    SIGN_TIME_CATEGORY(ms_sign_quat,
                    test = quat_lideal_isom(&elem_tmp, &lideal_tmp,
                                            &lideal_chall3_secret3,
                                            &QUATALG_PINFTY));
    assert(test);

    SIGN_TIME_CATEGORY(ms_sign_quat,
                    quat_lideal_conjugate_lattice(&lat_commit,
                                                  &lideal_commit_three));

    SIGN_TIME_CATEGORY(ms_sign_quat, {
      /* conjugation maps the HNF basis of the commitment left ideal to a
       * right-ideal basis.  Restore HNF before passing it to lattice helpers. */
      quat_lattice_hnf(&lat_commit);

      /* Let I' = lideal_tmp and C = lideal_commit_three.  I' was chosen with
       * gcd(N(I'), N(C)) = 1, hence the compatible right/left ideal product
       *
       *              conjugate(C) * I'
       *
       * equals the Hom lattice I' intersect conjugate(C).  The order matters:
       * conjugate(C) is a right ideal and I' is a left ideal. */
      quat_lattice_mul(&lattice_hom_chall_to_com, &lat_commit,
                       &lideal_tmp.lattice, &QUATALG_PINFTY);
    });

#ifndef NDEBUG
    {
      quat_lattice_t intersection_check;
      quat_lattice_init(&intersection_check);
      quat_lattice_intersect(&intersection_check, &lideal_tmp.lattice,
                             &lat_commit);
      assert(quat_lattice_equal(&lattice_hom_chall_to_com,
                                &intersection_check));
      quat_lattice_finalize(&intersection_check);
    }
#endif

    // This lattice contains all isogenies from chall3_secret3 to
    // dual(commit_three), represented as conjugate(C) * I'.

    SIGN_TIME_CATEGORY(ms_sign_quat, {
      ibz_mul(&lattice_content, &(lideal_tmp.norm), &(lideal_commit_three.norm));
    });
    if (verbose)
      TOC(t, "sample_response in");

    // 失敗したら commit からやりなおす
    SIGN_TIME_CATEGORY(ms_sign_quat,
                    ret = sample_response(&resp_quat_2dpush,
                                          &lattice_hom_chall_to_com,
                                          &lattice_content, verbose, timings));
    if (!ret) {
      if (timings)
        timings->ms_response +=
            (float)(clock() - t_step) * 1000.f / CLOCKS_PER_SEC;
      continue;
    }
#ifndef NDEBUG
    assert(is_good_2dpush(&resp_quat_2dpush, &lattice_content));
#endif

    SIGN_TIME_CATEGORY(ms_sign_quat,
                    quat_alg_mul(&resp_quat_2dpush, &resp_quat_2dpush,
                                 &elem_tmp, &QUATALG_PINFTY));
    // bring it to intersection of lat_commit and lideal_chall3_secret3
    SIGN_TIME_CATEGORY(ms_sign_quat, {
      ibz_mul(&lattice_content, &(lideal_chall3_secret3.norm),
              &(lideal_commit_three.norm));

      quat_alg_normalize(&resp_quat_2dpush);
    });

#ifndef NDEBUG
    assert(is_good_2dpush(&resp_quat_2dpush, &lattice_content));
#endif

    if (verbose)
      TOC(t, "sample_response out");

#ifndef NDEBUG
    {
      ibq_t N_q;
      ibz_t N, tmp;

      ibq_init(&N_q);
      ibz_init(&N);
      ibz_init(&tmp);

      assert(quat_lattice_contains(NULL, &(lideal_chall3_secret3.lattice),
                                   &resp_quat_2dpush, &QUATALG_PINFTY));
      assert(!quat_lattice_contains(NULL, &(lideal_commit_three.lattice),
                                    &resp_quat_2dpush, &QUATALG_PINFTY));

      quat_alg_conj(&resp_quat_2dpush, &resp_quat_2dpush);
      assert(quat_lattice_contains(NULL, &(lideal_commit_three.lattice),
                                   &resp_quat_2dpush, &QUATALG_PINFTY));
      assert(!quat_lattice_contains(NULL, &(lideal_chall3_secret3.lattice),
                                    &resp_quat_2dpush, &QUATALG_PINFTY));
      quat_alg_conj(&resp_quat_2dpush, &resp_quat_2dpush); // repair

      quat_alg_norm(&N_q, &resp_quat_2dpush, &QUATALG_PINFTY);
      ibq_to_ibz(&N, &N_q);
      // ibz_printf("norm of response: %Zd\n", &N);

      ibz_mul(&tmp, &(lideal_chall3_secret3.norm), &(lideal_commit_three.norm));
      assert(ibz_divides(&N, &tmp));
      assert(is_good_2dpush(&resp_quat_2dpush, &tmp));

      ibq_finalize(&N_q);
      ibz_finalize(&N);
      ibz_finalize(&tmp);
    }
#endif

    ibz_t c;
    ibz_init(&c);
    int inv_res;
    SIGN_TIME_CATEGORY(ms_sign_quat, {
      ibz_mul(&c, &pow3, &pow3);
      ibz_mul(&c, &c, &pow3);
      inv_res = ibz_invmod(&c, &c, &pow2); // c = 3^(-3*e2) mod 2^e1
      assert(inv_res);

      quat_alg_conj(&resp_quat_2dpush_conj, &resp_quat_2dpush);
    });
    // quat_alg_normalize(&resp_quat_2dpush_conj);

    SIGN_TIME_CATEGORY(ms_sign_quat,
                    matrix_of_endomorphism_even(&mat_alpha0,
                                                &resp_quat_2dpush_conj));

    SIGN_TIME_CATEGORY(ms_sign_quat, {
      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
          ibz_mul(&mat_alpha0[i][j], &mat_alpha0[i][j], &c);
          ibz_mod(&mat_alpha0[i][j], &mat_alpha0[i][j], &pow2);
        }
      }
      ibz_finalize(&c);
    });

    ec_basis_t PQ_rsp; // phi_rsp
    copy_point(&PQ_rsp.P, &Bchl.P);
    copy_point(&PQ_rsp.Q, &Bchl.Q);
    copy_point(&PQ_rsp.PmQ, &Bchl.PmQ);
    {
      clock_t _t_isog = clock();
      matrix_application_even_basis(&PQ_rsp, &E_chl, &mat_alpha0, EXPONENT_TWO);
      if (timings) {
        float _dt =
            (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
        timings->ms_sign_ec_non_isog += _dt;
      }
    }

    ec_basis_t Bchl_can2;
    SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                    sig->hint_chall = ec_curve_to_basis_2f_to_hint(
                        &Bchl_can2, &E_chl, TORSION_PLUS_EVEN_POWER));
    // ec_curve_to_basis_2(&Bchl_can2, &E_chl);

    ibz_mat_2x2_t mat_chl;
    ibz_mat_2x2_init(&mat_chl);
    SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                    change_of_basis_matrix_two(&mat_chl, &PQ_rsp, &Bchl_can2,
                                               &E_chl));

    ibq_t norm_q;
    ibz_t d_rsp, remain;
    ibq_init(&norm_q);
    ibz_init(&d_rsp);
    ibz_init(&remain);

    ibz_t d_rsp_odd;
    ibz_init(&d_rsp_odd);
    ibz_t d_aux;
    ibz_init(&d_aux);
    ibz_t d_aux_coprime6;
    ibz_init(&d_aux_coprime6);
    unsigned long n1, n2;

    SIGN_TIME_CATEGORY(ms_sign_quat, {
      quat_alg_norm(&norm_q, &resp_quat_2dpush, &QUATALG_PINFTY);
      ibq_to_ibz(&d_rsp, &norm_q);

      ibz_div(&d_rsp, &remain, &d_rsp, &lattice_content);
      assert(ibz_is_zero(&remain));

      // d_rsp = d_rsp_odd * 2^n1
      n1 = mpz_scan1(d_rsp, 0);
      ibz_div_2exp(&d_rsp_odd, &d_rsp, (uint64_t)n1);

      sig->n1 = (uint16_t)n1;

      // d_aux = 2^(e1 - n1) - d_rsp_odd
      ibz_set(&d_aux, 1);
      ibz_div_2exp(&d_aux, &pow2, (uint64_t)n1);
      ibz_sub(&d_aux, &d_aux, &d_rsp_odd);

      n2 = mpz_remove(d_aux_coprime6, d_aux, ibz_const_three);
    });

#ifndef NDEBUG
    {
      ibz_t gcd6, val6;
      ibz_init(&gcd6);
      ibz_init(&val6);
      ibz_set(&val6, 6);
      ibz_gcd(&gcd6, &d_aux_coprime6, &val6);
      assert(ibz_is_one(&gcd6));
      ibz_finalize(&gcd6);
      ibz_finalize(&val6);
    }
#endif

    ec_curve_t E_aux;
    ec_basis_t image_pushrandisog;

    // The callee records its own category timings.
    ret = pushrandisog(&E_aux, &image_pushrandisog,
                                       &d_aux_coprime6, &kernel_isocom_three1,
                                       &kernel_isocom_three2, &E_com_mid,
                                       &Bcom0_mid, (int)n1, &pow2, &pow3,
                                       timings);

    if (!ret) {
      // pushrandisog() may fail after allocating all response-local
      // objects above (for example when the theta chain fails).  Do not use
      // its output objects, and do not let the next retry reuse finalized
      // GMP objects.
      if (timings)
        timings->ms_response +=
            (float)(clock() - t_step) * 1000.f / CLOCKS_PER_SEC;
      ibz_mat_2x2_finalize(&mat_chl);
      ibq_finalize(&norm_q);
      ibz_finalize(&d_rsp);
      ibz_finalize(&remain);
      ibz_finalize(&d_rsp_odd);
      ibz_finalize(&d_aux);
      ibz_finalize(&d_aux_coprime6);
      continue;
    }

    ec_basis_t RS_aux;
    copy_point(&RS_aux.P, &image_pushrandisog.P);
    copy_point(&RS_aux.Q, &image_pushrandisog.Q);
    copy_point(&RS_aux.PmQ, &image_pushrandisog.PmQ);

    int is_odd = (int)(n2 % 2 == 1);
    unsigned long itr_tpl;

    clock_t t_aux_odd_adjust = clock();
    if (is_odd) {
      ec_point_t ker_iso3, push_pts[3];
      ec_isog_odd_t iso3;
      ec_point_init(&ker_iso3);

      copy_point(&push_pts[0], &RS_aux.P);
      copy_point(&push_pts[1], &RS_aux.Q);
      copy_point(&push_pts[2], &RS_aux.PmQ);

      ec_random_point_order_3(&ker_iso3, &E_aux);
#ifdef DEBUG
      assert(point_order_3f(&ker_iso3, &E_aux, 1));
#endif

      isog_init_3(&iso3, &E_aux, &ker_iso3);
      if (timings)
        timings->ms_sign_ec_non_isog +=
            (float)(clock() - t_aux_odd_adjust) * 1000.f / CLOCKS_PER_SEC;
      SIGN_TIME_CATEGORY(ms_sign_isog,
                         ec_eval_three(&E_aux, &iso3, push_pts, 3));
      t_aux_odd_adjust = timings ? clock() : 0;

      copy_point(&RS_aux.P, &push_pts[0]);
      copy_point(&RS_aux.Q, &push_pts[1]);
      copy_point(&RS_aux.PmQ, &push_pts[2]);

      itr_tpl = (n2 - 1) / 2;

      ec_point_t A3;
      fp2_copy(&A3.x, &E_aux.A24.x);
      fp2_copy(&A3.z, &A3.x);
      fp2_sub(&A3.z, &A3.z, &E_aux.A24.z);

      for (unsigned long i = 0; i < itr_tpl; i++) {
        TPL_A3(&RS_aux.P, &RS_aux.P, &A3);
        TPL_A3(&RS_aux.Q, &RS_aux.Q, &A3);
        TPL_A3(&RS_aux.PmQ, &RS_aux.PmQ, &A3);
      }
    } else {
      itr_tpl = n2 / 2;

      ec_point_t A3;
      fp2_copy(&A3.x, &E_aux.A24.x);
      fp2_copy(&A3.z, &A3.x);
      fp2_sub(&A3.z, &A3.z, &E_aux.A24.z);

      for (unsigned long i = 0; i < itr_tpl; i++) {
        TPL_A3(&RS_aux.P, &RS_aux.P, &A3);
        TPL_A3(&RS_aux.Q, &RS_aux.Q, &A3);
        TPL_A3(&RS_aux.PmQ, &RS_aux.PmQ, &A3);
      }
    }
    if (timings) {
      timings->ms_sign_ec_non_isog +=
          (float)(clock() - t_aux_odd_adjust) * 1000.f /
          (float)CLOCKS_PER_SEC;
    }

    copy_curve(&sig->E_aux, &E_aux);

    ec_basis_t Baux_can;
    SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                    sig->hint_aux = ec_curve_to_basis_2f_to_hint(
                        &Baux_can, &E_aux, TORSION_PLUS_EVEN_POWER));

    ibz_mat_2x2_t mat_aux;
    ibz_mat_2x2_init(&mat_aux);

    SIGN_TIME_CATEGORY(ms_sign_ec_non_isog,
                    change_of_basis_matrix_two(&mat_aux, &RS_aux, &Baux_can,
                                               &E_aux));

    ibz_mat_2x2_t mat_aux_inv;
    ibz_mat_2x2_init(&mat_aux_inv);

    ibz_mat_2x2_t mat_rsp;
    ibz_mat_2x2_init(&mat_rsp);
    SIGN_TIME_CATEGORY(ms_sign_quat, {
      ibz_2x2_inv_mod(&mat_aux_inv, &mat_aux, &pow2);
      ibz_2x2_mul_mod(&mat_rsp, &mat_chl, &mat_aux_inv, &pow2);
      ibz_mat_2x2_copy(&sig->mat_rsp, &mat_rsp);
    });

    if (timings)
      timings->ms_response +=
          (float)(clock() - t_step) * 1000.f / (float)CLOCKS_PER_SEC;

    ibz_mat_2x2_finalize(&mat_chl);
    ibz_mat_2x2_finalize(&mat_aux);
    ibz_mat_2x2_finalize(&mat_aux_inv);
    ibz_mat_2x2_finalize(&mat_rsp);
    ibq_finalize(&norm_q);
    ibz_finalize(&d_rsp);
    ibz_finalize(&remain);
    ibz_finalize(&d_rsp_odd);
    ibz_finalize(&d_aux);
    ibz_finalize(&d_aux_coprime6);
  }

  // Objects shared by all retry attempts are finalized only once, after the
  // retry loop.  This is important when a response attempt fails and the
  // loop is entered again.
  ibz_vec_2_finalize(&vec);
  ibz_vec_2_finalize(&vec_can);
  ibz_mat_2x2_finalize(&mat_alpha0);
  ibz_mat_2x2_finalize(&mat_Bcom0_to_Bcom);
  ibz_mat_2x2_finalize(&mat_sigma_phichall_BA_to_Bcomcan);
  ibz_mat_2x2_finalize(&mat_sigma_phichall_BA0_to_Bcom0);
  ibz_mat_2x2_finalize(&mat_BAcan_to_BA0_two);
  ibz_mat_2x2_finalize(&mat_BAcan_to_BA0_three);

  quat_alg_elem_finalize(&resp_quat);
  quat_alg_elem_finalize(&elem_tmp);
  quat_alg_elem_finalize(&resp_quat_2dpush);
  quat_alg_elem_finalize(&resp_quat_2dpush_conj);
  quat_lattice_finalize(&lattice_hom_chall_to_com);
  quat_lattice_finalize(&lat_commit);
  quat_left_ideal_finalize(&lideal_commit_three);
  quat_left_ideal_finalize(&lideal_chall_three);
  quat_left_ideal_finalize(&lideal_chall3_secret2);
  quat_left_ideal_finalize(&lideal_chall3_secret3);
  quat_left_ideal_finalize(&lideal_tmp);

  ibz_finalize(&degree_com_isogeny);
  ibz_finalize(&tmp);
  ibz_finalize(&lattice_content);
  ibz_finalize(&pow2);
  ibz_finalize(&pow3);

  // protocols_sign() uses 0 for success for compatibility with its callers.
  return ret ? 0 : 1;
}
