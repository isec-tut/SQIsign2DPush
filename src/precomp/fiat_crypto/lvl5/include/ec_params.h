#ifndef EC_PARAMS_H
#define EC_PARAMS_H

#include <tutil.h>

#include <fp_constants.h>

#define POWER_OF_2 263
#define POWER_OF_3 156

static digit_t TWOpF[NWORDS_ORDER] = {
    0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000080, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000}; // Fp representation for the power
                                             // of 2
static digit_t TWOpFm1[NWORDS_ORDER] = {
    0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000040, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000}; // Fp representation for half the
                                             // power of 2
static digit_t THREEpE[NWORDS_ORDER] = {
    0x94fd9829d87f5079, 0x0c5afe6ff302bcbf, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000}; // Approximate squareroot of the
                                             // power of 3
static digit_t THREEpF[NWORDS_ORDER] = {
    0x36988ddf2159d931, 0x4171b143838aa52d, 0xbe4694b88a96dc00,
    0x0098a832626176cc, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000}; // Fp representation for the power
                                             // of 3
static digit_t THREEpFdiv2[NWORDS_ORDER] = {
    0x9b4c46ef90acec98, 0x20b8d8a1c1c55296, 0x5f234a5c454b6e00,
    0x004c54193130bb66, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x0000000000000000}; // Floor of half the power of 3

#define scaled                                                                 \
  1 // unscaled (0) or scaled (1) remainder tree approach for squareroot velu
#define gap                                                                    \
  83 // Degree above which we use squareroot velu reather than traditional

#define P_LEN 1 // Number of odd primes in p+1
#define M_LEN 5 // Number of odd primes in p-1

// Bitlength of the odd prime factors
static digit_t p_plus_minus_bitlength[P_LEN + M_LEN] = {2, 4, 5, 6, 10, 14};

// p+1 divided by the power of 2
static digit_t p_cofactor_for_2f[4] = {0x36988ddf2159d931, 0x4171b143838aa52d,
                                       0xbe4694b88a96dc00, 0x0098a832626176cc};
#define P_COFACTOR_FOR_2F_BITLENGTH 248

// p+1 divided by the power of 3
static digit_t p_cofactor_for_3g[5] = {0x0000000000000000, 0x0000000000000000,
                                       0x0000000000000000, 0x0000000000000000,
                                       0x0000000000000080};
#define P_COFACTOR_FOR_3G_BITLENGTH 264

// p+1 divided by the powers of 2 and 3
static digit_t p_cofactor_for_6fg[1] = {0x0000000000000001};
#define P_COFACTOR_FOR_6FG_BITLENGTH 1

static int STRATEGY3[155] = {
    61, 38, 22, 13, 8, 5, 3,  2,  1, 1, 1, 1, 1, 2,  1, 1, 1, 3, 2, 1, 1, 1,
    1,  1,  5,  3,  2, 1, 1,  1,  1, 1, 2, 1, 1, 1,  9, 5, 3, 2, 1, 1, 1, 1,
    1,  2,  1,  1,  1, 4, 2,  1,  1, 1, 2, 1, 1, 16, 9, 5, 3, 2, 1, 1, 1, 1,
    1,  2,  1,  1,  1, 4, 2,  1,  1, 1, 2, 1, 1, 7,  4, 2, 1, 1, 1, 2, 1, 1,
    3,  2,  1,  1,  1, 1, 23, 16, 9, 5, 3, 2, 1, 1,  1, 1, 1, 2, 1, 1, 1, 4,
    2,  1,  1,  1,  2, 1, 1,  7,  4, 2, 1, 1, 1, 2,  1, 1, 3, 2, 1, 1, 1, 1,
    9,  5,  4,  2,  1, 1, 1,  2,  1, 1, 2, 1, 1, 1,  4, 2, 1, 1, 1, 2, 1, 1};
// static int STRATEGY3[155] = {
//     61, 38, 22, 13, 8, 5, 3,  2,  1, 1, 1, 1, 1, 2,  1, 1, 1, 3, 2, 1, 1, 1,
//     1,  1,  5,  3,  2, 1, 1,  1,  1, 1, 2, 1, 1, 1,  9, 5, 3, 2, 1, 1, 1, 1,
//     1,  2,  1,  1,  1, 4, 2,  1,  1, 1, 2, 1, 1, 16, 9, 5, 3, 2, 1, 1, 1, 1,
//     1,  2,  1,  1,  1, 4, 2,  1,  1, 1, 2, 1, 1, 7,  4, 2, 1, 1, 1, 2, 1, 1,
//     3,  2,  1,  1,  1, 1, 23, 16, 9, 5, 3, 2, 1, 1,  1, 1, 1, 2, 1, 1, 1, 4,
//     2,  1,  1,  1,  2, 1, 1,  7,  4, 2, 1, 1, 1, 2,  1, 1, 3, 2, 1, 1, 1, 1,
//     9,  5,  4,  2,  1, 1, 1,  2,  1, 1, 2, 1, 1, 1,  4, 2, 1, 1, 1, 2, 1, 1};

// Strategy for 4-isogenies
static int STRATEGY4[130] = {
    59, 32, 17, 9, 5, 4,  2,  1, 1, 2, 1, 1, 2, 1, 1, 1,  4, 2, 1,  1, 1, 2,
    1,  1,  8,  4, 2, 1,  1,  1, 2, 1, 1, 4, 2, 1, 1, 2,  1, 1, 15, 8, 4, 2,
    1,  1,  1,  2, 1, 1,  4,  2, 1, 1, 2, 1, 1, 7, 4, 2,  1, 1, 2,  1, 1, 3,
    2,  1,  1,  1, 1, 27, 15, 8, 4, 2, 1, 1, 1, 2, 1, 1,  4, 2, 1,  1, 2, 1,
    1,  7,  4,  2, 1, 1,  2,  1, 1, 3, 2, 1, 1, 1, 1, 12, 7, 4, 2,  1, 1, 2,
    1,  1,  3,  2, 1, 1,  1,  1, 5, 3, 2, 1, 1, 1, 1, 2,  1, 1, 1};

// static int STRATEGY4[130] = {
//     59, 32, 17, 9, 5, 4,  2,  1, 1, 2, 1, 1, 2, 1, 1, 1,  4, 2, 1,  1, 1, 2,
//     1,  1,  8,  4, 2, 1,  1,  1, 2, 1, 1, 4, 2, 1, 1, 2,  1, 1, 15, 8, 4, 2,
//     1,  1,  1,  2, 1, 1,  4,  2, 1, 1, 2, 1, 1, 7, 4, 2,  1, 1, 2,  1, 1, 3,
//     2,  1,  1,  1, 1, 27, 15, 8, 4, 2, 1, 1, 1, 2, 1, 1,  4, 2, 1,  1, 2, 1,
//     1,  7,  4,  2, 1, 1,  2,  1, 1, 3, 2, 1, 1, 1, 1, 12, 7, 4, 2,  1, 1, 2,
//     1,  1,  3,  2, 1, 1,  1,  1, 5, 3, 2, 1, 1, 1, 1, 2,  1, 1, 1};

// Optimal sizes for I,J,K in squareroot Velu
static int sizeI[6] = {1, 2, 2, 4, 16, 62};
static int sizeJ[6] = {0, 1, 2, 3, 14, 61};
static int sizeK[6] = {1, 1, 0, 5, 25, 100};

#define sI_max 62
#define sJ_max 61
#define sK_max 100

#define ceil_log_sI_max 6
#define ceil_log_sJ_max 6

#endif
