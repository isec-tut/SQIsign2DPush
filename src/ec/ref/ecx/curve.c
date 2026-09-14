#include "ec.h"

#include <fp2.h>

/*
 * Curve-state helpers shared by the EC implementation and the 2DPush
 * protocols.  These functions used to live in keygen.c, which made the
 * protocol layer own part of the Montgomery curve representation.
 */

void ec_normalize_curve_and_A24(ec_curve_t *E) {
  /* First normalize (A:C) to (A/C:1). */
  if (!fp2_is_one(&E->C)) {
    ec_normalize_curve(E);
  }

  /* Then compute the normalized A24 = ((A + 2) / 4 : 1). */
  ec_curve_normalize_A24(E);
}

void xDBL_A24(ec_point_t *Q, const ec_point_t *P,
              const ec_point_t *A24) {
  /*
   * Differential doubling with A24 = (A+2C : 4C), or its normalized
   * equivalent ((A+2C)/(4C) : 1).
   */
  fp2_t t0, t1, t2;

  fp2_add(&t0, &P->x, &P->z);
  fp2_sqr(&t0, &t0);
  fp2_sub(&t1, &P->x, &P->z);
  fp2_sqr(&t1, &t1);
  fp2_sub(&t2, &t0, &t1);
  fp2_mul(&Q->x, &t0, &t1);
  fp2_mul(&t0, &t2, &A24->x);
  fp2_add(&t0, &t0, &t1);
  fp2_mul(&Q->z, &t0, &t2);
}

void xDBL_E0(ec_point_t *Q, const ec_point_t *P) {
  /* Differential doubling on E0: (A:C) = (0:1). */
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
