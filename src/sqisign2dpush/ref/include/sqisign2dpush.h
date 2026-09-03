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
  uint8_t n1;
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
 * Each field holds the wall-clock time in milliseconds for the
 * corresponding step.  Pass a pointer to protocols_keygen() to
 * fill it; pass NULL to skip timing entirely.
 */
typedef struct keygen_timings {
  float ms_doublepath;         ///< time for doublepath()
  float ms_basis_two_hint;     ///< time for ec_curve_to_basis_2f_to_hint()
  float ms_change_basis_two;   ///< time for change_of_basis_matrix_two()
  float ms_basis_three_hint;   ///< time for ec_curve_to_basis_3f_to_hint()
  float ms_change_basis_three; ///< time for change_of_basis_matrix_three()
  float
      ms_eval_even; ///< cumulative time for ec_eval_even() inside doublepath()
  float ms_eval_three; ///< cumulative time for ec_eval_three() inside
                       ///< doublepath()
  float ms_dp_represent_integer;
  float ms_dp_lideal_create;
  float ms_dp_quat_to_isog_two;
  float ms_dp_quat_to_kernel_three;
  float ms_dp_quat_to_isog_three;
  float ms_dp_quat_to_kernel_two;
  float ms_dp_eval_even1;
  float ms_dp_isog_init_three;
  float ms_dp_biscalar1;
  float ms_dp_complete_three_basis;
  float ms_dp_curve_to_basis_2;
  float ms_dp_eval_three1;
  float ms_dp_eval_three2;
  float ms_dp_biscalar2;
  float ms_dp_isog_init_two;
  float ms_dp_complete_two_basis;
  float ms_dp_curve_to_basis_3;
  float ms_dp_eval_even2;
  float ms_dp_dlog_3_tate_R;
  float ms_dp_biscalar3;
  float ms_dp_eval_three3;
  float ms_dp_dlog_2_tate_R;
  float ms_dp_biscalar4;
  float ms_dp_eval_even3;
  unsigned long total_isog_length_even;  ///< cumulative chain length for degree
                                         ///< 2^e isogenies
  unsigned long total_isog_length_three; ///< cumulative chain length for degree
                                         ///< 3^e isogenies
  float ms_keygen_isog;
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
 * Each field holds the cumulative wall-clock time in milliseconds for the
 * corresponding step over all retry iterations.
 * Pass a pointer to protocols_sign() to fill it; pass NULL to skip timing.
 */
typedef struct sign_timings {
  float ms_commit; ///< cumulative time for commit()
  float
      ms_challenge; ///< cumulative time for hash_to_challenge() + kernel setup
  float
      ms_response; ///< cumulative time for sample_response() + post-processing
  float ms_aux;
  float ms_hash_to_challenge;
  float ms_challenge_biscalar;
  float ms_challenge_eval_three;
  float ms_challenge_ideal;
  float ms_challenge_lideal_inter;
  float ms_challenge_lideal_mul;
  float ms_generator_coprime;
  float ms_lideal_mul_tmp;
  float ms_lideal_isom;
  float ms_lideal_conjugate_lattice;
  float ms_lattice_intersect;
  float ms_sample_response;
  float ms_response_quat_mul;
  float ms_response_matrix;
  float ms_response_matrix_apply;
  float ms_response_basis_hint;
  float ms_response_change_basis;
  float ms_aux_norm;
  float ms_pushrandisog;
  float ms_pri_setup;
  float ms_pri_choose_e;
  float ms_pri_composed_rand_isog;
  float ms_pri_delta_doubles;
  float ms_pri_copy_kernel;
  float ms_pri_theta_setup;
  float ms_pri_theta_chain;
  float ms_pri_weil;
  float ms_pri_select_codomain;
  float ms_pri_eval_three;
  float ms_cri_setup;
  float ms_cri_basis_even_doubles;
  float ms_cri_inv_scalar;
  float ms_cri_represent_integer;
  float ms_cri_endomorphism_matrix;
  float ms_cri_matrix_application;
  float ms_cri_kernel_three;
  float ms_cri_kernel_two;
  float ms_cri_kernel_two_doubles;
  float ms_cri_eval_even_delta;
  float ms_cri_eval_three_first;
  float ms_cri_basis_three;
  float ms_cri_copy_kernel;
  float ms_cri_theta_setup;
  float ms_cri_theta_chain;
  float ms_cri_dlog_3_tate_R;
  float ms_cri_biscalar_final;
  float ms_cri_eval_three_final;
  float ms_aux_odd_adjust;
  float ms_aux_basis_hint;
  float ms_aux_change_basis;
  float ms_aux_matrix_finalize;
  float ms_fc_represent_integer;
  float ms_fc_lideal_create;
  float ms_fc_quat_to_isog_two;
  float ms_fc_quat_to_kernel_three;
  float ms_fc_quat_to_isog_three;
  float ms_fc_quat_to_kernel_two;
  float ms_fc_quat_to_quat;
  float ms_fc_quat_to_ec;
  float ms_fc_quat_to_other;
  float ms_fc_biscalar1;
  float ms_fc_complete_three_basis;
  float ms_fc_eval_even1;
  float ms_fc_eval_three1;
  float ms_fc_eval_three2;
  float ms_fc_curve_to_basis_3;
  float ms_fc_biscalar2;
  float ms_fc_eval_even2;
  float ms_fc_dlog_3_tate_R;
  float ms_fc_biscalar3;
  float ms_fc_eval_three3;
  unsigned long total_sample_response_trials; ///< cumulative number of trials
                                              ///< in sample_response()
  unsigned long total_isog_length_even_commit;
  unsigned long total_isog_length_three_commit;
  unsigned long total_isog_length_three_challenge;
  unsigned long total_isog_length_even_response;
  unsigned long total_isog_length_three_response;
  unsigned long total_isog_length_two_two_response;
  float ms_commit_isog;
  float ms_challenge_isog;
  float ms_response_isog;
  float ms_aux_isog;
} sign_timings_t;

typedef struct verify_timings {
  float ms_eval_three; ///< cumulative time for ec_eval_three_prev()
  float ms_basis_three_hint;
  float ms_challenge_biscalar;
  float ms_eval_three_prev;
  float ms_three_isogenous_coeff_to_kernel;
  float ms_challenge_basis_hint;
  float ms_matrix_application;
  float ms_aux_basis_hint;
  float ms_kernel_iso2;
  float ms_eval_even;
  float ms_aux_doubles;
  float ms_copy_bases_to_kernel;
  float ms_theta_setup;
  float ms_theta_chain_verify;
  float ms_hash_challenge;
  float ms_final_check;
  unsigned long total_isog_length_three; ///< cumulative chain length for degree
                                         ///< 3^e isogenies
  unsigned long total_isog_length_even;
  unsigned long total_isog_length_two_two;
} verify_timings_t;

int fastcommit(quat_alg_elem_t *gamma, quat_left_ideal_t *lideal_even,
               quat_left_ideal_t *lideal_odd, ec_basis_t *basis_two_image,
               ec_curve_t *E_target, ec_point_t *kernel_iso_three1,
               ec_point_t *kernel_iso_three2, ec_curve_t *E_com_mid,
               ec_basis_t *basis_two_mid, int verbose, sign_timings_t *timings);
void ec_point_init(ec_point_t *P);
void ec_normalize_curve_and_A24(ec_curve_t *E);
void xDBL_A24(ec_point_t *Q, const ec_point_t *P, const ec_point_t *A24);
void xDBL_E0(ec_point_t *Q, const ec_point_t *P);
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
