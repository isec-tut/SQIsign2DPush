/** @file
 *
 * @authors Luca De Feo, Francisco RH
 *
 * @brief Elliptic curve stuff
 */

#ifndef EC_H
#define EC_H

#include <ec_params.h>
#include <fp2.h>
#include <stdbool.h>

/** @defgroup ec Elliptic curves
 * @{
 */

/** @defgroup ec_t Data structures
 * @{
 */

/** @brief Projective point
 *
 * @typedef ec_point_t
 *
 * @struct ec_point_t
 *
 * A projective point in (X:Z) or (X:Y:Z) coordinates (tbd).
 */
typedef struct ec_point_t {
  fp2_t x;
  fp2_t z;
} ec_point_t;

/** @brief Projective point in Montgomery coordinates
 *
 * @typedef jac_point_t
 *
 * @struct jac_point_t
 *
 * A projective point in (X:Y:Z) coordinates
 */
typedef struct jac_point_t {
  fp2_t x;
  fp2_t y;
  fp2_t z;
} jac_point_t;

/** @brief Addition components
 *
 * @typedef add_components_t
 *
 * @struct add_components_t
 *
 * 3 components u,v,w that define the (X:Z) coordinates of both
 * addition and substraction of two distinct points with
 * P+Q =(u-v:w) and P-Q = (u+v=w)
 */
typedef struct add_components_t {
  fp2_t u;
  fp2_t v;
  fp2_t w;
} add_components_t;

/** @brief A basis of a torsion subgroup
 *
 * @typedef ec_basis_t
 *
 * @struct ec_basis_t
 *
 * A pair of points (or a triplet, tbd) forming a basis of a torsion subgroup.
 */
typedef struct ec_basis_t {
  ec_point_t P;
  ec_point_t Q;
  ec_point_t PmQ;
} ec_basis_t;

/** @brief An elliptic curve
 *
 * @typedef ec_curve_t
 *
 * @struct ec_curve_t
 *
 * An elliptic curve in projective Montgomery form
 */
typedef struct ec_curve_t {
  fp2_t A;
  fp2_t C; ///< cannot be 0
  ec_point_t A24;
  /* True when A24 is the normalized (A+2C : 4C) value. */
  bool is_A24_computed_and_normalized;
} ec_curve_t;

/* A24 = (A + 2C : 4C).  Keep this conversion in the public EC interface so
 * the different EC modules use one definition, as in v2.0. */
static inline void AC_to_A24(ec_point_t *A24, const ec_curve_t *E) {
  fp2_add(&A24->z, &E->C, &E->C);
  fp2_add(&A24->x, &E->A, &A24->z);
  fp2_add(&A24->z, &A24->z, &A24->z);
}

/**
 * @brief Initialize a curve structure
 */
void ec_curve_init(ec_curve_t *curve);
int ec_curve_init_from_A(ec_curve_t *curve, const fp2_t *A);
void ec_point_init(ec_point_t *P);
void ec_curve_normalize_A24(ec_curve_t *curve);
void ec_normalize_curve_and_A24(ec_curve_t *curve);
void xDBL_A24(ec_point_t *Q, const ec_point_t *P, const ec_point_t *A24);
void xDBL_E0(ec_point_t *Q, const ec_point_t *P);

/** @brief An isogeny of degree a power of 2
 *
 * @typedef ec_isog_even_t
 *
 * @struct ec_isog_even_t
 */
typedef struct ec_isog_even_t {
  ec_curve_t curve;      ///< The domain curve
  ec_point_t kernel;     ///< A kernel generator
  unsigned length; ///< The length as a 2-isogeny walk
} ec_isog_even_t;

/** @brief An odd divisor of p² - 1
 *
 * @typedef ec_isog_odd_t
 *
 * Given that the list of divisors of p² - 1 is known, this is
 * represented as a fixed-length vector of integer exponents.
 */

typedef uint8_t ec_degree_odd_t[P_LEN + M_LEN];

/** @brief An isogeny of odd degree dividing p² - 1
 *
 * @typedef ec_isog_odd_t
 *
 * @struct ec_isog_odd_t
 */
typedef struct ec_isog_odd_t {
  ec_curve_t curve;
  ec_point_t ker_plus;    ///< A generator of E[p+1] ∩ ker(φ)
  ec_point_t ker_minus;   ///< A generator of E[p-1] ∩ ker(φ)
  ec_degree_odd_t degree; ///< The degree of the isogeny
} ec_isog_odd_t;

/** @brief Isomorphism of Montgomery curves
 *
 * @typedef ec_isom_t
 *
 * @struct ec_isom_t
 *
 * The isomorphism is given by the map maps (X:Z) ↦ ( (Nx X - Nz Z) : (D Z) )
 */
typedef struct ec_isom_t {
  fp2_t Nx;
  fp2_t Nz;
  fp2_t D;
} ec_isom_t;

typedef struct pairing_dlog_diff_points {
  ec_point_t PmR;
  ec_point_t PmS;
  ec_point_t RmQ;
  ec_point_t SmQ;
} pairing_dlog_diff_points_t;

typedef struct pairing_dlog_params {
  uint32_t e;                      // Points have order 2^e
  ec_basis_t PQ;                   // x(P), x(Q), x(P-Q)
  ec_basis_t RS;                   // x(R), x(S), x(R-S)
  pairing_dlog_diff_points_t diff; // x(P - R), x(P - S), x(R - Q), x(S - Q)
  fp2_t ixP;                       // PZ/PX
  fp2_t ixQ;                       // QZ/QX
  fp2_t ixR;                       // RZ/RX
  fp2_t ixS;                       // SZ/SX
  ec_point_t A24;                  // ((A+2)/4 : 1)
} pairing_dlog_params_t;

typedef struct pairing_dlog_params_3 {
  uint32_t e;                      // Points have order 3^e
  ec_basis_t PQ;                   // x(P), x(Q), x(P-Q)
  ec_basis_t RS;                   // x(R), x(S), x(R-S)
  pairing_dlog_diff_points_t diff; // x(P - R), x(P - S), x(R - Q), x(S - Q)
  fp2_t ixP;                       // PZ/PX
  fp2_t ixQ;                       // QZ/QX
  fp2_t ixR;                       // RZ/RX
  fp2_t ixS;                       // SZ/SX
  ec_point_t A3;                   // A3 = A+2C : A-2C
} pairing_dlog_params_3_t;

int ec_curve_verify_A(const fp2_t *A);

// end ec_t
/** @}
 */

/** @defgroup ec_curve_t Curves and isomorphisms
 * @{
 */

/**
 * @brief j-invariant.
 *
 * @param j_inv computed j_invariant
 * @param curve input curve
 */
void ec_j_inv(fp2_t *j_inv, const ec_curve_t *curve);

/**
 * @brief Isomorphism of elliptic curve
 *
 * @param isom computed isomorphism
 * @param from domain curve
 * @param to image curve
 */
void ec_isomorphism(ec_isom_t *isom, const ec_curve_t *from,
                    const ec_curve_t *to);

/**
 * @brief In-place inversion of an isomorphism
 *
 * @param isom an isomorphism
 */
void ec_iso_inv(ec_isom_t *isom);

/**
 * @brief In-place evaluation of an isomorphism
 *
 * @param P a point
 * @param isom an isomorphism
 */
void ec_iso_eval(ec_point_t *P, ec_isom_t *isom);

/**
 * @brief Given a Montgomery curve, computes a standard model for it and the
 * isomorphism to it.
 *
 * @param new computed new curve
 * @param isom computed isomorphism from `old` to `new`
 * @param old A Montgomery curve
 */
void ec_curve_normalize(ec_curve_t *new, ec_isom_t *isom,
                        const ec_curve_t *old);

/** @}
 */
/** @defgroup ec_point_t Point operations
 * @{
 */

/**
 * @brief Point equality
 *
 * @param P a point
 * @param Q a point
 * @return 1 if equal
 */
bool ec_is_equal(const ec_point_t *P, const ec_point_t *Q);

/**
 * @brief Reduce Z-coordinate of point in place
 *
 * @param P a point
 */
void ec_normalize(ec_point_t *P);

void ec_normalize_curve(ec_curve_t *E);
void ec_normalize_point(ec_point_t *P);

void difference_point(ec_point_t *PQ, const ec_point_t *P, const ec_point_t *Q,
                      const ec_curve_t *curve);

/**
 * @brief Test whether a point is on a curve
 *
 * @param curve a curve
 * @param P a point
 * @return 1 if P is on the curve
 */
int ec_is_on_curve(const ec_curve_t *curve, const ec_point_t *P);

/**
 * @brief Point negation
 *
 * @param res computed opposite of P
 * @param P a point
 */
void ec_neg(ec_point_t *res, const ec_point_t *P);

/**
 * @brief Point addition
 *
 * @param res computed sum of P and Q
 * @param P a point
 * @param Q a point
 * @param PQ the difference P-Q
 */
void ec_add(ec_point_t *res, const ec_point_t *P, const ec_point_t *Q,
            const ec_point_t *PQ);

/**
 * @brief Point doubling
 *
 * @param res computed double of P
 * @param P a point
 */
void ec_dbl(ec_point_t *res, const ec_curve_t *curve, const ec_point_t *P);

/**
 * @brief Point multiplication
 *
 * @param res computed scalar * P
 * @param curve the curve
 * @param scalar an unsigned multi-precision integer
 * @param P a point
 */
void ec_mul(ec_point_t *res, const ec_curve_t *curve, const digit_t *scalar,
            const ec_point_t *P);

/**
 * @brief Check if Jacobian point has order 2^f
 */
int test_jac_order_twof(const jac_point_t *P, const ec_curve_t *curve, int f);

/**
 * @brief Check if projective point has order 2^f
 */
int test_point_order_twof(const ec_point_t *P, const ec_curve_t *curve, int f);

/**
 * @brief Lifts a basis from Montgomery XZ to Jacobian coordinates
 */
int lift_basis(jac_point_t *P, jac_point_t *Q, const ec_basis_t *B,
               ec_curve_t *E);

/**
 * @brief Compute add components u,v,w equivalent for Jacobian points P and Q
 */
void jac_to_xz_add_components(add_components_t *dest, const jac_point_t *P,
                              const jac_point_t *Q, const ec_curve_t *curve);

/**
 * @brief Point multiplication by a scalar of limited length
 *
 * @param res computed scalar * P
 * @param curve the curve
 * @param scalar an unsigned multi-precision integer
 * @param kbits the bit size of scalar
 * @param P a point
 */
void xMULv2(ec_point_t *Q, ec_point_t const *P, digit_t const *k,
            const int kbits, ec_point_t const *A24);

/**
 * @brief Combination P+m*Q
 *
 * @param R computed P + m * Q
 * @param curve the curve
 * @param m an unsigned multi-precision integer
 * @param P a point
 * @param Q a point
 * @param PQ the difference P-Q
 */
void ec_ladder3pt(ec_point_t *R, fp_t const m, ec_point_t const *P,
                  ec_point_t const *Q, ec_point_t const *PQ,
                  ec_curve_t const *A);

/**
 * @brief Linear combination of points of a basis
 *
 * @param res computed scalarP * P + scalarQ * Q
 * @param curve the curve
 * @param scalarP an unsigned multi-precision integer
 * @param scalarQ an unsigned multi-precision integer
 * @param PQ a torsion basis consisting of points P and Q
 */
void ec_biscalar_mul(ec_point_t *res, const ec_curve_t *curve,
                     const digit_t *scalarP, const digit_t *scalarQ,
                     const ec_basis_t *PQ);

/** @}
 */

/** @defgroup ec_dlog_t Discrete logs and bases
 * @{
 */

/**
 * @brief Generate a Montgomery curve and a 2^f-torsion basis
 *
 * The algorithm is deterministc
 *
 * @param PQ2 computed basis of the 2^f-torsion
 * @param curve the computed curve
 */
void ec_curve_to_basis_2(ec_basis_t *PQ2, const ec_curve_t *curve);

/**
 * @brief Complete a basis of the 2^f-torsion
 *
 * The algorithm is deterministic
 *
 * @param PQ2 a basis of the 2^f-torsion containing P as first generator
 * @param curve the curve
 * @param P a point of order 2^f
 */
void ec_complete_basis_2(ec_basis_t *PQ2, const ec_curve_t *curve,
                         const ec_point_t *P);

/**
 * @brief Generate a Montgomery curve and a 2^f-torsion basis
 *
 * The algorithm is deterministic
 *
 * @param PQ2 computed basis of the 2^f-torsion
 * @param curve the computed curve
 * @param f the exponent of 2
 * @return the hint for the basis computation
 */
uint8_t ec_curve_to_basis_2f_to_hint(ec_basis_t *PQ2, ec_curve_t *curve, int f);

/**
 * @brief Generate a Montgomery curve and a 2^f-torsion basis from a hint
 *
 * The algorithm is deterministic
 *
 * @param PQ2 computed basis of the 2^f-torsion
 * @param curve the computed curve
 * @param f the exponent of 2
 * @param hint the hint for the basis computation
 */
int ec_curve_to_basis_2f_from_hint(ec_basis_t *PQ2, ec_curve_t *curve, int f,
                                   const uint8_t hint);

/**
 * @brief Generate a random point of order 3
 *
 * The algorithm is deterministic
 *
 * @param P the computed point of order 3
 * @param curve a curve
 */
void ec_random_point_order_3(ec_point_t *P, const ec_curve_t *curve);

/**
 * @brief Generate a 3^e-torsion basis
 *
 * The algorithm is deterministic
 *
 * @param PQ3 the computed 3^e-torsion basis
 * @param curve a curve
 */
void ec_curve_to_basis_3(ec_basis_t *PQ3, const ec_curve_t *curve);

/**
 * @brief Generate a 6^e-torsion basis
 *
 * The algorithm is deterministic
 *
 * @param PQ6 the computed 2^f*3^g-torsion basis
 * @param curve a curve
 */
void ec_curve_to_basis_6(ec_basis_t *PQ6, const ec_curve_t *curve);

/**
 * @brief Generate a Montgomery curve and a 3^e-torsion basis
 *
 * The algorithm is deterministic
 *
 * @param PQ3 computed basis of the 3^e-torsion
 * @param curve the computed curve
 * @param e the exponent of 3
 * @return the hint for the basis computation
 */
uint16_t ec_curve_to_basis_3f_to_hint(ec_basis_t *PQ3, const ec_curve_t *curve);

/**
 * @brief Generate a Montgomery curve and a 3^e-torsion basis from a hint
 *
 * The algorithm is deterministic
 *
 * @param PQ3 computed basis of the 3^e-torsion
 * @param curve the computed curve
 * @param hint the hint for the basis computation
 * @return 1 on success, 0 on failure
 */
int ec_curve_to_basis_3f_from_hint(ec_basis_t *PQ3, const ec_curve_t *curve,
                                   const uint16_t hint);

/**
 * @brief Compute the generalized dlog of R wrt the 2^f-basis PQ2
 *
 * Ensure that R = scalarP * P + scalarQ * Q
 *
 * @param scalarP the computed dlog
 * @param scalarQ the computed dlog
 * @param PQ2 a 2^f-torsion basis
 * @param R a point of order dividing 2^f
 */
void ec_dlog_2(digit_t *scalarP, digit_t *scalarQ, const ec_basis_t *PQ2,
               const ec_point_t *R, const ec_curve_t *curve);

/**
 * @brief Compute the generalized dlog of R wrt the 3^e-basis PQ3
 *
 * Ensure that R = scalarP * P + scalarQ * Q
 *
 * @param scalarP the computed dlog
 * @param scalarQ the computed dlog
 * @param PQ3 a 3^e-torsion basis
 * @param R a point of order dividing 3^e
 */
void ec_dlog_3(digit_t *scalarP, digit_t *scalarQ, const ec_basis_t *PQ3,
               const ec_point_t *R, const ec_curve_t *curve);
/** @}
 */

/** @defgroup ec_isog_t Isogenies
 * @{
 */

/**
 * @brief Evaluate isogeny of even degree on list of points
 *
 * @param image computed image curve
 * @param phi isogeny
 * @param points a list of points to evaluate the isogeny on, modified in place
 * @param length of the list points
 */
void ec_eval_even(ec_curve_t *image, const ec_isog_even_t *phi,
                  ec_point_t *points, unsigned short length);

/**
 * @brief Evaluate isogeny of even degree on list of points, assuming the point
 * (0,0) is not in the kernel
 *
 * @param image computed image curve
 * @param phi isogeny
 * @param points a list of points to evaluate the isogeny on, modified in place
 * @param length of the list points
 */
void ec_eval_even_nonzero(ec_curve_t *image, const ec_isog_even_t *phi,
                          ec_point_t *points, unsigned short length);

/**
 * @brief Evaluate isogeny of even degree on list of torsion bases
 *
 * @param image computed image curve
 * @param phi isogeny
 * @param points a list of bases to evaluate the isogeny on, modified in place
 * @param length of the list bases
 */
static inline void ec_eval_even_basis(ec_curve_t *image,
                                      const ec_isog_even_t *phi,
                                      ec_basis_t *points,
                                      unsigned short length) {
  ec_eval_even(image, phi, (ec_point_t *)points,
               sizeof(ec_basis_t) / sizeof(ec_point_t) * length);
}

/**
 * @brief Evaluate isogeny of odd degree on list of points
 *
 * @param image computed image curve
 * @param phi isogeny
 * @param points a list of points to evaluate the isogeny on, modified in place
 * @param length of the list points
 */
void ec_eval_odd(ec_curve_t *image, const ec_isog_odd_t *phi,
                 ec_point_t *points, unsigned short length);

void ec_eval_three(ec_curve_t *image, const ec_isog_odd_t *phi,
                   ec_point_t *points, unsigned short length);

void ec_eval_three_prev(ec_curve_t *image, ec_curve_t *image_prev,
                        const ec_isog_odd_t *phi);

/**
 * @brief Evaluate isogeny of odd degree on list of torsion bases
 *
 * @param image computed image curve
 * @param phi isogeny
 * @param points a list of bases to evaluate the isogeny on, modified in place
 * @param length of the list bases
 */
static inline void ec_eval_odd_basis(ec_curve_t *image,
                                     const ec_isog_odd_t *phi,
                                     ec_basis_t *points,
                                     unsigned short length) {
  ec_eval_odd(image, phi, (ec_point_t *)points,
              sizeof(ec_basis_t) / sizeof(ec_point_t) * length);
}

void A24_from_AC(ec_point_t *A24, ec_curve_t const *E);

void weil(fp2_t *r, uint32_t e, ec_point_t *P, ec_point_t *Q, ec_point_t *PQ,
          ec_curve_t *E);

// Computes (reduced) z = t_{2^e}(P, Q) using biextension ladder
void reduced_tate(fp2_t *r, uint32_t e, const ec_point_t *P,
                  const ec_point_t *Q, const ec_point_t *PQ, ec_curve_t *E);

// Given two bases <P, Q> and <R, S> computes scalars
// such that R = [r1]P + [r2]Q, S = [s1]P + [s2]Q
void ec_dlog_2_weil(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                    const ec_basis_t *PQ, const ec_basis_t *RS,
                    ec_curve_t *curve, int e);

// Given a basis <P, Q> and a point R, computes scalars r1, r2
// such that R = [r1]P + [r2]Q using the Weil pairing.
// PQ2 must be a 2^e-torsion basis.  R must have order dividing 2^e.
void ec_dlog_2_weil_R(digit_t *scalarP, digit_t *scalarQ, const ec_basis_t *PQ2,
                      const ec_point_t *R, ec_curve_t *curve, int e);

// Given a FULL E[2^f]-torsion basis <P, Q> and a point R of order dividing 2^e,
// computes scalars r1, r2 such that R = [r1]P + [r2]Q using the Tate pairing.
void ec_dlog_2_tate_R(digit_t *scalarP, digit_t *scalarQ, const ec_basis_t *PQ2,
                      const ec_point_t *R, ec_curve_t *curve, int e);

// Given a 3^f-torsion basis <P, Q> and two points R, S, computes
// r1, r2, s1, s2 such that R = [r1]P + [r2]Q, S = [s1]P + [s2]Q
// using the Weil pairing.  f is the exponent (i.e. torsion order = 3^f).
// Given two bases <P, Q> and <R, S> computes scalars
// such that R = [r1]P + [r2]Q, S = [s1]P + [s2]Q
void ec_dlog_3_(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                const ec_basis_t *PQ, const ec_basis_t *RS,
                const ec_curve_t *curve, int e);

// Given a basis <P, Q> and a point R, computes scalars r1, r2
// such that R = [r1]P + [r2]Q using the Weil pairing.
// PQ3 must be a 3^f-torsion basis. R must have order dividing 3^f.
void ec_dlog_3_weil_R(digit_t *r1, digit_t *r2, const ec_basis_t *PQ3,
                      const ec_point_t *R, const ec_curve_t *curve, int f);

// Given two bases <P, Q> and <R, S>
// where <P, Q> is a basis for E[2^f]
// the full 2-torsion, and <R,S> a basis
// for smaller torsion E[2^e]
// computes scalars r1, r2, s1, s2
// such that R = [r1]P + [r2]Q, S = [s1]P + [s2]Q
void ec_dlog_2_tate(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                    const ec_basis_t *PQ, const ec_basis_t *RS,
                    const ec_curve_t *curve, int e);

void ec_dlog_2_tate_to_full(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                            ec_basis_t *PQ, ec_basis_t *RS, ec_curve_t *curve,
                            int e);

void ec_dlog_3_tate(digit_t *r1, digit_t *r2, digit_t *s1, digit_t *s2,
                    const ec_basis_t *PQ, const ec_basis_t *RS,
                    const ec_curve_t *curve, int e);

void ec_dlog_3_tate_R(digit_t *r1, digit_t *r2, const ec_basis_t *PQ3,
                      const ec_point_t *R, const ec_curve_t *curve, int f);

#endif
