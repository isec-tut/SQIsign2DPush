#include "isog.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// uint64_t measure_three_TPL = 0;
// uint64_t measure_three_ISOG = 0;
// uint64_t measure_three_EVAL_ker = 0;
// uint64_t measure_three_EVAL_pts = 0;

static inline void AC_to_A24(ec_point_t *A24, ec_curve_t const *E) {
  // A24 = (A+2C : 4C)
  fp2_add(&A24->z, &E->C, &E->C);
  fp2_add(&A24->x, &E->A, &A24->z);
  fp2_add(&A24->z, &A24->z, &A24->z);
}

static inline void AC_to_A24minus(ec_point_t *A24minus, ec_curve_t const *E) {
  // Input:  curve constant (A:C)
  // Output: A24minus = (A-2C : 4C)

  fp2_add(&A24minus->z, &E->C, &E->C);               // 2C
  fp2_sub(&A24minus->x, &E->A, &A24minus->z);        // A - 2C
  fp2_add(&A24minus->z, &A24minus->z, &A24minus->z); // 4C
}

static inline void A24_to_AC(ec_curve_t *E, ec_point_t const *A24) {
  // (A:C) = ((A+2C)*2-4C : 4C)
  fp2_add(&E->A, &A24->x, &A24->x);
  fp2_sub(&E->A, &E->A, &A24->z);
  fp2_add(&E->A, &E->A, &E->A);
  fp2_copy(&E->C, &A24->z);
}

static inline void A24minus_to_AC(ec_curve_t *E, ec_point_t const *A24minus) {
  // Input:  A24minus = (A-2C : 4C)
  // Output: (4A : 4C) ≡ (A : C)
  fp2_add(&E->A, &A24minus->x, &A24minus->x); // 2*(A-2C) = 2A-4C
  fp2_add(&E->A, &E->A, &A24minus->z);        // 2A-4C + 4C = 2A
  fp2_add(&E->A, &E->A, &E->A);               // 2A * 2 = 4A
  fp2_copy(&E->C, &A24minus->z);              // 4C
}

static inline void A24minus_to_A24plus(ec_point_t *A24plus,
                                       ec_point_t const *A24minus) {
  // (A+2C : 4C) = ((A-2C)+4C : 4C)
  fp2_add(&A24plus->x, &A24minus->x, &A24minus->z);
  fp2_copy(&A24plus->z, &A24minus->z);
}

// functions moved to end of file to avoid declaration issues

// Sequential (non-strategy) even isogeny chain for arbitrary lengths.
// Applies a chain of 2- and 4-isogenies of total length `isog_len`
// using the naive doubling approach.
static void ec_eval_even_sequential(ec_curve_t *image, ec_point_t *points,
                                    unsigned short points_len, ec_point_t *A24,
                                    const ec_point_t *kernel,
                                    const int isog_len) {
  int i, j;
  int rem = isog_len;
  ec_point_t Q, K;
  copy_point(&Q, kernel);
  if (rem & 1) {
    copy_point(&K, &Q);
    for (i = 0; i < rem - 1; i++)
      xDBLv2(&K, &K, A24);
    if (fp2_is_zero(&K.x)) {
      ec_point_t A24_old;
      copy_point(&A24_old, A24);
      xisog_2_singular(A24, A24_old);
      xeval_2_singular(&Q, &Q, 1);
      // global_isog_counters.eval2_sing_ker_count += 1;
      xeval_2_singular(points, points, points_len);
      // global_isog_counters.eval2_sing_pts_count += points_len;
    } else {
      xisog_2(A24, K);
      xeval_2(&Q, &Q, 1);
      // global_isog_counters.eval2_ker_count += 1;
      xeval_2(points, points, points_len);
      // global_isog_counters.eval2_pts_count += points_len;
    }
    rem -= 1;
  }
  for (j = 0; j < rem / 2; j++) {
    copy_point(&K, &Q);
    for (i = 0; i < rem - 2 * j - 2; i++)
      xDBLv2(&K, &K, A24);
    xisog_4(A24, K);
    if (j < rem / 2 - 1) {
      xeval_4(&Q, &Q, 1);
        // global_isog_counters.eval4_ker_count += 1;
    }
    xeval_4(points, points, points_len);
    // global_isog_counters.eval4_pts_count += points_len;
  }

  // Output curve in the form (A:C)
  A24_to_AC(image, A24);
  copy_point(&(image->A24), A24);
}

static void ec_eval_even_strategy(ec_curve_t *image, ec_point_t *points,
                                  unsigned short points_len, ec_point_t *A24,
                                  const ec_point_t *kernel,
                                  const int isog_len) {
  if (isog_len >= 0 && isog_len != POWER_OF_2 - 2) {
    // Sequential implementation for other lengths
    ec_eval_even_sequential(image, points, points_len, A24, kernel, isog_len);
    return;
  }

  assert(isog_len == POWER_OF_2 - 2);
  uint8_t log2_of_e, tmp;
  uint32_t e_half = (uint32_t)(isog_len >> 1);
  for (tmp = (uint8_t)e_half, log2_of_e = 0; tmp > 0; tmp >>= 1, ++log2_of_e)
    ;
  {
    log2_of_e *= 2; // In order to ensure each splits is at most size log2_of_e
  }

  ec_point_t SPLITTING_POINTS[log2_of_e];
  copy_point(&SPLITTING_POINTS[0], kernel);

  int strategy = 0, // Current element of the strategy to be used
      i, j;

  int BLOCK = 0,        // Keeps track of point order
      current = 0;      // Number of points being carried
  int XDBLs[log2_of_e]; // Number of doubles performed

  // If walk length is odd, we start with a 2-isogeny
  if (isog_len & 1) {
    copy_point(&SPLITTING_POINTS[1], &SPLITTING_POINTS[0]);
    for (i = 0; i < isog_len - 1; i++)
      xDBLv2(&SPLITTING_POINTS[1], &SPLITTING_POINTS[1], A24);
    xisog_2(A24, SPLITTING_POINTS[1]);
    xeval_2(SPLITTING_POINTS, SPLITTING_POINTS, 1);
    // global_isog_counters.eval2_ker_count += 1;
    xeval_2(points, points, points_len);
    // global_isog_counters.eval2_pts_count += points_len;
  }

  // Chain of 4-isogenies
  for (j = 0; j < (e_half - 1); j++) {
    // Get the next point of order 4
    while (BLOCK != (e_half - 1 - j)) {
      // A new split will be added
      current += 1;
      // We set the seed of the new split to be computed and saved
      copy_point(&SPLITTING_POINTS[current], &SPLITTING_POINTS[current - 1]);
      for (i = 0; i < 2 * STRATEGY4[strategy]; i++)
        xDBLv2(&SPLITTING_POINTS[current], &SPLITTING_POINTS[current], A24);
      XDBLs[current] =
          STRATEGY4[strategy]; // The number of doublings performed is saved
      BLOCK += STRATEGY4[strategy]; // BLOCK is increased by the number of
                                    // doublings performed
      strategy += 1; // Next, we move to the next element of the strategy
    }

    // Evaluate 4-isogeny
    xisog_4(A24, SPLITTING_POINTS[current]);
    xeval_4(SPLITTING_POINTS, SPLITTING_POINTS, current);
    // global_isog_counters.eval4_ker_count += current;
    xeval_4(points, points, points_len);
    // global_isog_counters.eval4_pts_count += points_len;

    BLOCK -= XDBLs[current];
    XDBLs[current] = 0;
    current -= 1;
  }

  // Final 4-isogeny
  xisog_4(A24, SPLITTING_POINTS[current]);
  xeval_4(points, points, points_len);
  // global_isog_counters.eval4_pts_count += points_len;

  // Output curve in the form (A:C)
  A24_to_AC(image, A24);
  copy_point(&(image->A24), A24);
}

void ec_eval_even(ec_curve_t *image, const ec_isog_even_t *phi,
                  ec_point_t *points, unsigned short length) {
  ec_point_t Q, A24;

  AC_to_A24(&A24, &phi->curve);
  copy_point(&Q, &phi->kernel);

  if (phi->length < 2) {
    ec_eval_even_strategy(image, points, length, &A24, &Q, phi->length);
    return;
  }

  ec_point_t Q4, K2;
  copy_point(&Q4, &Q);
  for (int i = 0; i < phi->length - 2; i++)
    xDBLv2(&Q4, &Q4, &A24);

  xDBLv2(&K2, &Q4, &A24);
  if (fp2_is_zero(&K2.x)) {
    xisog_4_singular(&A24, Q4, A24);
    xeval_4_singular(points, points, length, Q4);
    // global_isog_counters.eval4_sing_pts_count += length;
    xeval_4_singular(&Q, &phi->kernel, 1, Q4);
    // global_isog_counters.eval4_sing_ker_count += 1;
  } else {
    xisog_4(&A24, Q4);
    xeval_4(points, points, length);
    // global_isog_counters.eval4_pts_count += length;
    xeval_4(&Q, &phi->kernel, 1);
    // global_isog_counters.eval4_ker_count += 1;
  }
  ec_eval_even_strategy(image, points, length, &A24, &Q, phi->length - 2);
}

void ec_eval_even_nonzero(ec_curve_t *image, const ec_isog_even_t *phi,
                          ec_point_t *points, unsigned short length) {
  ec_eval_even(image, phi, points, length);
}

static void ec_eval_three_sequential(ec_curve_t *image, ec_curve_t *image_prev,
                                     ec_point_t *points,
                                     unsigned short points_len,
                                     ec_point_t *A24minus,
                                     const ec_point_t *kernel,
                                     const int isog_len) {
  int i, j;
  ec_point_t Q, K;
  ec_point_t A3_local;

  copy_point(&Q, kernel);

  // Initialize A3_local
  fp2_copy(&A3_local.z, &A24minus->x);              // A-2C
  fp2_add(&A3_local.x, &A24minus->x, &A24minus->z); // A+2C

  for (j = 0; j < isog_len; j++) {
    copy_point(&K, &Q);

    for (i = 0; i < isog_len - 1 - j; i++) {
      TPL_A3(&K, &K, &A3_local);
      // measure_three_TPL++;
    }

    if (image_prev != NULL && j == isog_len - 1) {
      A24minus_to_AC(image_prev, A24minus);
      A24minus_to_A24plus(&(image_prev->A24), A24minus);
    }

    xisog_3(A24minus, K);
    // measure_three_ISOG++;
    
    // Update A3_local for the next iteration
    fp2_copy(&A3_local.z, &A24minus->x);
    fp2_add(&A3_local.x, &A24minus->x, &A24minus->z);

    if (j < isog_len - 1) {
      xeval_3(&Q, &Q, 1);
      // measure_three_EVAL_ker += 1;
    }
    if (points != NULL && points_len > 0) {
      xeval_3(points, points, points_len);
      // measure_three_EVAL_pts += points_len;
    }
  }

  // Output curve in the form (A:C) plus legacy A24=(A+2C:4C)
  A24minus_to_AC(image, A24minus);
  A24minus_to_A24plus(&(image->A24), A24minus);
}

static void ec_eval_three_strategy(ec_curve_t *image, ec_curve_t *image_prev,
                                   ec_point_t *points,
                                   unsigned short points_len,
                                   ec_point_t *A24minus,
                                   const ec_point_t *kernel,
                                   const int isog_len) {
  if (isog_len >= 0 && isog_len != POWER_OF_3 - 1) {
    ec_eval_three_sequential(image, image_prev, points, points_len, A24minus,
                             kernel, isog_len);
    return;
  }

  assert(isog_len == POWER_OF_3 - 1);

  uint8_t log2_of_e = 0;
  uint32_t e_len = (uint32_t)isog_len;
  uint32_t tmp32;
  for (tmp32 = e_len; tmp32 > 0; tmp32 >>= 1, ++log2_of_e)
    ;
  log2_of_e *= 2;

  ec_point_t SPLITTING_POINTS[log2_of_e];
  int XTPLs[log2_of_e];

  for (int t = 0; t < (int)log2_of_e; t++) {
    XTPLs[t] = 0;
  }

  copy_point(&SPLITTING_POINTS[0], kernel);

  int strategy = 0, i, j;
  int BLOCK = 0, current = 0;

  ec_point_t A3_local;
  // Initialize A3_local
  fp2_copy(&A3_local.z, &A24minus->x);              // A-2C
  fp2_add(&A3_local.x, &A24minus->x, &A24minus->z); // A+2C

  for (j = 0; j < isog_len; j++) {
    while (BLOCK != (isog_len - 1 - j)) {
      current += 1;
      assert(current < (int)log2_of_e);

      copy_point(&SPLITTING_POINTS[current], &SPLITTING_POINTS[current - 1]);

      for (i = 0; i < STRATEGY3[strategy]; i++) {
        TPL_A3(&SPLITTING_POINTS[current], &SPLITTING_POINTS[current],
               &A3_local);
        // measure_three_TPL++;
      }

      XTPLs[current] = STRATEGY3[strategy];
      BLOCK += STRATEGY3[strategy];
      strategy += 1;
    }

    if (image_prev != NULL && j == isog_len - 1) {
      A24minus_to_AC(image_prev, A24minus);
      A24minus_to_A24plus(&(image_prev->A24), A24minus);
    }

    xisog_3(A24minus, SPLITTING_POINTS[current]);
    // measure_three_ISOG++;
    
    // Update A3_local for the next iteration
    fp2_copy(&A3_local.z, &A24minus->x);
    fp2_add(&A3_local.x, &A24minus->x, &A24minus->z);

    if (current > 0) {
      xeval_3(SPLITTING_POINTS, SPLITTING_POINTS, current);
      // measure_three_EVAL_ker += current;
    }
    if (points != NULL && points_len > 0) {
      xeval_3(points, points, points_len);
      // measure_three_EVAL_pts += points_len;
    }

    BLOCK -= XTPLs[current];
    XTPLs[current] = 0;
    current -= 1;
  }

  A24minus_to_AC(image, A24minus);
  A24minus_to_A24plus(&(image->A24), A24minus);
}

void ec_eval_three(ec_curve_t *image, const ec_isog_odd_t *phi,
                   ec_point_t *points, unsigned short length) {
  ec_point_t ker, A24minus;
  int i;

  AC_to_A24minus(&A24minus, &phi->curve);

  for (i = 1; i < P_LEN + M_LEN; i++) {
    assert(phi->degree[i] == 0);
  }

  copy_point(&ker, &phi->ker_plus);

  if (phi->degree[0] < 1) {
    ec_eval_three_strategy(image, NULL, points, length, &A24minus, &ker,
                           phi->degree[0]);
    return;
  }

  ec_point_t Q3, A3_local;
  copy_point(&Q3, &ker);

  // TPL_A3 expects A3 = (A+2C : A-2C)
  fp2_copy(&A3_local.z, &A24minus.x);             // A-2C
  fp2_add(&A3_local.x, &A24minus.x, &A24minus.z); // A+2C

  for (i = 0; i < phi->degree[0] - 1; i++) {
    TPL_A3(&Q3, &Q3, &A3_local);
    // measure_three_TPL++;
  }

  xisog_3(&A24minus, Q3);
  // measure_three_ISOG++;

  if (length > 0) {
    xeval_3(points, points, length);
    // measure_three_EVAL_pts += length;
  }
  xeval_3(&ker, &ker, 1);
  // measure_three_EVAL_ker += 1;

  ec_eval_three_strategy(image, NULL, points, length, &A24minus, &ker,
                         phi->degree[0] - 1);
}

void ec_eval_three_prev(ec_curve_t *image, ec_curve_t *image_prev,
                        const ec_isog_odd_t *phi) {
  ec_point_t ker, A24minus;
  int i;

  AC_to_A24minus(&A24minus, &phi->curve);

  for (i = 1; i < P_LEN + M_LEN; i++) {
    assert(phi->degree[i] == 0);
  }

  copy_point(&ker, &phi->ker_plus);

  if (phi->degree[0] < 1) {
    ec_eval_three_strategy(image, image_prev, NULL, 0, &A24minus, &ker,
                           phi->degree[0]);
    return;
  }

  ec_point_t Q3, A3_local;
  copy_point(&Q3, &ker);

  // TPL_A3 expects A3 = (A+2C : A-2C)
  fp2_copy(&A3_local.z, &A24minus.x);             // A-2C
  fp2_add(&A3_local.x, &A24minus.x, &A24minus.z); // A+2C

  for (i = 0; i < phi->degree[0] - 1; i++) {
    TPL_A3(&Q3, &Q3, &A3_local);
    // measure_three_TPL++;
  }

  if (image_prev != NULL && phi->degree[0] == 1) {
    A24minus_to_AC(image_prev, &A24minus);
    A24minus_to_A24plus(&(image_prev->A24), &A24minus);
  }

  xisog_3(&A24minus, Q3);
  // measure_three_ISOG++;
  xeval_3(&ker, &ker, 1);
  // measure_three_EVAL_ker += 1;

  ec_eval_three_strategy(image, image_prev, NULL, 0, &A24minus, &ker,
                         phi->degree[0] - 1);
}

// static void ec_eval_three_sequential(ec_curve_t *image,
//                                      ec_curve_t //
//                                          *image_prev,
//                                      ec_point_t *points,
//                                      unsigned short points_len, ec_point_t
//                                      *A24, const ec_point_t *kernel, const
//                                      int isog_len) {
//   int i, j;
//   ec_point_t Q, K;
//   copy_point(&Q, kernel);
//   ec_point_t A3_local;

//   for (j = 0; j < isog_len; j++) {
//     copy_point(&K, &Q);
//     fp2_copy(&A3_local.x, &A24->x);
//     fp2_sub(&A3_local.z, &A24->x, &A24->z);
//     for (i = 0; i < isog_len - 1 - j; i++) {
//       TPL_A3(&K, &K, &A3_local);
//     }

//     if (image_prev != NULL && j == isog_len - 1) {
//       A24_to_AC(image_prev, A24);
//       copy_point(&(image_prev->A24), A24);
//     }

//     xisog_3(A24, K);
//     if (j < isog_len - 1)
//       xeval_3(&Q, &Q, 1);
//     if (points_len > 0)
//       xeval_3(points, points, points_len);
//   }

//   //   // Ou// tput curve in the form (A:C)
//   A24_to_AC(image, A24);
//   copy_point(&(image->A24), A24);
// }

// static void ec_eval_three_strategy(ec_curve_t *image, ec_curve_t *image_prev,
//                                    ec_point_t *points,
//                                    unsigned short points_len, ec_point_t
//                                    *A24, const ec_point_t *kernel, const int
//                                    isog_len) {
//   if (isog_len >= 0 && isog_len != POWER_OF_3) {
//     // Sequential implementation for other lengths
//     ec_eval_three_sequential(image, image_prev, points, points_len, A24,
//     kernel,
//                              isog_len);
//     return;
//   }

//   ec_point_t SPLITTING_POINTS[30];
//   copy_point(&SPLITTING_POINTS[0], kernel);

//   int strategy = 0, i, j;
//   int BLOCK = 0, current = 0; //
//   int XDBLs[30];

//   ec_point_t A3_local;

//   for (j = 0; j < isog_len; j++) {
//     while (BLOCK != (isog_len - 1 - j)) {
//       current += 1;
//       copy_point(&SPLITTING_POINTS[current], &SPLITTING_POINTS[current - 1]);
//       fp2_copy(&A3_local.x, &A24->x);
//       fp2_sub(&A3_local.z, &A24->x, &A24->z);
//       for (i = 0; i < STRATEGY3[strategy]; i++)
//         TPL_A3(&SPLITTING_POINTS[current], &SPLITTING_POINTS[current],
//                &A3_local);
//       XDBLs[current] = STRATEGY3[strategy];
//       BLOCK += STRATEGY3[strategy];
//       strategy += 1;
//     }

//     if (image_prev != NULL && j == isog_len - 1) {
//       A24_to_AC(image_prev, A24);
//       copy_point(&(image_prev->A24), A24);
//     } //

//     // Evaluate 3-isogeny
//     xisog_3(A24, SPLITTING_POINTS[current]);
//     // if (current > 0)
//     xeval_3(SPLITTING_POINTS, SPLITTING_POINTS, current);
//     if (points_len > 0)
//       xeval_3(points, points, points_len);

//     BLOCK -= XDBLs[current];
//     XDBLs[current] = 0;
//     current -= 1;
//   }

//   // Output curve in the f// orm (A:C)
//   A24_to_AC(image, A24);
//   copy_point(&(image->A24), A24);
// }

// void ec_eval_three(ec_curve_t *image, // const ec_isog_odd_t *phi,
//                    ec_point_t *points, unsigned short length) {

//   ec_point_t ker, A24;
//   int i;

//   AC_to_A24(&A24, &phi->curve);

//   for (i = 1; i < P_LEN + M_LEN; i++) {
//     assert(phi->degree[i] == 0);
//   }

//   copy_point(&ker, &phi->ker_plus);
//   ec_eval_three_strategy(image, NULL, points, length, &A24, &ker,
//                          phi->degree[0]);
// }

// void ec_eval_three_prev(ec_curve_t *image, ec_curve_t *image_prev,
//                         const ec_isog_odd_t *phi) {

//   ec_point_t ker, A24;
//   int i;

//   AC_to_A24(&A24, &phi->curve);

//   for (i = 1; i < P_LEN + M_LEN; i++) {
//     assert(phi->degree[i] == 0);
//   }

//   copy_point(&ker, &phi->ker_plus);
//   ec_eval_three_strategy(image, image_prev, NULL, 0, &A24, &ker,
//                          phi->degree[0]);
// }

void ec_eval_odd(ec_curve_t *image, const ec_isog_odd_t *phi,
                 ec_point_t *points, unsigned short length) {

  ec_point_t ker_plus, ker_minus, P, K, A24, B24;
  int i, j, k;

  AC_to_A24(&A24, &phi->curve);

  // Isogenies with kernel in E[p+1]
  copy_point(&ker_plus, &phi->ker_plus);
  copy_point(&ker_minus, &phi->ker_minus);
  for (i = 0; i < P_LEN; i++) {
    if (phi->degree[i] == 0) continue;
    copy_point(&P, &ker_plus);
    for (j = i + 1; j < P_LEN; j++) {
      for (k = 0; k < phi->degree[j]; k++)
        xMULv2(&P, &P, &(TORSION_ODD_PRIMES[j]), p_plus_minus_bitlength[j],
               &A24);
    }
    for (k = 0; k < phi->degree[i]; k++) {
      copy_point(&K, &P);
      for (j = 0; j < phi->degree[i] - k - 1; j++)
        xMULv2(&K, &K, &(TORSION_ODD_PRIMES[i]), p_plus_minus_bitlength[i],
               &A24);
      kps(i, K, A24);
      xisog(&B24, i, A24);
      xeval(&P, i, P, A24);
      xeval(&ker_plus, i, ker_plus, A24);
      xeval(&ker_minus, i, ker_minus, A24);
      for (j = 0; j < length; j++)
        xeval(&points[j], i, points[j], A24);
      copy_point(&A24, &B24);
      kps_clear(i);
    }
  }

  // Isogenies with kernel in E[p-1]
  for (i = P_LEN; i < P_LEN + M_LEN; i++) {
    if (phi->degree[i] == 0) continue;
    copy_point(&P, &ker_minus);
    for (j = i + 1; j < P_LEN + M_LEN; j++) {
      for (k = 0; k < phi->degree[j]; k++)
        xMULv2(&P, &P, &(TORSION_ODD_PRIMES[j]), p_plus_minus_bitlength[j],
               &A24);
    }
    for (k = 0; k < phi->degree[i]; k++) {
      copy_point(&K, &P);
      for (j = 0; j < phi->degree[i] - k - 1; j++)
        xMULv2(&K, &K, &(TORSION_ODD_PRIMES[i]), p_plus_minus_bitlength[i],
               &A24);
      kps(i, K, A24);
      xisog(&B24, i, A24);
      xeval(&P, i, P, A24);
      xeval(&ker_minus, i, ker_minus, A24);
      for (j = 0; j < length; j++)
        xeval(&points[j], i, points[j], A24);
      copy_point(&A24, &B24);
      kps_clear(i);
    }
  }

  A24_to_AC(image, &A24);
  copy_point(&(image->A24), &A24);
}

void ec_curve_normalize(ec_curve_t *new, ec_isom_t *isom,
                        const ec_curve_t *old) {
  fp2_t t0, t1, t2, t3, t4, t5;
  // Compute the other solutions:
  // A'^2 = [ sqrt(A^2-4C^2)*(9C^2-A^2) +- (A^3-3AC^2) ] / [
  // 2C^2*sqrt(A^2-4C^2)
  // ]
  fp2_sqr(&t0, &old->C);      // C^2
  fp2_add(&t1, &t0, &t0);     // 2C^2
  fp2_add(&t2, &t1, &t1);     // 4C^2
  fp2_sqr(&t3, &old->A);      // A^2
  fp2_sub(&t2, &t3, &t2);     // A^2-4C^2
  fp2_sqrt(&t2);              // sqrt(A^2-4C^2)
  fp2_add(&t0, &t0, &t1);     // 3C^2
  fp2_mul(&t1, &t2, &t1);     // 2C^2*sqrt(A^2-4C^2)
  fp2_sub(&t5, &t3, &t0);     // A^2-3C^2
  fp2_mul(&t5, &t5, &old->A); // A^3-3AC^2
  fp2_add(&t4, &t0, &t0);     // 6C^2
  fp2_add(&t0, &t4, &t0);     // 9C^2
  fp2_sub(&t0, &t0, &t3);     // 9C^2-A^2
  fp2_add(&t3, &t3, &t3);     // 2A^2
  fp2_mul(&t3, &t3, &t2);     // 2A^2*sqrt(A^2-4C^2)
  fp2_mul(&t2, &t2, &t0);     // sqrt(A^2-4C^2)*(9C^2-A^2)
  fp2_add(&t0, &t2, &t5);     // sqrt(A^2-4C^2)*(9C^2-A^2) + (A^3-3AC^2)
  fp2_sub(&t2, &t2, &t5);     // sqrt(A^2-4C^2)*(9C^2-A^2) - (A^3-3AC^2)
  fp2_inv(&t1);               // 1/2C^2*sqrt(A^2-4C^2)
  fp2_mul(&t0, &t0, &t1);     // First solution
  fp2_mul(&t2, &t2, &t1);     // Second solution
  fp2_mul(&t1, &t3, &t1);     // Original solution

  // Chose the lexicographically first solution
  if (fp2_cmp(&t0, &t1) == 1)
    fp2_copy(&t0, &t1);
  if (fp2_cmp(&t0, &t2) == 1)
    fp2_copy(&t0, &t2);

  // Copy the solution
  fp2_sqrt(&t0);
  ec_curve_t E;
  fp2_copy(&E.A, &t0);
  fp_mont_setone(E.C.re);
  fp_set(E.C.im, 0);
  ec_isomorphism(isom, old, &E);
  fp2_copy(&new->A, &E.A);
  fp2_copy(&new->C, &E.C);
  AC_to_A24(&(new->A24), new);
}

void ec_isomorphism(ec_isom_t *isom, const ec_curve_t *from,
                    const ec_curve_t *to) {
  fp2_t t0, t1, t2, t3, t4;
  fp2_mul(&t0, &from->A, &to->C);
  fp2_sqr(&t0, &t0); // fromA^2toC^2
  fp2_mul(&t1, &to->A, &from->C);
  fp2_sqr(&t1, &t1); // toA^2fromC^2
  fp2_mul(&t2, &to->C, &from->C);
  fp2_sqr(&t2, &t2); // toC^2fromC^2
  fp2_add(&t3, &t2, &t2);
  fp2_add(&t2, &t3, &t2); // 3toC^2fromC^2
  fp2_sub(&t3, &t2, &t0); // 3toC^2fromC^2-fromA^2toC^2
  fp2_sub(&t4, &t2, &t1); // 3toC^2fromC^2-toA^2fromC^2
  fp2_inv(&t3);
  fp2_mul(&t4, &t4, &t3);
  fp2_sqrt(&t4); // lambda^2 constant for SW isomorphism
  fp2_sqr(&t3, &t4);
  fp2_mul(&t3, &t3, &t4); // lambda^6

  // Check sign of lambda^2, such that lambda^6 has the right sign
  fp2_sqr(&t0, &from->C);
  fp2_add(&t1, &t0, &t0);
  fp2_add(&t1, &t1, &t1);
  fp2_add(&t1, &t1, &t1);
  fp2_add(&t0, &t0, &t1); // 9fromC^2
  fp2_sqr(&t2, &from->A);
  fp2_add(&t2, &t2, &t2); // 2fromA^2
  fp2_sub(&t2, &t2, &t0);
  fp2_mul(&t2, &t2, &from->A); // -9fromC^2fromA+2fromA^3
  fp2_sqr(&t0, &to->C);
  fp2_mul(&t0, &t0, &to->C);
  fp2_mul(&t2, &t2, &t0); // toC^3* [-9fromC^2fromA+2fromA^3]
  fp2_mul(&t3, &t3, &t2); // lambda^6*(-9fromA+2fromA^3)*toC^3
  fp2_sqr(&t0, &to->C);
  fp2_add(&t1, &t0, &t0);
  fp2_add(&t1, &t1, &t1);
  fp2_add(&t1, &t1, &t1);
  fp2_add(&t0, &t0, &t1); // 9toC^2
  fp2_sqr(&t2, &to->A);
  fp2_add(&t2, &t2, &t2); // 2toA^2
  fp2_sub(&t2, &t2, &t0);
  fp2_mul(&t2, &t2, &to->A); // -9toC^2toA+2toA^3
  fp2_sqr(&t0, &from->C);
  fp2_mul(&t0, &t0, &from->C);
  fp2_mul(&t2, &t2, &t0); // fromC^3* [-9toC^2toA+2toA^3]
  if (!fp2_is_equal(&t2, &t3))
    fp2_neg(&t4, &t4);

  // Mont -> SW -> SW -> Mont
  fp_mont_setone(t0.re);
  fp_set(t0.im, 0);
  fp2_add(&isom->D, &t0, &t0);
  fp2_add(&isom->D, &isom->D, &t0);
  fp2_mul(&isom->D, &isom->D, &from->C);
  fp2_mul(&isom->D, &isom->D, &to->C);
  fp2_mul(&isom->Nx, &isom->D, &t4);
  fp2_mul(&t4, &t4, &from->A);
  fp2_mul(&t4, &t4, &to->C);
  fp2_mul(&t0, &to->A, &from->C);
  fp2_sub(&isom->Nz, &t0, &t4);
}

void ec_iso_inv(ec_isom_t *isom) {
  fp2_t tmp;
  fp2_copy(&tmp, &isom->D);
  fp2_copy(&isom->D, &isom->Nx);
  fp2_copy(&isom->Nx, &tmp);
  fp2_neg(&isom->Nz, &isom->Nz);
}

void ec_iso_eval(ec_point_t *P, ec_isom_t *isom) {
  fp2_t tmp;
  fp2_mul(&P->x, &P->x, &isom->Nx);
  fp2_mul(&tmp, &P->z, &isom->Nz);
  fp2_sub(&P->x, &P->x, &tmp);
  fp2_mul(&P->z, &P->z, &isom->D);
}
