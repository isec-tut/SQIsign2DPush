/** @file
 *
 * @authors Antonin Leroux
 *
 * @brief The protocols
 */

#ifndef SQISIGN2DPUSH_H
#define SQISIGN2DPUSH_H

#include <ec.h>
#include <encoded_sizes.h>
#include <endomorphism_action.h>
#include <fp_constants.h>
#include <intbig.h>
#include <klpt_constants.h>
#include <quaternion.h>
#include <quaternion_data.h>
#include <rng.h>
#include <torsion_constants.h>

#include <id2iso.h>
#include <klpt.h>
#include <stdio.h>

/** @defgroup sqisign2dpush_sqisign2dpush sqisign2dpush protocols
 * @{
 */
/** @defgroup sqisign2dpush_t Types for sqisign2dpush protocols
 * @{
 */

/** @brief Type for the signature
 *
 * @typedef signature_t
 *
 * @struct signature
 *
 */

typedef digit_t scalar_t[NWORDS_ORDER];

typedef struct signature {
  ec_curve_t E_aux; /// auxiliary curve
  uint16_t n1;
  ibz_mat_2x2_t mat_rsp;
  scalar_t challenge;
  uint8_t hint_aux;
  uint8_t hint_chall;
} signature_t;

/** @brief Type for the public keys
 *
 * @typedef public_key_t
 *
 * @struct public_key
 *
 */
typedef struct public_key {
  ec_curve_t curve; /// the normalized A coefficient of the Montgomery curve
  // uint8_t hint_pk_even;
  uint16_t hint_pk_three;
} public_key_t;

/** @brief Type for the secret keys
 *
 * @typedef secret_key_t
 *
 * @struct secret_key
 *
 */
typedef struct secret_key {
  ec_curve_t curve; /// the public curve
  quat_left_ideal_t secret_ideal_two;
  quat_alg_elem_t two_to_three_transporter;
  // ibz_mat_2x2_t
  //     mat_BAcan_to_BA0_two; /// mat_BA0_to_BAcan*BA0 = BAcan, where BAcan is the
  //                           /// canonical basis of EA[2^e], and BA0 the image of
  //                           /// the basis of E0[2^e] through the secret odd
  //                           /// isogeny
  ibz_mat_2x2_t
      mat_BAcan_to_BA0_three; /// mat_BA0_to_BAcan*BA0 = BAcan, where BAcan is
  /// the canonical basis of EA[2^e], and BA0 the
  /// image of the basis of E0[2^e] through the
  /// secret odd isogeny

  // ec_basis_t phi_sk1_even;
  ec_basis_t phi_sk2_three;
} secret_key_t;

/** @}
 */

/*************************** Functions *****************************/

/** @brief Timing data collected during protocols_keygen
 *
 * @typedef keygen_timings_t
 *
 * Each field holds the process CPU time in milliseconds for the
 * corresponding step.  Pass a pointer to protocols_keygen() to
 * fill it; pass NULL to skip category accumulation.
 */
typedef struct keygen_timings {
  float ms_keygen_ec_isog;
  float ms_keygen_ec_non_isog;
  float ms_keygen_quat;
} keygen_timings_t;

// basis_two is set to dual_three(basis of E0)
void doublepath(quat_alg_elem_t *gamma, quat_left_ideal_t *lideal_even,
                quat_left_ideal_t *lideal_odd, ec_basis_t *basis_three,
                ec_basis_t *basis_two, ec_curve_t *E_target, int verbose,
                keygen_timings_t *timings);

/** @brief Timing data collected during protocols_sign
 *
 * @typedef sign_timings_t
 *
 * Each field holds the cumulative process CPU time in milliseconds for the
 * corresponding step over all retry iterations.
 * Pass a pointer to protocols_sign() to fill it; pass NULL to skip timing.
 */
typedef struct sign_timings {
  float ms_sign_isog;
  float ms_sign_ec_non_isog;
  float ms_sign_quat;
  float ms_commit;
  float ms_challenge;
  float ms_response; ///< includes unsuccessful sampling attempts
} sign_timings_t;

typedef struct verify_timings {
  float ms_ver_ec_isog;
  float ms_ver_ec_non_isog;
  float ms_ver_quat;
} verify_timings_t;

int fastcommit(quat_alg_elem_t *gamma, quat_left_ideal_t *lideal_even,
               quat_left_ideal_t *lideal_odd, ec_basis_t *basis_two_image,
               ec_curve_t *E_target, ec_point_t *kernel_iso_three1,
               ec_point_t *kernel_iso_three2, ec_curve_t *E_com_mid,
               ec_basis_t *basis_two_mid, int verbose, sign_timings_t *timings);
void isog_init_two_f(ec_isog_even_t *isog, const ec_curve_t *curve,
                     const ec_point_t *ker, int length);
void isog_init_three(ec_isog_odd_t *isog, const ec_curve_t *curve,
                     const ec_point_t *ker, int length);
int sqisign2dpush_test_point_order_twof(const ec_point_t *P,
                                        const ec_curve_t *E);
int sqisign2dpush_test_point_order_threef(const ec_point_t *P,
                                          const ec_curve_t *E);

/** @brief Key-generation protocol
 *
 * @param pk         output public key
 * @param sk         output secret key
 * @param timings    if non-NULL, receives per-step timing data (ms)
 */
void protocols_keygen(public_key_t *pk, secret_key_t *sk,
                      keygen_timings_t *timings);

/** @brief Signing protocol
 *
 * @param sig        output signature
 * @param pk         public key
 * @param sk         secret key
 * @param m          message
 * @param l          message length
 * @param verbose    verbosity flag
 * @param timings    if non-NULL, receives per-step timing data (ms)
 */
int protocols_sign(signature_t *sig, const public_key_t *pk,
                   const secret_key_t *sk, const unsigned char *m, size_t l,
                   int verbose, sign_timings_t *timings);
int protocols_verify(signature_t *sig, const public_key_t *pk,
                     const unsigned char *m, size_t l,
                     verify_timings_t *timings);

void secret_key_init(secret_key_t *sk);
void secret_key_finalize(secret_key_t *sk);
void secret_sig_init(signature_t *sig);
void secret_sig_finalize(signature_t *sig);

/* Binary codecs: 1 on success, 0 on invalid input/length; outputs are unchanged
 * on failure. Use the exact *_BYTES lengths. Initialize secret keys and
 * signatures with secret_key_init / secret_sig_init before decoding.
 * Decoding checks the format, not signature validity or full key consistency.
 * Secret-key encoding supports keys produced by the current DoublePath.
 */
int public_key_to_bytes(unsigned char *out, size_t size, const public_key_t *pk);
int public_key_from_bytes(public_key_t *pk, const unsigned char *in, size_t size);
int signature_to_bytes(unsigned char *out, size_t size, const signature_t *sig);
int signature_from_bytes(signature_t *sig, const unsigned char *in, size_t size);
int secret_key_to_bytes(unsigned char *out, size_t size, const secret_key_t *sk,
                        const public_key_t *pk);
int secret_key_from_bytes(secret_key_t *sk, public_key_t *pk,
                          const unsigned char *in, size_t size);

void print_signature(const signature_t *sig);
void print_public_key(const public_key_t *pk);
void print_secret_key(const secret_key_t *sk);

/** @defgroup signature The signature protocol
 * @{
 */

/** @}
 */

/** @}
 */

#endif
