#ifndef EC_PARAMS_H
#define EC_PARAMS_H

#include <tutil.h>

#include <fp_constants.h>

#define POWER_OF_2 191
#define POWER_OF_3 117

static digit_t TWOpF[NWORDS_ORDER] = {
    0x0000000000000000,
    0x0000000000000000,
    0x8000000000000000,
    0x0000000000000000,
    0x0000000000000000,
    0x0000000000000000}; // Fp representation for the power of 2
static digit_t TWOpFm1[NWORDS_ORDER] = {
    0x0000000000000000,
    0x0000000000000000,
    0x4000000000000000,
    0x0000000000000000,
    0x0000000000000000,
    0x0000000000000000}; // Fp representation for half the power of 2
static digit_t THREEpE[NWORDS_ORDER] = {
    0x4fe184719e8ae269,
    0x000000000f38208c,
    0x0000000000000000,
    0x0000000000000000,
    0x0000000000000000,
    0x0000000000000000}; // Approximate squareroot of the power of 3
static digit_t THREEpF[NWORDS_ORDER] = {
    0x168daa8d7854ad33,
    0x50f398d3119cf9ea,
    0x02b6e05c18a8432d,
    0x0000000000000000,
    0x0000000000000000,
    0x0000000000000000}; // Fp representation for the power of 3
static digit_t THREEpFdiv2[NWORDS_ORDER] = {
    0x0b46d546bc2a5699, 0xa879cc6988ce7cf5,
    0x015b702e0c542196, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000}; // Floor of half the power of 3

#define scaled                                                                 \
  1 // unscaled (0) or scaled (1) remainder tree approach for squareroot velu
#define gap                                                                    \
  83 // Degree above which we use squareroot velu reather than traditional

#define P_LEN 1 // Number of odd primes in p+1
#define M_LEN 1 // Number of odd primes in p-1

// Bitlength of the odd prime factors
static digit_t p_plus_minus_bitlength[P_LEN + M_LEN] = {2, 10};

// p+1 divided by the power of 2
static digit_t p_cofactor_for_2f[3] = {0x168daa8d7854ad33, 0x50f398d3119cf9ea,
                                       0x02b6e05c18a8432d};
#define P_COFACTOR_FOR_2F_BITLENGTH 186

// p+1 divided by the power of 3
static digit_t p_cofactor_for_3g[3] = {0x0000000000000000, 0x0000000000000000,
                                       0x8000000000000000};
#define P_COFACTOR_FOR_3G_BITLENGTH 192

// p+1 divided by the powers of 2 and 3
static digit_t p_cofactor_for_6fg[1] = {0x0000000000000001};
#define P_COFACTOR_FOR_6FG_BITLENGTH 1

// Strategy for 3-isogenies
static int STRATEGY3[116] = {
    48, 28, 16, 9, 6, 4, 2, 1, 1, 1, 2, 1, 1, 2, 2, 1, 1, 1, 4,  2, 1, 1, 1,
    2,  1,  1,  7, 4, 2, 1, 1, 1, 2, 1, 1, 3, 2, 1, 1, 1, 1, 12, 7, 4, 2, 1,
    1,  1,  2,  1, 1, 3, 2, 1, 1, 1, 1, 5, 3, 2, 1, 1, 1, 1, 2,  1, 1, 1, 20,
    12, 7,  4,  2, 1, 1, 1, 2, 1, 1, 3, 2, 1, 1, 1, 1, 5, 3, 2,  1, 1, 1, 1,
    2,  1,  1,  1, 8, 5, 3, 2, 1, 1, 1, 1, 2, 1, 1, 1, 3, 2, 1,  1, 1, 1, 1};
// static int STRATEGY3[116] = {
//     48, 28, 16, 9, 6, 4, 2, 1, 1, 1, 2, 1, 1, 2, 2, 1, 1, 1, 4,  2, 1, 1, 1,
//     2,  1,  1,  7, 4, 2, 1, 1, 1, 2, 1, 1, 3, 2, 1, 1, 1, 1, 12, 7, 4, 2, 1,
//     1,  1,  2,  1, 1, 3, 2, 1, 1, 1, 1, 5, 3, 2, 1, 1, 1, 1, 2,  1, 1, 1, 20,
//     12, 7,  4,  2, 1, 1, 1, 2, 1, 1, 3, 2, 1, 1, 1, 1, 5, 3, 2,  1, 1, 1, 1,
//     2,  1,  1,  1, 8, 5, 3, 2, 1, 1, 1, 1, 2, 1, 1, 1, 3, 2, 1,  1, 1, 1, 1};

// Strategy for 4-isogenies
static int STRATEGY4[94] = {
    38, 24, 15, 8, 4, 2, 1, 1, 1,  2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 7, 4, 2, 1, 1,
    2,  1,  1,  3, 2, 1, 1, 1, 1,  9, 7, 4, 2, 1, 1, 2, 1, 1, 3, 2, 1, 1, 1, 1,
    4,  2,  1,  1, 1, 2, 1, 1, 17, 9, 5, 3, 2, 1, 1, 1, 1, 2, 1, 1, 1, 4, 2, 1,
    1,  1,  2,  1, 1, 8, 4, 2, 1,  1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1, 1};

// static int STRATEGY4[94] = {
//     38, 24, 15, 8, 4, 2, 1, 1, 1,  2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 7, 4, 2, 1,
//     1, 2,  1,  1,  3, 2, 1, 1, 1, 1,  9, 7, 4, 2, 1, 1, 2, 1, 1, 3, 2, 1, 1,
//     1, 1, 4,  2,  1,  1, 1, 2, 1, 1, 17, 9, 5, 3, 2, 1, 1, 1, 1, 2, 1, 1, 1,
//     4, 2, 1, 1,  1,  2,  1, 1, 8, 4, 2, 1,  1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1,
//     1};

// Optimal sizes for I,J,K in squareroot Velu
static int sizeI[2] = {1, 16};
static int sizeJ[2] = {0, 15};
static int sizeK[2] = {1, 30};

#define sI_max 16
#define sJ_max 15
#define sK_max 83

#define ceil_log_sI_max 4
#define ceil_log_sJ_max 4

#endif
