#include "isog.h"
#include <assert.h>
#include <ec.h>
#include <sqisign_namespace.h>

#ifndef BASIS_EVEN_DECLARED
#define BASIS_EVEN_DECLARED
extern const ec_basis_t BASIS_EVEN;
#endif

static inline void AC_to_A24(ec_point_t *A24, ec_curve_t const *E) {
  // A24 = (A+2C : 4C)
  fp2_add(&A24->z, &E->C, &E->C);
  fp2_add(&A24->x, &E->A, &A24->z);
  fp2_add(&A24->z, &A24->z, &A24->z);
}

static void xTPL(ec_point_t *Q, const ec_point_t *P, const ec_point_t *A3) {
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

int ec_is_on_curve(const ec_curve_t *curve, const ec_point_t *P) {

  fp2_t t0, t1, t2;

  // Check if xz*(C^2x^2+zACx+z^2C^2) is a square
  fp2_mul(&t0, &curve->C, &P->x);
  fp2_mul(&t1, &t0, &P->z);
  fp2_mul(&t1, &t1, &curve->A);
  fp2_mul(&t2, &curve->C, &P->z);
  fp2_sqr(&t0, &t0);
  fp2_sqr(&t2, &t2);
  fp2_add(&t0, &t0, &t1);
  fp2_add(&t0, &t0, &t2);
  fp2_mul(&t0, &t0, &P->x);
  fp2_mul(&t0, &t0, &P->z);
  return fp2_is_square(&t0);
}

void difference_point(ec_point_t *PQ, const ec_point_t *P, const ec_point_t *Q,
                      const ec_curve_t *curve) {
  // Given P,Q in projective x-only, computes a deterministic choice for (P-Q)
  // Based on Proposition 3 of https://eprint.iacr.org/2017/518.pdf

  fp2_t Bxx, Bxz, Bzz, t0, t1;

  fp2_mul(&t0, &P->x, &Q->x);
  fp2_mul(&t1, &P->z, &Q->z);
  fp2_sub(&Bxx, &t0, &t1);
  fp2_sqr(&Bxx, &Bxx);
  fp2_mul(&Bxx, &Bxx, &curve->C); // C*(P.x*Q.x-P.z*Q.z)^2
  fp2_add(&Bxz, &t0, &t1);
  fp2_mul(&t0, &P->x, &Q->z);
  fp2_mul(&t1, &P->z, &Q->x);
  fp2_add(&Bzz, &t0, &t1);
  fp2_mul(&Bxz, &Bxz, &Bzz); // (P.x*Q.x+P.z*Q.z)(P.x*Q.z+P.z*Q.x)
  fp2_sub(&Bzz, &t0, &t1);
  fp2_sqr(&Bzz, &Bzz);
  fp2_mul(&Bzz, &Bzz, &curve->C); // C*(P.x*Q.z-P.z*Q.x)^2
  fp2_mul(&Bxz, &Bxz, &curve->C); // C*(P.x*Q.x+P.z*Q.z)(P.x*Q.z+P.z*Q.x)
  fp2_mul(&t0, &t0, &t1);
  fp2_mul(&t0, &t0, &curve->A);
  fp2_add(&t0, &t0, &t0);
  fp2_add(&Bxz, &Bxz,
          &t0); // C*(P.x*Q.x+P.z*Q.z)(P.x*Q.z+P.z*Q.x) + 2*A*P.x*Q.z*P.z*Q.x

  // To ensure that the denominator is a fourth power in Fp, we normalize by
  // C*C_bar^2*(P.z)_bar^2*(Q.z)_bar^2
  fp_copy(t0.re, curve->C.re);
  fp_neg(t0.im, curve->C.im);
  fp2_sqr(&t0, &t0);
  fp2_mul(&t0, &t0, &curve->C);
  fp_copy(t1.re, P->z.re);
  fp_neg(t1.im, P->z.im);
  fp2_sqr(&t1, &t1);
  fp2_mul(&t0, &t0, &t1);
  fp_copy(t1.re, Q->z.re);
  fp_neg(t1.im, Q->z.im);
  fp2_sqr(&t1, &t1);
  fp2_mul(&t0, &t0, &t1);
  fp2_mul(&Bxx, &Bxx, &t0);
  fp2_mul(&Bxz, &Bxz, &t0);
  fp2_mul(&Bzz, &Bzz, &t0);

  // Solving quadratic equation
  fp2_sqr(&t0, &Bxz);
  fp2_mul(&t1, &Bxx, &Bzz);
  fp2_sub(&t0, &t0, &t1);
  // No need to check if t0 is square, as per the entangled basis algorithm.
  fp2_sqrt(&t0);
  fp2_add(&PQ->x, &Bxz, &t0);
  fp2_copy(&PQ->z, &Bzz);
}

void ec_curve_to_basis_2(ec_basis_t *PQ2, const ec_curve_t *curve) {
  fp2_t x, t0, t1, t2;
  ec_point_t P, Q, Q2, P2, A24;

  // Curve coefficient in the form A24 = (A+2C:4C)
  if (fp2_is_zero(&curve->A24.z)) {
    A24_from_AC(&A24, curve);
  } else {
    copy_point(&A24, &curve->A24);
  }

  fp_mont_setone(x.re);
  fp_set(x.im, 0);

  fp2_t C2, AC;
  fp2_sqr(&C2, &curve->C);
  fp2_mul(&AC, &curve->A, &curve->C);

  // Find P
  while (1) {
    fp_add(x.im, x.re, x.im);

    // Check if point is rational
    fp2_mul(&t1, &C2, &x);
    fp2_add(&t1, &t1, &AC);
    fp2_mul(&t1, &t1, &x);
    fp2_add(&t1, &t1, &C2);
    fp2_mul(&t1, &t1, &x);
    if (fp2_is_square(&t1)) {
      fp2_copy(&P.x, &x);
      fp_mont_setone(P.z.re);
      fp_set(P.z.im, 0);
    } else
      continue;

    // Clear odd factors from the order
    xMULv2(&P, &P, p_cofactor_for_2f, P_COFACTOR_FOR_2F_BITLENGTH, &A24);

    // Check if point has order 2^f
    copy_point(&P2, &P);
    for (int i = 0; i < POWER_OF_2 - 1; i++)
      xDBLv2(&P2, &P2, &A24);
    if (ec_is_zero(&P2))
      continue;
    else
      break;
  }

  // Find Q
  while (1) {
    fp_add(x.im, x.re, x.im);

    // Check if point is rational
    fp2_mul(&t1, &C2, &x);
    fp2_add(&t1, &t1, &AC);
    fp2_mul(&t1, &t1, &x);
    fp2_add(&t1, &t1, &C2);
    fp2_mul(&t1, &t1, &x);
    if (fp2_is_square(&t1)) {
      fp2_copy(&Q.x, &x);
      fp_mont_setone(Q.z.re);
      fp_set(Q.z.im, 0);
    } else
      continue;

    // Clear odd factors from the order
    xMULv2(&Q, &Q, p_cofactor_for_2f, P_COFACTOR_FOR_2F_BITLENGTH, &A24);

    // Check if point has order 2^f
    copy_point(&Q2, &Q);
    for (int i = 0; i < POWER_OF_2 - 1; i++)
      xDBLv2(&Q2, &Q2, &A24);
    if (ec_is_zero(&Q2))
      continue;

    // Check if point is orthogonal to P
    if (is_point_equal(&P2, &Q2))
      continue;
    else
      break;
  }

  // Normalize points
  ec_curve_t E;
  fp2_mul(&t0, &P.z, &Q.z);
  fp2_mul(&t1, &t0, &curve->C);
  fp2_inv(&t1);
  fp2_mul(&P.x, &P.x, &t1);
  fp2_mul(&Q.x, &Q.x, &t1);
  fp2_mul(&E.A, &curve->A, &t1);
  fp2_mul(&P.x, &P.x, &Q.z);
  fp2_mul(&P.x, &P.x, &curve->C);
  fp2_mul(&Q.x, &Q.x, &P.z);
  fp2_mul(&Q.x, &Q.x, &curve->C);
  fp2_mul(&E.A, &E.A, &t0);
  fp_mont_setone(P.z.re);
  fp_set(P.z.im, 0);
  fp2_copy(&Q.z, &P.z);
  fp2_copy(&E.C, &P.z);
  AC_to_A24(&E.A24, &E);

  // Compute P-Q
  difference_point(&PQ2->PmQ, &P, &Q, &E);
  copy_point(&PQ2->P, &P);
  copy_point(&PQ2->Q, &Q);
}

void ec_complete_basis_2(ec_basis_t *PQ2, const ec_curve_t *curve,
                         const ec_point_t *P) {

  fp2_t x, t0, t1, t2;
  ec_point_t Q, Q2, P2, A24;

  // Curve coefficient in the form A24 = (A+2C:4C)
  if (fp2_is_zero(&curve->A24.z)) {
    A24_from_AC(&A24, curve);
  } else {
    copy_point(&A24, &curve->A24);
  }

  // Point of order 2 generated by P
  copy_point(&P2, P);
  for (int i = 0; i < POWER_OF_2 - 1; i++)
    xDBLv2(&P2, &P2, &A24);

  // Find Q
  fp_mont_setone(x.re);
  fp_set(x.im, 0);

  fp2_t C2, AC;
  fp2_sqr(&C2, &curve->C);
  fp2_mul(&AC, &curve->A, &curve->C);

  while (1) {
    fp_add(x.im, x.re, x.im);

    // Check if point is rational
    fp2_mul(&t1, &C2, &x);
    fp2_add(&t1, &t1, &AC);
    fp2_mul(&t1, &t1, &x);
    fp2_add(&t1, &t1, &C2);
    fp2_mul(&t1, &t1, &x);
    if (fp2_is_square(&t1)) {
      fp2_copy(&Q.x, &x);
      fp_mont_setone(Q.z.re);
      fp_set(Q.z.im, 0);
    } else
      continue;

    // Clear odd factors from the order
    xMULv2(&Q, &Q, p_cofactor_for_2f, P_COFACTOR_FOR_2F_BITLENGTH, &A24);

    // Check if point has order 2^f
    copy_point(&Q2, &Q);
    for (int i = 0; i < POWER_OF_2 - 1; i++)
      xDBLv2(&Q2, &Q2, &A24);
    if (ec_is_zero(&Q2))
      continue;

    // Check if point is orthogonal to P
    if (is_point_equal(&P2, &Q2))
      continue;
    else
      break;
  }

  // Normalize points
  ec_curve_t E;
  ec_point_t PP;
  fp2_mul(&t0, &P->z, &Q.z);
  fp2_mul(&t1, &t0, &curve->C);
  fp2_inv(&t1);
  fp2_mul(&PP.x, &P->x, &t1);
  fp2_mul(&Q.x, &Q.x, &t1);
  fp2_mul(&E.A, &curve->A, &t1);
  fp2_mul(&PP.x, &PP.x, &Q.z);
  fp2_mul(&PP.x, &PP.x, &curve->C);
  fp2_mul(&Q.x, &Q.x, &P->z);
  fp2_mul(&Q.x, &Q.x, &curve->C);
  fp2_mul(&E.A, &E.A, &t0);
  fp_mont_setone(PP.z.re);
  fp_set(PP.z.im, 0);
  fp2_copy(&Q.z, &PP.z);
  fp2_copy(&E.C, &PP.z);
  AC_to_A24(&E.A24, &E);

  // Compute P-Q
  difference_point(&PQ2->PmQ, &PP, &Q, &E);
  copy_point(&PQ2->P, &PP);
  copy_point(&PQ2->Q, &Q);
}

// void ec_normalize_curve_and_A24(
//     ec_curve_t
//         *E) { // Neither the curve or A24 are guaranteed to be normalized.
//               // First we normalize (A/C : 1) and conditionally compute
//   if (!fp2_is_one(&E->C)) {
//     ec_normalize_curve(E);
//   }

//   fp2_t one, two_inv;
//   fp2_set_one(&one);
//   fp2_add(&two_inv, &one, &one);
//   fp2_inv(&two_inv);

//   fp2_add(&E->A24.x, &E->A, &one);     // re(A24.x) = re(A) + 1
//   fp2_add(&E->A24.x, &E->A24.x, &one); // re(A24.x) = re(A) + 2
//   fp_copy(E->A24.x.im, E->A.im);       // im(A24.x) = im(A)

//   fp2_mul(&E->A24.x, &E->A24.x, &two_inv); // (A + 2) / 2
//   fp2_mul(&E->A24.x, &E->A24.x, &two_inv); // (A + 2) / 4
//   fp2_set_one(&E->A24.z);
// }

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

// // The entangled basis generation does not allow A = 0
// // so we simply return the one we have already precomputed
// static void ec_basis_E0_2f(ec_basis_t *PQ2, ec_curve_t *curve, int f) {
//   assert(fp2_is_zero(&curve->A));
//   ec_point_t P, Q;

//   // Set P, Q to precomputed (X : 1) values
//   fp2_copy(&P.x, &BASIS_EVEN.P.x);
//   fp2_copy(&Q.x, &BASIS_EVEN.Q.x);
//   fp2_set_one(&P.z);
//   fp2_set_one(&Q.z);

//   // clear the power of two to get a point of order 2^f
//   for (int i = 0; i < TORSION_PLUS_EVEN_POWER - f; i++) {
//     xDBL_E0(&P, &P);
//     xDBL_E0(&Q, &Q);
//   }

//   // Set P, Q in the basis and compute x(P - Q)
//   copy_point(&PQ2->P, &P);
//   copy_point(&PQ2->Q, &Q);
//   difference_point(&PQ2->PmQ, &P, &Q, curve);
// }

// // Given an x-coordinate, determines if this is a valid
// // point on the curve. Assumes C=1.
// static uint32_t is_on_curve(const fp2_t *x, const ec_curve_t *curve) {
//   assert(fp2_is_one(&curve->C));
//   fp2_t t0, one;
//   fp2_set_one(&one);

//   fp2_add(&t0, x, &curve->A); // x + (A/C)
//   fp2_mul(&t0, &t0, x);       // x^2 + (A/C)*x
//   fp2_add(&t0, &t0, &one);    // x^2 + (A/C)*x + 1
//   fp2_mul(&t0, &t0, x);       // x^3 + (A/C)*x^2 + x

//   return fp2_is_square(&t0);
// }

// // Helper function which finds a point x(P) = n * A
// static uint8_t find_nA_x_coord(fp2_t *x, ec_curve_t *curve,
//                                const uint8_t start) {
//   assert(!fp2_is_square(&curve->A)); // Only to be called when A is a NQR

//   // when A is NQR we allow x(P) to be a multiple n*A of A
//   uint8_t n = start;
//   fp2_t tmp_n;
//   if (n == 1) {
//     fp2_copy(x, &curve->A);
//   } else {
//     // fp2_mul_small(x, &curve->A, n);
//     fp2_set(&tmp_n, n);
//     fp2_mul(x, &curve->A, &tmp_n);
//   }

//   while (!is_on_curve(x, curve)) {
//     fp2_add(x, x, &curve->A);
//     n++;
//   }

//   /*
//    * With very low probability (1/2^128), n will not fit in 7 bits.
//    * In this case, we set hint = 0 which signals failure and the need
//    * to generate a value on the fly during verification
//    */
//   uint8_t hint = n < 128 ? n : 0;
//   return hint;
// }

// // Helper function which finds an NQR -1 / (1 + i*b) for entangled basis
// // generation
// static uint8_t find_nqr_factor(fp2_t *x, ec_curve_t *curve,
//                                const uint8_t start) {
//   // factor = -1/(1 + i*b) for b in Fp will be NQR whenever 1 + b^2 is NQR
//   // in Fp, so we find one of these and then invert (1 + i*b). We store b
//   // as a u8 hint to save time in verification.

//   // We return the hint as a u8, but use (uint16_t)n to give 2^16 - 1
//   // to make failure cryptographically negligible, with a fallback when
//   // n > 128 is required.
//   uint8_t hint;
//   uint32_t found = 0;
//   uint16_t n = start;

//   bool qr_b = 1;
//   fp_t b, tmp;
//   fp2_t z, t0, t1;

//   do {
//     while (qr_b) {
//       // find b with 1 + b^2 a non-quadratic residue
//       // fp_set_small(&tmp, (uint32_t)n * n + 1);
//       fp_set(tmp, (uint32_t)n * n + 1);
//       qr_b = fp_is_square(tmp);
//       n++; // keeps track of b = n - 1
//     }

//     // for Px := -A/(1 + i*b) to be on the curve
//     // is equivalent to A^2*(z-1) - z^2 NQR for z = 1 + i*b
//     // thus prevents unnecessary inversion pre-check

//     // t0 = z - 1 = i*b
//     // t1 = z = 1 + i*b
//     // fp_set_small(&b, (uint32_t)n - 1);
//     fp_set(b, (uint32_t)n - 1);
//     fp2_set(&t0, 0);
//     fp2_set_one(&z);
//     fp_copy(z.im, b);
//     fp_copy(t0.im, b);

//     // A^2*(z-1) - z^2
//     fp2_sqr(&t1, &curve->A);
//     fp2_mul(&t0, &t0, &t1); // A^2 * (z - 1)
//     fp2_sqr(&t1, &z);
//     fp2_sub(&t0, &t0, &t1); // A^2 * (z - 1) - z^2
//     found = !fp2_is_square(&t0);

//     qr_b = 1;
//   } while (!found);

//   // set Px to -A/(1 + i*b)
//   fp2_copy(x, &z);
//   fp2_inv(x);
//   fp2_mul(x, x, &curve->A);
//   fp2_neg(x, x);

//   /*
//    * With very low probability n will not fit in 7 bits.
//    * We set hint = 0 which signals failure and the need
//    * to generate a value on the fly during verification
//    */
//   hint = n <= 128 ? n - 1 : 0;

//   return hint;
// }

// void xDBL_A24(ec_point_t *Q, const ec_point_t *P,
//               const ec_point_t *A24) { // Doubling of a Montgomery point in
//                                        // projective coordinates
//   // (X:Z). Input: projective Montgomery x-coordinates P =
//   // (XP:ZP), where xP=XP/ZP, and
//   //        the Montgomery curve constants A24 = (A+2C:4C) (or A24 =
//   (A+2C/4C:1)
//   //        if normalized).
//   // Output: projective Montgomery x-coordinates Q <- 2*P =
//   // (XQ:ZQ) such that x(2P)=XQ/ZQ.
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

// // Helper function which given a point of order k*2^n with n maximal
// // and k odd, computes a point of order 2^f
// static inline void
// clear_cofactor_for_maximal_even_order(ec_point_t *P, ec_curve_t *curve, int
// f) {
//   // clear out the odd cofactor to get a point of order 2^n
//   xMULv2(P, P, p_cofactor_for_2f, P_COFACTOR_FOR_2F_BITLENGTH, &curve->A24);

//   // clear the power of two to get a point of order 2^f
//   for (int i = 0; i < TORSION_PLUS_EVEN_POWER - f; i++) {
//     xDBL_A24(P, P, &curve->A24);
//   }
// }

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

//   // clear out the odd cofactor to get a point of order 2^f
//   clear_cofactor_for_maximal_even_order(&P, curve, f);
//   clear_cofactor_for_maximal_even_order(&Q, curve, f);

//   // compute PmQ, set PmQ to Q to ensure Q above (0,0)
//   difference_point(&PQ2->Q, &P, &Q, curve);
//   copy_point(&PQ2->P, &P);
//   copy_point(&PQ2->PmQ, &Q);

//   // Finally, we compress hint_A and hint into a single bytes.
//   // We choose to set the LSB of hint to hint_A
//   assert(hint < 128); // We expect hint to be 7-bits in size
//   return (hint << 1) | hint_A;
// }

void ec_curve_to_basis_3(ec_basis_t *PQ3, const ec_curve_t *curve) {

  fp2_t x, t0, t1, t2;
  ec_point_t P, Q, Q3, P3, A24, A3;

  // Curve coefficient in the form A24 = (A+2C:4C)
  if (fp2_is_zero(&curve->A24.z)) {
    A24_from_AC(&A24, curve);
  } else {
    copy_point(&A24, &curve->A24);
  }

  // Curve coefficient in the form A3 = (A+2C:A-2C)
  fp2_sub(&A3.z, &A24.x, &A24.z);
  fp2_copy(&A3.x, &A24.x);

  fp_mont_setone(x.re);
  fp_set(x.im, 0);

  fp2_t C2, AC;
  fp2_sqr(&C2, &curve->C);
  fp2_mul(&AC, &curve->A, &curve->C);

  fp2_t K0, K1, K2, K3, D1, D2, D3;
  // K0 = 2*C2 + AC
  fp2_add(&K0, &C2, &C2);
  fp2_add(&K0, &K0, &AC);

  // K1 = 2 * K0 * i
  fp2_add(&K1, &K0, &K0);
  fp_t tmp;
  fp_copy(tmp, K1.re);
  fp_neg(K1.re, K1.im);
  fp_copy(K1.im, tmp);

  // K2 = -3C2 - AC = -K0 - C2
  fp2_add(&K2, &K0, &C2);
  fp2_neg(&K2, &K2);

  // K3 = -C2 * i
  fp_copy(K3.re, C2.im);
  fp_neg(K3.im, C2.re);

  // D3 = 6 K3
  fp2_add(&D3, &K3, &K3);
  fp2_add(&t0, &D3, &K3);
  fp2_add(&D3, &t0, &t0);

  // D2 = 2 K2 + 6 K3 = 2 K2 + D3
  fp2_add(&D2, &K2, &K2);
  fp2_add(&D2, &D2, &D3);

  // D1 = K1 + K2 + K3
  fp2_add(&D1, &K1, &K2);
  fp2_add(&D1, &D1, &K3);

  fp2_copy(&t1, &K0);

  // Find P
  while (1) {
    fp_add(x.im, x.re, x.im);
    fp2_add(&t1, &t1, &D1);
    fp2_add(&D1, &D1, &D2);
    fp2_add(&D2, &D2, &D3);

    // Check if point is rational
    if (fp2_is_square(&t1)) {
      fp2_copy(&P.x, &x);
      fp_mont_setone(P.z.re);
      fp_set(P.z.im, 0);
    } else
      continue;

    // Clear non-3 factors from the order
    xMULv2(&P, &P, p_cofactor_for_3g, P_COFACTOR_FOR_3G_BITLENGTH, &A24);

    // Check if point has order 3^g
    copy_point(&P3, &P);
    for (int i = 0; i < POWER_OF_3 - 1; i++)
      xTPL(&P3, &P3, &A3);
    if (ec_is_zero(&P3))
      continue;
    else
      break;
  }

  // Find Q
  while (1) {
    fp_add(x.im, x.re, x.im);
    fp2_add(&t1, &t1, &D1);
    fp2_add(&D1, &D1, &D2);
    fp2_add(&D2, &D2, &D3);

    // Check if point is rational
    if (fp2_is_square(&t1)) {
      fp2_copy(&Q.x, &x);
      fp_mont_setone(Q.z.re);
      fp_set(Q.z.im, 0);
    } else
      continue;

    // Clear non-3 factors from the order
    xMULv2(&Q, &Q, p_cofactor_for_3g, P_COFACTOR_FOR_3G_BITLENGTH, &A24);

    // Check if point has order 3^g
    copy_point(&Q3, &Q);
    for (int i = 0; i < POWER_OF_3 - 1; i++)
      xTPL(&Q3, &Q3, &A3);
    if (ec_is_zero(&Q3))
      continue;

    // Check if point is orthogonal to P
    if (is_point_equal(&P3, &Q3))
      continue;
    else
      break;
  }

  // Normalize points
  ec_curve_t E;
  fp2_mul(&t0, &P.z, &Q.z);
  fp2_mul(&t1, &t0, &curve->C);
  fp2_inv(&t1);
  fp2_mul(&P.x, &P.x, &t1);
  fp2_mul(&Q.x, &Q.x, &t1);
  fp2_mul(&E.A, &curve->A, &t1);
  fp2_mul(&P.x, &P.x, &Q.z);
  fp2_mul(&P.x, &P.x, &curve->C);
  fp2_mul(&Q.x, &Q.x, &P.z);
  fp2_mul(&Q.x, &Q.x, &curve->C);
  fp2_mul(&E.A, &E.A, &t0);
  fp_mont_setone(P.z.re);
  fp_set(P.z.im, 0);
  fp2_copy(&Q.z, &P.z);
  fp2_copy(&E.C, &P.z);
  AC_to_A24(&E.A24, &E);

  // Compute P-Q
  difference_point(&PQ3->PmQ, &P, &Q, &E);
  copy_point(&PQ3->P, &P);
  copy_point(&PQ3->Q, &Q);
}

uint16_t ec_curve_to_basis_3f_to_hint(ec_basis_t *PQ3,
                                      const ec_curve_t *curve) {

  fp2_t x, t0, t1, t2;
  ec_point_t P, Q, Q3, P3, A24, A3;

  // Curve coefficient in the form A24 = (A+2C:4C)
  if (fp2_is_zero(&curve->A24.z)) {
    A24_from_AC(&A24, curve);
  } else {
    copy_point(&A24, &curve->A24);
  }

  // Curve coefficient in the form A3 = (A+2C:A-2C)
  fp2_sub(&A3.z, &A24.x, &A24.z);
  fp2_copy(&A3.x, &A24.x);

  fp_mont_setone(x.re);
  fp_set(x.im, 0);

  fp2_t C2, AC;
  fp2_sqr(&C2, &curve->C);
  fp2_mul(&AC, &curve->A, &curve->C);

  // Find P: count iterations until we get a point of order 3^g
  uint16_t hint1 = 0;
  while (1) {
    hint1++;
    fp_add(x.im, x.re, x.im);

    // Check if point is rational
    fp2_mul(&t1, &C2, &x);
    fp2_add(&t1, &t1, &AC);
    fp2_mul(&t1, &t1, &x);
    fp2_add(&t1, &t1, &C2);
    fp2_mul(&t1, &t1, &x);
    if (!fp2_is_square(&t1))
      continue;

    fp2_copy(&P.x, &x);
    fp_mont_setone(P.z.re);
    fp_set(P.z.im, 0);

    // Clear non-3 factors from the order
    xMULv2(&P, &P, p_cofactor_for_3g, P_COFACTOR_FOR_3G_BITLENGTH, &A24);

    // Check if point has order 3^g
    copy_point(&P3, &P);
    for (int i = 0; i < POWER_OF_3 - 1; i++)
      xTPL(&P3, &P3, &A3);
    if (ec_is_zero(&P3))
      continue;
    else
      break;
  }

  // Find Q: start from hint1 + 2 (mirroring Julia's hint_start + 2)
  // x.im is already at the value from step hint1; advance 2 more steps
  // so we begin Q search at iteration hint1 + 2
  fp_add(x.im, x.re, x.im); // step hint1 + 1
  fp_add(x.im, x.re, x.im); // step hint1 + 2
  uint16_t hint2 = 0;
  while (1) {
    hint2++;
    fp_add(x.im, x.re, x.im);

    // Check if point is rational
    fp2_mul(&t1, &C2, &x);
    fp2_add(&t1, &t1, &AC);
    fp2_mul(&t1, &t1, &x);
    fp2_add(&t1, &t1, &C2);
    fp2_mul(&t1, &t1, &x);
    if (!fp2_is_square(&t1))
      continue;

    fp2_copy(&Q.x, &x);
    fp_mont_setone(Q.z.re);
    fp_set(Q.z.im, 0);

    // Clear non-3 factors from the order
    xMULv2(&Q, &Q, p_cofactor_for_3g, P_COFACTOR_FOR_3G_BITLENGTH, &A24);

    // Check if point has order 3^g
    copy_point(&Q3, &Q);
    for (int i = 0; i < POWER_OF_3 - 1; i++)
      xTPL(&Q3, &Q3, &A3);
    if (ec_is_zero(&Q3))
      continue;

    // Check if point is orthogonal to P
    if (is_point_equal(&P3, &Q3))
      continue;
    else
      break;
  }

  // Normalize points
  ec_curve_t E;
  fp2_mul(&t0, &P.z, &Q.z);
  fp2_mul(&t1, &t0, &curve->C);
  fp2_inv(&t1);
  fp2_mul(&P.x, &P.x, &t1);
  fp2_mul(&Q.x, &Q.x, &t1);
  fp2_mul(&E.A, &curve->A, &t1);
  fp2_mul(&P.x, &P.x, &Q.z);
  fp2_mul(&P.x, &P.x, &curve->C);
  fp2_mul(&Q.x, &Q.x, &P.z);
  fp2_mul(&Q.x, &Q.x, &curve->C);
  fp2_mul(&E.A, &E.A, &t0);
  fp_mont_setone(P.z.re);
  fp_set(P.z.im, 0);
  fp2_copy(&Q.z, &P.z);
  fp2_copy(&E.C, &P.z);
  AC_to_A24(&E.A24, &E);

  // Compute P-Q
  difference_point(&PQ3->PmQ, &P, &Q, &E);
  copy_point(&PQ3->P, &P);
  copy_point(&PQ3->Q, &Q);

  // hint1: iterations to find P (fits in 8 bits)
  // hint2: extra iterations beyond hint1+2 to find Q (fits in 8 bits)
  assert(hint1 < 256);
  assert(hint2 < 256);

  return (uint16_t)((hint1 << 8) | hint2);
}

int ec_curve_to_basis_3f_from_hint(ec_basis_t *PQ3, const ec_curve_t *curve,
                                   const uint16_t hint) {

  fp2_t x, t0, t1, t2;
  ec_point_t P, Q, P3, Q3, A24, A3;

  // Decode: high byte = hint1 (iterations for P), low byte = hint2 (extra iters
  // for Q)
  uint16_t hint1 = hint >> 8;
  uint16_t hint2 = hint & 0xFF;

  // Curve coefficient in the form A24 = (A+2C:4C)
  if (fp2_is_zero(&curve->A24.z)) {
    A24_from_AC(&A24, curve);
  } else {
    copy_point(&A24, &curve->A24);
  }

  // Curve coefficient in the form A3 = (A+2C:A-2C)
  fp2_sub(&A3.z, &A24.x, &A24.z);
  fp2_copy(&A3.x, &A24.x);

  // Replay P: advance x.im by hint1 steps
  fp_mont_setone(x.re);
  fp_set(x.im, 0);
  for (uint16_t i = 0; i < hint1; i++)
    fp_add(x.im, x.re, x.im);

  fp2_sqr(&t0, &curve->C);
  fp2_mul(&t1, &t0, &x);
  fp2_mul(&t2, &curve->A, &curve->C);
  fp2_add(&t1, &t1, &t2);
  fp2_mul(&t1, &t1, &x);
  fp2_add(&t1, &t1, &t0);
  fp2_mul(&t1, &t1, &x);

#ifndef NDEBUG
  int passed = fp2_is_square(&t1);
  if (!passed)
    return 0;
#endif

  fp2_copy(&P.x, &x);
  fp_mont_setone(P.z.re);
  fp_set(P.z.im, 0);

  // Clear non-3 factors from the order
  xMULv2(&P, &P, p_cofactor_for_3g, P_COFACTOR_FOR_3G_BITLENGTH, &A24);

  // Replay Q: advance x.im by 2 more (the mandatory gap) + hint2 extra steps
  for (uint16_t i = 0; i < 2 + hint2; i++)
    fp_add(x.im, x.re, x.im);

  fp2_sqr(&t0, &curve->C);
  fp2_mul(&t1, &t0, &x);
  fp2_mul(&t2, &curve->A, &curve->C);
  fp2_add(&t1, &t1, &t2);
  fp2_mul(&t1, &t1, &x);
  fp2_add(&t1, &t1, &t0);
  fp2_mul(&t1, &t1, &x);

#ifndef NDEBUG
  passed &= fp2_is_square(&t1);
  if (!passed)
    return 0;
#endif

  fp2_copy(&Q.x, &x);
  fp_mont_setone(Q.z.re);
  fp_set(Q.z.im, 0);

  // Clear non-3 factors from the order
  xMULv2(&Q, &Q, p_cofactor_for_3g, P_COFACTOR_FOR_3G_BITLENGTH, &A24);

#ifndef NDEBUG
  // Check if points have order 3^g and are orthogonal
  copy_point(&P3, &P);
  for (int i = 0; i < POWER_OF_3 - 1; i++)
    xTPL(&P3, &P3, &A3);
  passed &= !ec_is_zero(&P3);

  copy_point(&Q3, &Q);
  for (int i = 0; i < POWER_OF_3 - 1; i++)
    xTPL(&Q3, &Q3, &A3);
  passed &= !ec_is_zero(&Q3);
  passed &= !is_point_equal(&P3, &Q3);
  xDBLv2(&P3, &P3, &A24);
  passed &= !is_point_equal(&P3, &Q3);

  if (!passed)
    return 0;
#endif

  // Normalize points
  ec_curve_t E;
  fp2_mul(&t0, &P.z, &Q.z);
  fp2_mul(&t1, &t0, &curve->C);
  fp2_inv(&t1);
  fp2_mul(&P.x, &P.x, &t1);
  fp2_mul(&Q.x, &Q.x, &t1);
  fp2_mul(&E.A, &curve->A, &t1);
  fp2_mul(&P.x, &P.x, &Q.z);
  fp2_mul(&P.x, &P.x, &curve->C);
  fp2_mul(&Q.x, &Q.x, &P.z);
  fp2_mul(&Q.x, &Q.x, &curve->C);
  fp2_mul(&E.A, &E.A, &t0);
  fp_mont_setone(P.z.re);
  fp_set(P.z.im, 0);
  fp2_copy(&Q.z, &P.z);
  fp2_copy(&E.C, &P.z);
  AC_to_A24(&E.A24, &E);

  // Compute P-Q
  difference_point(&PQ3->PmQ, &P, &Q, &E);
  copy_point(&PQ3->P, &P);
  copy_point(&PQ3->Q, &Q);

  return 1;
}

void ec_curve_to_basis_6(ec_basis_t *PQ6, const ec_curve_t *curve) {

  fp2_t x, t0, t1, t2;
  ec_point_t P, Q, Q6, P6, R, T, A24, A3;

  // Curve coefficient in the form A24 = (A+2C:4C)
  if (fp2_is_zero(&curve->A24.z)) {
    A24_from_AC(&A24, curve);
  } else {
    copy_point(&A24, &curve->A24);
  }

  // Curve coefficient in the form A3 = (A+2C:A-2C)
  fp2_sub(&A3.z, &A24.x, &A24.z);
  fp2_copy(&A3.x, &A24.x);

  fp_mont_setone(x.re);
  fp_set(x.im, 0);

  fp2_t C2, AC;
  fp2_sqr(&C2, &curve->C);
  fp2_mul(&AC, &curve->A, &curve->C);

  // Find P
  while (1) {
    fp_add(x.im, x.re, x.im);

    // Check if point is rational
    fp2_mul(&t1, &C2, &x);
    fp2_add(&t1, &t1, &AC);
    fp2_mul(&t1, &t1, &x);
    fp2_add(&t1, &t1, &C2);
    fp2_mul(&t1, &t1, &x);
    if (fp2_is_square(&t1)) {
      fp2_copy(&P.x, &x);
      fp_mont_setone(P.z.re);
      fp_set(P.z.im, 0);
    } else
      continue;

    // Clear non-2 factors and non-3 factors from the order
    xMULv2(&P, &P, p_cofactor_for_6fg, P_COFACTOR_FOR_6FG_BITLENGTH, &A24);

    // Check if point has order 2^f*3^g
    copy_point(&P6, &P);
    for (int i = 0; i < POWER_OF_2 - 1; i++)
      xDBLv2(&P6, &P6, &A24);
    for (int i = 0; i < POWER_OF_3 - 1; i++)
      xTPL(&P6, &P6, &A3);
    if (ec_is_zero(&P6))
      continue;
    xDBLv2(&T, &P6, &A24);
    if (ec_is_zero(&T))
      continue;
    xTPL(&T, &P6, &A3);
    if (ec_is_zero(&T))
      continue;
    break;
  }

  // Find Q
  while (1) {
    fp_add(x.im, x.re, x.im);

    // Check if point is rational
    fp2_mul(&t1, &C2, &x);
    fp2_add(&t1, &t1, &AC);
    fp2_mul(&t1, &t1, &x);
    fp2_add(&t1, &t1, &C2);
    fp2_mul(&t1, &t1, &x);
    if (fp2_is_square(&t1)) {
      fp2_copy(&Q.x, &x);
      fp_mont_setone(Q.z.re);
      fp_set(Q.z.im, 0);
    } else
      continue;

    // Clear non-6 factors from the order
    xMULv2(&Q, &Q, p_cofactor_for_6fg, P_COFACTOR_FOR_6FG_BITLENGTH, &A24);

    // Check first if point has order 2^f*3^g
    copy_point(&Q6, &Q);
    for (int i = 0; i < POWER_OF_2 - 1; i++)
      xDBLv2(&Q6, &Q6, &A24);
    for (int i = 0; i < POWER_OF_3 - 1; i++)
      xTPL(&Q6, &Q6, &A3);
    if (ec_is_zero(&Q6))
      continue;
    xDBLv2(&T, &Q6, &A24);
    if (ec_is_zero(&T))
      continue;
    xTPL(&T, &Q6, &A3);
    if (ec_is_zero(&T))
      continue;

    // Check if point P is independent from point Q
    xTPL(&R, &P6, &A3);
    xTPL(&T, &Q6, &A3);
    if (is_point_equal(&R, &T))
      continue;
    xDBLv2(&R, &P6, &A24);
    xDBLv2(&T, &Q6, &A24);
    if (is_point_equal(&R, &T))
      continue;
    break;
  }

  // Normalize points
  ec_curve_t E;
  fp2_mul(&t0, &P.z, &Q.z);
  fp2_mul(&t1, &t0, &curve->C);
  fp2_inv(&t1);
  fp2_mul(&P.x, &P.x, &t1);
  fp2_mul(&Q.x, &Q.x, &t1);
  fp2_mul(&E.A, &curve->A, &t1);
  fp2_mul(&P.x, &P.x, &Q.z);
  fp2_mul(&P.x, &P.x, &curve->C);
  fp2_mul(&Q.x, &Q.x, &P.z);
  fp2_mul(&Q.x, &Q.x, &curve->C);
  fp2_mul(&E.A, &E.A, &t0);
  fp_mont_setone(P.z.re);
  fp_set(P.z.im, 0);
  fp2_copy(&Q.z, &P.z);
  fp2_copy(&E.C, &P.z);
  AC_to_A24(&E.A24, &E);

  // Compute P-Q
  difference_point(&PQ6->PmQ, &P, &Q, &E);
  copy_point(&PQ6->P, &P);
  copy_point(&PQ6->Q, &Q);
}
