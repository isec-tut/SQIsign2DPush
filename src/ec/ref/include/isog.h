#ifndef _ISOG_H_
#define _ISOG_H_

#include "curve_extras.h"
#include "poly.h"
#include <stdint.h>

// typedef struct {
//   uint64_t add;
//   uint64_t mul;
//   uint64_t inv;
//   uint64_t sqr;
//   uint64_t sqrt;
// } field_ops_t;

// typedef struct {
//   uint64_t dbl_count;
//   uint64_t tpl_count;
//   uint64_t isog2_count;
//   uint64_t isog2_sing_count;
//   uint64_t isog4_count;
//   uint64_t isog4_sing_count;
//   uint64_t isog3_count;
//   uint64_t eval2_ker_count;
//   uint64_t eval2_pts_count;
//   uint64_t eval2_sing_ker_count;
//   uint64_t eval2_sing_pts_count;
//   uint64_t eval4_ker_count;
//   uint64_t eval4_pts_count;
//   uint64_t eval4_sing_ker_count;
//   uint64_t eval4_sing_pts_count;
//   uint64_t eval3_count;
//   field_ops_t dbl_ops;
//   field_ops_t tpl_ops;
//   field_ops_t isog2_ops;
//   field_ops_t isog2_sing_ops;
//   field_ops_t isog4_ops;
//   field_ops_t isog4_sing_ops;
//   field_ops_t isog3_ops;
//   field_ops_t eval2_ops;
//   field_ops_t eval2_sing_ops;
//   field_ops_t eval4_ops;
//   field_ops_t eval4_sing_ops;
//   field_ops_t eval3_ops;
// } isog_counters_t;

// extern isog_counters_t global_isog_counters;

// extern uint64_t fp2_add_count;
// extern uint64_t fp2_mul_count;
// extern uint64_t fp2_inv_count;
// extern uint64_t fp2_sqr_count;
// extern uint64_t fp2_sqrt_count;

// #define ISOG_MEASURE_START(start)                                              \
  // field_ops_t start;                                                        \
  // start.add = fp2_add_count;                                                \
  // start.mul = fp2_mul_count;                                                \
  // start.inv = fp2_inv_count;                                                \
  // start.sqr = fp2_sqr_count;                                                \
  // start.sqrt = fp2_sqrt_count;

// #define ISOG_MEASURE_END(dest_ptr, start)                                      \
  // (dest_ptr)->add += fp2_add_count - (start).add;                           \
  // (dest_ptr)->mul += fp2_mul_count - (start).mul;                           \
  // (dest_ptr)->inv += fp2_inv_count - (start).inv;                           \
  // (dest_ptr)->sqr += fp2_sqr_count - (start).sqr;                           \
  // (dest_ptr)->sqrt += fp2_sqrt_count - (start).sqrt;

// static inline void isog_measure_reset() {
  // global_isog_counters.dbl_count = 0;
  // global_isog_counters.tpl_count = 0;
  // global_isog_counters.isog2_count = 0;
  // global_isog_counters.isog2_sing_count = 0;
  // global_isog_counters.isog4_count = 0;
  // global_isog_counters.isog4_sing_count = 0;
  // global_isog_counters.isog3_count = 0;
  // global_isog_counters.eval2_ker_count = 0;
  // global_isog_counters.eval2_pts_count = 0;
  // global_isog_counters.eval2_sing_ker_count = 0;
  // global_isog_counters.eval2_sing_pts_count = 0;
  // global_isog_counters.eval4_ker_count = 0;
  // global_isog_counters.eval4_pts_count = 0;
  // global_isog_counters.eval4_sing_ker_count = 0;
  // global_isog_counters.eval4_sing_pts_count = 0;
  // // global_isog_counters.eval3_count = 0;
  // memset(&global_isog_counters.dbl_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.tpl_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.isog2_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.isog2_sing_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.isog4_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.isog4_sing_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.isog3_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.eval2_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.eval2_sing_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.eval4_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.eval4_sing_ops, 0, sizeof(field_ops_t));
  // memset(&global_isog_counters.eval3_ops, 0, sizeof(field_ops_t));
  //   fp2_add_count = 0;
  //   fp2_mul_count = 0;
  //   fp2_inv_count = 0;
  //   fp2_sqr_count = 0;
  //   fp2_sqrt_count = 0;
// }

extern int sI, sJ, sK; // Sizes of each current I, J, and K

extern fp2_t
    I[sI_max][2], // I plays also as the linear factors of the polynomial h_I(X)
    EJ_0[sJ_max][3], EJ_1[sJ_max][3]; // To be used in xisog y xeval

extern ec_point_t J[sJ_max], K[sK_max]; // Finite subsets of the kernel
extern fp2_t
    XZJ4[sJ_max], // -4* (Xj * Zj) for each j in J, and x([j]P) = (Xj : Zj)
    rtree_A[(1 << (ceil_log_sI_max + 2)) -
            1], // constant multiple of the reciprocal tree computation
    A0;         // constant multiple of the reciprocal R0

extern poly
    ptree_hI[(1 << (ceil_log_sI_max + 2)) - 1], // product tree of h_I(X)
    rtree_hI[(1 << (ceil_log_sI_max + 2)) - 1], // reciprocal tree of h_I(X)
    ptree_EJ[(1 << (ceil_log_sJ_max + 2)) - 1]; // product tree of E_J(X)

extern fp2_t R0[2 * sJ_max + 1]; // Reciprocal of h_I(X) required in the scaled
                                 // remainder tree approach

extern int deg_ptree_hI[(1 << (ceil_log_sI_max + 2)) -
                        1], // degree of each noed in the product tree of h_I(X)
    deg_ptree_EJ[(1 << (ceil_log_sJ_max + 2)) -
                 1]; // degree of each node in the product tree of E_J(X)

extern fp2_t leaves[sI_max]; // leaves of the remainder tree, which are required
                             // in the Resultant computation

void eds2mont(ec_point_t *P); // mapping from Twisted edwards into Montogmery
void yadd(
    ec_point_t *R, ec_point_t *const P, ec_point_t *const Q,
    ec_point_t *const PQ); // differential addition on Twisted edwards model
void CrissCross(fp2_t *r0, fp2_t *r1, fp2_t const alpha, fp2_t const beta,
                fp2_t const gamma, fp2_t const delta);

void kps_t(uint64_t const i, ec_point_t const P,
           ec_point_t const A); // tvelu formulae
void kps_s(uint64_t const i, ec_point_t const P,
           ec_point_t const A); // svelu formulae

void xisog_4(ec_point_t *B,
             ec_point_t const P); // degree-4 isogeny construction
void xisog_4_singular(ec_point_t *B24, ec_point_t const P, ec_point_t A24);
void xisog_2(ec_point_t *B,
             ec_point_t const P); // degree-2 isogeny construction
void xisog_2_singular(ec_point_t *B24, ec_point_t A24);
void xisog_3(ec_point_t *B24, ec_point_t const P);
void xisog_t(ec_point_t *B, uint64_t const i,
             ec_point_t const A); // tvelu formulae
void xisog_s(ec_point_t *B, uint64_t const i,
             ec_point_t const A); // svelu formulae

void xeval_4(ec_point_t *R, const ec_point_t *Q,
             const int lenQ); // degree-4 isogeny evaluation
void xeval_4_singular(ec_point_t *R, const ec_point_t *Q, const int lenQ,
                      const ec_point_t P);
void xeval_2(ec_point_t *R, ec_point_t *const Q,
             const int lenQ); // degree-2 isogeny evaluation
void xeval_2_singular(ec_point_t *R, const ec_point_t *Q, const int lenQ);
void xeval_3(ec_point_t *R, const ec_point_t *Q, const int lenQ);
void xeval_t(ec_point_t *Q, uint64_t const i,
             ec_point_t const P); // tvelu formulae
void xeval_s(ec_point_t *Q, uint64_t const i, ec_point_t const P,
             ec_point_t const A); // svelu formulae

// Strategy-based 4-isogeny chain
static void ec_eval_even_strategy(ec_curve_t *image, ec_point_t *points,
                                  unsigned short points_len, ec_point_t *A24,
                                  const ec_point_t *kernel, const int isog_len);

void kps_clear(int i); // Clear memory assigned by KPS

// hybrid velu formulae
static inline void kps(uint64_t const i, ec_point_t const P,
                       ec_point_t const A) {
  // Next branch only depends on a fixed public bound (named gap)
  if (TORSION_ODD_PRIMES[i] <= gap)
    kps_t(i, P, A);
  else
    kps_s(i, P, A);
}

static inline void xisog(ec_point_t *B, uint64_t const i, ec_point_t const A) {
  // if (i == 0)
  // global_isog_counters.isog3_count++;
  // Next branch only depends on a fixed public bound (named gap)
  if (TORSION_ODD_PRIMES[i] <= gap)
    xisog_t(B, i, A);
  else
    xisog_s(B, i, A);
}

static inline void xeval(ec_point_t *Q, uint64_t const i, ec_point_t const P,
                         ec_point_t const A) {
  // if (i == 0)
  // global_isog_counters.eval3_count++;
  // Next branch only depends on a fixed public bound (named gap)
  if (TORSION_ODD_PRIMES[i] <= gap)
    xeval_t(Q, i, P);
  else
    xeval_s(Q, i, P, A);
}

#endif
