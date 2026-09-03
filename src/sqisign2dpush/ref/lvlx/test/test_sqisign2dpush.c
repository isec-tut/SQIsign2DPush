#include <ec.h>
#include <fips202.h>
#include <inttypes.h>
#include <rng.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "test_sqisign2dpush.h"
#include "toolbox.h"

#if 0
extern uint64_t count_ec_dlog_2;
extern uint64_t count_ec_dlog_2_tate_R;
extern uint64_t count_ec_dlog_2_tate;
extern uint64_t count_ec_dlog_3;
extern uint64_t count_ec_dlog_3_tate_R;
extern uint64_t count_ec_dlog_3_tate;

extern fp2_counters_t count_ec_dlog_2_tate_R_fp2;
extern fp2_counters_t count_ec_dlog_2_tate_fp2;
extern fp2_counters_t count_ec_dlog_3_tate_R_fp2;
extern fp2_counters_t count_ec_dlog_3_tate_fp2;
#endif

static inline int64_t cpucycles(void) {
#if (defined(TARGET_AMD64) || defined(TARGET_X86))
  unsigned int hi, lo;

  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((int64_t)lo) | (((int64_t)hi) << 32);
#elif (defined(TARGET_S390X))
  uint64_t tod;
  asm volatile("stckf %0\n" : "=Q"(tod) : : "cc");
  return (tod * 1000 / 4096);
#else
  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  return (int64_t)(time.tv_sec * 1e9 + time.tv_nsec);
#endif
}

static __inline__ uint64_t rdtsc(void) { return (uint64_t)cpucycles(); }

// XXX FIXME stolen from src/ec/opt/generic/test/isog-test.c
static void fp2_print(char *name, fp2_t const a) {
  fp2_t b;
  fp2_set(&b, 1);
  fp2_mul(&b, &b, &a);
  printf("%s = 0x", name);
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016" PRIx64, b.re[i]);
  printf(" + i*0x");
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016" PRIx64, b.im[i]);
  printf("\n");
}

static void point_print(char *name, ec_point_t P) {
  fp2_t a;
  if (fp2_is_zero(&P.z)) {
    printf("%s = INF\n", name);
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
// XXX

bool curve_is_canonical(ec_curve_t const *E) {
  ec_curve_t EE;
  ec_isom_t isom;
  ec_curve_normalize(&EE, &isom, E);

  fp2_t lhs, rhs;
  fp2_mul(&lhs, &E->A, &EE.C);
  fp2_mul(&rhs, &E->C, &EE.A);
  return fp2_is_equal(&lhs, &rhs);
}

static int cmp_double(const void *a, const void *b) {
  const double va = *(const double *)a;
  const double vb = *(const double *)b;
  return (va > vb) - (va < vb);
}

static double average_double(const double *samples, int n) {
  double total = 0.0;
  for (int i = 0; i < n; ++i) {
    total += samples[i];
  }
  return total / (double)n;
}

static double median_double(const double *samples, int n) {
  if (n & 1) {
    return samples[n / 2];
  }
  return (samples[n / 2 - 1] + samples[n / 2]) / 2.0;
}

static void print_cycle_stat_line(const char *stat, const char *name,
                                  double value) {
  int pad = 17 - (int)(strlen(stat) + 1 + strlen(name) + 1);
  if (pad < 1) {
    pad = 1;
  }
  printf("\x1b[34m%s %s:%*s%.5f Mcycles\x1b[0m\n", stat, name, pad, "",
         value);
}

static void print_time_stats(const char *name, double *ms_samples, int repeat,
                             double mcycles_per_ms) {
  qsort(ms_samples, (size_t)repeat, sizeof(*ms_samples), cmp_double);
  print_cycle_stat_line("Avg", name,
                        average_double(ms_samples, repeat) * mcycles_per_ms);
  print_cycle_stat_line("Median", name,
                        median_double(ms_samples, repeat) * mcycles_per_ms);
  print_cycle_stat_line("Min", name, ms_samples[0] * mcycles_per_ms);
  print_cycle_stat_line("Max", name, ms_samples[repeat - 1] * mcycles_per_ms);
}

static void print_category_ratio(const char *label, double ms, double total) {
  printf("    \x1b[35m%-29s %.5f ms (%6.2f%%)\x1b[0m\n", label, ms,
         total > 0.0 ? 100.0 * ms / total : 0.0);
}

static void add_sign_timings(sign_timings_t *total,
                             const sign_timings_t *sample) {
  total->ms_commit += sample->ms_commit;
  total->ms_challenge += sample->ms_challenge;
  total->ms_response += sample->ms_response;
  total->ms_aux += sample->ms_aux;
  total->ms_commit_isog += sample->ms_commit_isog;
  total->ms_challenge_isog += sample->ms_challenge_isog;
  total->ms_response_isog += sample->ms_response_isog;
  total->ms_aux_isog += sample->ms_aux_isog;
  total->ms_hash_to_challenge += sample->ms_hash_to_challenge;
  total->ms_challenge_biscalar += sample->ms_challenge_biscalar;
  total->ms_challenge_eval_three += sample->ms_challenge_eval_three;
  total->ms_challenge_ideal += sample->ms_challenge_ideal;
  total->ms_challenge_lideal_inter += sample->ms_challenge_lideal_inter;
  total->ms_challenge_lideal_mul += sample->ms_challenge_lideal_mul;
  total->ms_generator_coprime += sample->ms_generator_coprime;
  total->ms_lideal_mul_tmp += sample->ms_lideal_mul_tmp;
  total->ms_lideal_isom += sample->ms_lideal_isom;
  total->ms_lideal_conjugate_lattice +=
      sample->ms_lideal_conjugate_lattice;
  total->ms_lattice_intersect += sample->ms_lattice_intersect;
  total->ms_sample_response += sample->ms_sample_response;
  total->ms_response_quat_mul += sample->ms_response_quat_mul;
  total->ms_response_matrix += sample->ms_response_matrix;
  total->ms_response_matrix_apply += sample->ms_response_matrix_apply;
  total->ms_response_basis_hint += sample->ms_response_basis_hint;
  total->ms_response_change_basis += sample->ms_response_change_basis;
  total->ms_aux_norm += sample->ms_aux_norm;
  total->ms_pushrandisog += sample->ms_pushrandisog;
  total->ms_pri_setup += sample->ms_pri_setup;
  total->ms_pri_choose_e += sample->ms_pri_choose_e;
  total->ms_pri_composed_rand_isog += sample->ms_pri_composed_rand_isog;
  total->ms_pri_delta_doubles += sample->ms_pri_delta_doubles;
  total->ms_pri_copy_kernel += sample->ms_pri_copy_kernel;
  total->ms_pri_theta_setup += sample->ms_pri_theta_setup;
  total->ms_pri_theta_chain += sample->ms_pri_theta_chain;
  total->ms_pri_weil += sample->ms_pri_weil;
  total->ms_pri_select_codomain += sample->ms_pri_select_codomain;
  total->ms_pri_eval_three += sample->ms_pri_eval_three;
  total->ms_cri_setup += sample->ms_cri_setup;
  total->ms_cri_basis_even_doubles += sample->ms_cri_basis_even_doubles;
  total->ms_cri_inv_scalar += sample->ms_cri_inv_scalar;
  total->ms_cri_represent_integer += sample->ms_cri_represent_integer;
  total->ms_cri_endomorphism_matrix += sample->ms_cri_endomorphism_matrix;
  total->ms_cri_matrix_application += sample->ms_cri_matrix_application;
  total->ms_cri_kernel_three += sample->ms_cri_kernel_three;
  total->ms_cri_kernel_two += sample->ms_cri_kernel_two;
  total->ms_cri_kernel_two_doubles += sample->ms_cri_kernel_two_doubles;
  total->ms_cri_eval_even_delta += sample->ms_cri_eval_even_delta;
  total->ms_cri_eval_three_first += sample->ms_cri_eval_three_first;
  total->ms_cri_basis_three += sample->ms_cri_basis_three;
  total->ms_cri_copy_kernel += sample->ms_cri_copy_kernel;
  total->ms_cri_theta_setup += sample->ms_cri_theta_setup;
  total->ms_cri_theta_chain += sample->ms_cri_theta_chain;
  total->ms_cri_dlog_3_tate_R += sample->ms_cri_dlog_3_tate_R;
  total->ms_cri_biscalar_final += sample->ms_cri_biscalar_final;
  total->ms_cri_eval_three_final += sample->ms_cri_eval_three_final;
  total->ms_aux_odd_adjust += sample->ms_aux_odd_adjust;
  total->ms_aux_basis_hint += sample->ms_aux_basis_hint;
  total->ms_aux_change_basis += sample->ms_aux_change_basis;
  total->ms_aux_matrix_finalize += sample->ms_aux_matrix_finalize;
  total->ms_fc_represent_integer += sample->ms_fc_represent_integer;
  total->ms_fc_lideal_create += sample->ms_fc_lideal_create;
  total->ms_fc_quat_to_isog_two += sample->ms_fc_quat_to_isog_two;
  total->ms_fc_quat_to_kernel_three += sample->ms_fc_quat_to_kernel_three;
  total->ms_fc_quat_to_isog_three += sample->ms_fc_quat_to_isog_three;
  total->ms_fc_quat_to_kernel_two += sample->ms_fc_quat_to_kernel_two;
  total->ms_fc_quat_to_quat += sample->ms_fc_quat_to_quat;
  total->ms_fc_quat_to_ec += sample->ms_fc_quat_to_ec;
  total->ms_fc_quat_to_other += sample->ms_fc_quat_to_other;
  total->ms_fc_biscalar1 += sample->ms_fc_biscalar1;
  total->ms_fc_complete_three_basis += sample->ms_fc_complete_three_basis;
  total->ms_fc_eval_even1 += sample->ms_fc_eval_even1;
  total->ms_fc_eval_three1 += sample->ms_fc_eval_three1;
  total->ms_fc_eval_three2 += sample->ms_fc_eval_three2;
  total->ms_fc_curve_to_basis_3 += sample->ms_fc_curve_to_basis_3;
  total->ms_fc_biscalar2 += sample->ms_fc_biscalar2;
  total->ms_fc_eval_even2 += sample->ms_fc_eval_even2;
  total->ms_fc_dlog_3_tate_R += sample->ms_fc_dlog_3_tate_R;
  total->ms_fc_biscalar3 += sample->ms_fc_biscalar3;
  total->ms_fc_eval_three3 += sample->ms_fc_eval_three3;
  total->total_sample_response_trials += sample->total_sample_response_trials;
}

static void add_verify_timings(verify_timings_t *total,
                               const verify_timings_t *sample) {
  total->ms_eval_three += sample->ms_eval_three;
  total->ms_basis_three_hint += sample->ms_basis_three_hint;
  total->ms_challenge_biscalar += sample->ms_challenge_biscalar;
  total->ms_eval_three_prev += sample->ms_eval_three_prev;
  total->ms_three_isogenous_coeff_to_kernel +=
      sample->ms_three_isogenous_coeff_to_kernel;
  total->ms_challenge_basis_hint += sample->ms_challenge_basis_hint;
  total->ms_matrix_application += sample->ms_matrix_application;
  total->ms_aux_basis_hint += sample->ms_aux_basis_hint;
  total->ms_kernel_iso2 += sample->ms_kernel_iso2;
  total->ms_eval_even += sample->ms_eval_even;
  total->ms_aux_doubles += sample->ms_aux_doubles;
  total->ms_copy_bases_to_kernel += sample->ms_copy_bases_to_kernel;
  total->ms_theta_setup += sample->ms_theta_setup;
  total->ms_theta_chain_verify += sample->ms_theta_chain_verify;
  total->ms_hash_challenge += sample->ms_hash_challenge;
  total->ms_final_check += sample->ms_final_check;
}

int test_sqisign(int repeat) {
  int res = 1;

  if (repeat <= 0) {
    return 0;
  }

  double *ms_kg_samples = calloc((size_t)repeat, sizeof(*ms_kg_samples));
  double *ms_sig_samples = calloc((size_t)repeat, sizeof(*ms_sig_samples));
  double *ms_ver_samples = calloc((size_t)repeat, sizeof(*ms_ver_samples));
  public_key_t *pks = calloc((size_t)repeat, sizeof(*pks));
  secret_key_t *sks = calloc((size_t)repeat, sizeof(*sks));
  signature_t *sigs = calloc((size_t)repeat, sizeof(*sigs));
  if (ms_kg_samples == NULL || ms_sig_samples == NULL ||
      ms_ver_samples == NULL || pks == NULL || sks == NULL || sigs == NULL) {
    free(ms_kg_samples);
    free(ms_sig_samples);
    free(ms_ver_samples);
    free(pks);
    free(sks);
    free(sigs);
    return 0;
  }

  unsigned char msg[32] = {0};

  for (int i = 0; i < repeat; ++i) {
    secret_key_init(&sks[i]);
    secret_sig_init(&sigs[i]);
  }

  uint64_t t0, t1, dt_kg, dt_sig, dt_ver;
  clock_t time1, time2;
  dt_kg = 0;
  dt_sig = 0;
  dt_ver = 0;

#if 0
  // fp2 operation accumulators for dlog algorithms
  fp2_counters_t kg_dlog_2_tate_R_fp2 = {0};
  fp2_counters_t kg_dlog_2_tate_fp2 = {0};
  fp2_counters_t kg_dlog_3_tate_R_fp2 = {0};
  fp2_counters_t kg_dlog_3_tate_fp2 = {0};

  fp2_counters_t sg_dlog_2_tate_R_fp2 = {0};
  fp2_counters_t sg_dlog_2_tate_fp2 = {0};
  fp2_counters_t sg_dlog_3_tate_R_fp2 = {0};
  fp2_counters_t sg_dlog_3_tate_fp2 = {0};

  fp2_counters_t vr_dlog_2_tate_R_fp2 = {0};
  fp2_counters_t vr_dlog_2_tate_fp2 = {0};
  fp2_counters_t vr_dlog_3_tate_R_fp2 = {0};
  fp2_counters_t vr_dlog_3_tate_fp2 = {0};
#endif

  // Per-step timing accumulators for keygen internals
  float ms_doublepath_total = 0.f;
  float ms_basis_two_hint_total = 0.f;
  float ms_change_basis_two_total = 0.f;
  float ms_basis_three_hint_total = 0.f;
  float ms_change_basis_three_total = 0.f;
  // float ms_eval_even_total = 0.f;
  // float ms_eval_three_total = 0.f;

  // float ms_keygen_isog_total = 0.f;
  // float ms_commit_isog_total = 0.f;
  // float ms_challenge_isog_total = 0.f;
  // float ms_response_isog_total = 0.f;
  // float ms_aux_isog_total = 0.f;

  // t0 = rdtsc();
  // clock_t time1 = clock();
  // protocols_keygen(&pk, &sk, NULL); // first warm-up call (no timing)
  // clock_t time2 = clock();
  // t1 = rdtsc();
  // printf("\x1b[34mkeygen          took %.2f ms\x1b[0m\n",
  //        1000.0 * (double)(time2 - time1) / CLOCKS_PER_SEC);

  // printf("Printing details of first signature\n");
  // int val = protocols_sign(&sig, &pk, &sk, msg, 32, 1, NULL);

  // printf("\n\nTesting more keygen, sign and verify\n");
  float ms_keygen = 0;
  float ms_sign = 0;
  float ms_verify = 0;

#if 0
  uint64_t total_kg_dlog_2 = 0, total_kg_dlog_2_tate_R = 0,
           total_kg_dlog_2_tate = 0;
  uint64_t total_kg_dlog_3 = 0, total_kg_dlog_3_tate_R = 0,
           total_kg_dlog_3_tate = 0;

  uint64_t total_sg_dlog_2 = 0, total_sg_dlog_2_tate_R = 0,
           total_sg_dlog_2_tate = 0;
  uint64_t total_sg_dlog_3 = 0, total_sg_dlog_3_tate_R = 0,
           total_sg_dlog_3_tate = 0;

  uint64_t total_vr_dlog_2 = 0, total_vr_dlog_2_tate_R = 0,
           total_vr_dlog_2_tate = 0;
  uint64_t total_vr_dlog_3 = 0, total_vr_dlog_3_tate_R = 0,
           total_vr_dlog_3_tate = 0;
#endif

  //   unsigned char public_key_bytes[PUBLICKEY_BYTES + 1];
  //   public_key_bytes[PUBLICKEY_BYTES] = 0;
  //   public_key_to_bytes(public_key_bytes, &pk);
  //   printf("Public key: %s\n", public_key_bytes);

  keygen_timings_t kg_timings;
  keygen_timings_t kg_timings_total = {0};
  sign_timings_t sg_timings;
  sign_timings_t sg_timings_total = {0};
  verify_timings_t vr_timings;
  verify_timings_t vr_timings_total = {0};

  // float ms_verify_eval_three_total = 0.f;

  // unsigned long total_isog_length_even_kg = 0UL;
  // unsigned long total_isog_length_three_kg = 0UL;
  // unsigned long total_isog_length_two_two_kg = 0UL;

  // unsigned long total_isog_length_even_vr = 0UL;
  // unsigned long total_isog_length_three_vr = 0UL;
  // unsigned long total_isog_length_two_two_vr = 0UL;

  printf("Testing keygen for %d iterations\n", repeat);
  for (int i = 0; i < repeat; ++i) {
#if 0
    count_ec_dlog_2 = 0;
    count_ec_dlog_2_tate_R = 0;
    count_ec_dlog_2_tate = 0;
    count_ec_dlog_3 = 0;
    count_ec_dlog_3_tate_R = 0;
    count_ec_dlog_3_tate = 0;
    memset(&count_ec_dlog_2_tate_R_fp2, 0, sizeof(fp2_counters_t));
    memset(&count_ec_dlog_2_tate_fp2, 0, sizeof(fp2_counters_t));
    memset(&count_ec_dlog_3_tate_R_fp2, 0, sizeof(fp2_counters_t));
    memset(&count_ec_dlog_3_tate_fp2, 0, sizeof(fp2_counters_t));
#endif

    t0 = rdtsc();
    time1 = clock();
    protocols_keygen(&pks[i], &sks[i], &kg_timings); // second and subsequent calls (timed)
    time2 = clock();
    t1 = rdtsc();
    ms_kg_samples[i] = 1000.0 * (double)(time2 - time1) / CLOCKS_PER_SEC;
    ms_keygen += ms_kg_samples[i];
    dt_kg += (t1 - t0);

#if 0
    total_kg_dlog_2 += count_ec_dlog_2;
    total_kg_dlog_2_tate_R += count_ec_dlog_2_tate_R;
    total_kg_dlog_2_tate += count_ec_dlog_2_tate;
    total_kg_dlog_3 += count_ec_dlog_3;
    total_kg_dlog_3_tate_R += count_ec_dlog_3_tate_R;
    total_kg_dlog_3_tate += count_ec_dlog_3_tate;

    kg_dlog_2_tate_R_fp2.add += count_ec_dlog_2_tate_R_fp2.add;
    kg_dlog_2_tate_R_fp2.sqr += count_ec_dlog_2_tate_R_fp2.sqr;
    kg_dlog_2_tate_R_fp2.mul += count_ec_dlog_2_tate_R_fp2.mul;
    kg_dlog_2_tate_R_fp2.inv += count_ec_dlog_2_tate_R_fp2.inv;
    kg_dlog_2_tate_R_fp2.sqrt += count_ec_dlog_2_tate_R_fp2.sqrt;

    kg_dlog_2_tate_fp2.add += count_ec_dlog_2_tate_fp2.add;
    kg_dlog_2_tate_fp2.sqr += count_ec_dlog_2_tate_fp2.sqr;
    kg_dlog_2_tate_fp2.mul += count_ec_dlog_2_tate_fp2.mul;
    kg_dlog_2_tate_fp2.inv += count_ec_dlog_2_tate_fp2.inv;
    kg_dlog_2_tate_fp2.sqrt += count_ec_dlog_2_tate_fp2.sqrt;

    kg_dlog_3_tate_R_fp2.add += count_ec_dlog_3_tate_R_fp2.add;
    kg_dlog_3_tate_R_fp2.sqr += count_ec_dlog_3_tate_R_fp2.sqr;
    kg_dlog_3_tate_R_fp2.mul += count_ec_dlog_3_tate_R_fp2.mul;
    kg_dlog_3_tate_R_fp2.inv += count_ec_dlog_3_tate_R_fp2.inv;
    kg_dlog_3_tate_R_fp2.sqrt += count_ec_dlog_3_tate_R_fp2.sqrt;

    kg_dlog_3_tate_fp2.add += count_ec_dlog_3_tate_fp2.add;
    kg_dlog_3_tate_fp2.sqr += count_ec_dlog_3_tate_fp2.sqr;
    kg_dlog_3_tate_fp2.mul += count_ec_dlog_3_tate_fp2.mul;
    kg_dlog_3_tate_fp2.inv += count_ec_dlog_3_tate_fp2.inv;
    kg_dlog_3_tate_fp2.sqrt += count_ec_dlog_3_tate_fp2.sqrt;
#endif

    // Accumulate per-step times
    ms_doublepath_total += kg_timings.ms_doublepath;
    ms_basis_two_hint_total += kg_timings.ms_basis_two_hint;
    // ms_keygen_isog_total += kg_timings.ms_keygen_isog;
    ms_change_basis_two_total += kg_timings.ms_change_basis_two;
    ms_basis_three_hint_total += kg_timings.ms_basis_three_hint;
    ms_change_basis_three_total += kg_timings.ms_change_basis_three;
    kg_timings_total.ms_dp_represent_integer +=
        kg_timings.ms_dp_represent_integer;
    kg_timings_total.ms_dp_lideal_create += kg_timings.ms_dp_lideal_create;
    kg_timings_total.ms_dp_quat_to_isog_two +=
        kg_timings.ms_dp_quat_to_isog_two;
    kg_timings_total.ms_dp_quat_to_kernel_three +=
        kg_timings.ms_dp_quat_to_kernel_three;
    kg_timings_total.ms_dp_quat_to_isog_three +=
        kg_timings.ms_dp_quat_to_isog_three;
    kg_timings_total.ms_dp_quat_to_kernel_two +=
        kg_timings.ms_dp_quat_to_kernel_two;
    kg_timings_total.ms_dp_eval_even1 += kg_timings.ms_dp_eval_even1;
    kg_timings_total.ms_dp_isog_init_three +=
        kg_timings.ms_dp_isog_init_three;
    kg_timings_total.ms_dp_biscalar1 += kg_timings.ms_dp_biscalar1;
    kg_timings_total.ms_dp_complete_three_basis +=
        kg_timings.ms_dp_complete_three_basis;
    kg_timings_total.ms_dp_curve_to_basis_2 +=
        kg_timings.ms_dp_curve_to_basis_2;
    kg_timings_total.ms_dp_eval_three1 += kg_timings.ms_dp_eval_three1;
    kg_timings_total.ms_dp_eval_three2 += kg_timings.ms_dp_eval_three2;
    kg_timings_total.ms_dp_biscalar2 += kg_timings.ms_dp_biscalar2;
    kg_timings_total.ms_dp_isog_init_two += kg_timings.ms_dp_isog_init_two;
    kg_timings_total.ms_dp_complete_two_basis +=
        kg_timings.ms_dp_complete_two_basis;
    kg_timings_total.ms_dp_curve_to_basis_3 +=
        kg_timings.ms_dp_curve_to_basis_3;
    kg_timings_total.ms_dp_eval_even2 += kg_timings.ms_dp_eval_even2;
    kg_timings_total.ms_dp_dlog_3_tate_R +=
        kg_timings.ms_dp_dlog_3_tate_R;
    kg_timings_total.ms_dp_biscalar3 += kg_timings.ms_dp_biscalar3;
    kg_timings_total.ms_dp_eval_three3 += kg_timings.ms_dp_eval_three3;
    kg_timings_total.ms_dp_dlog_2_tate_R +=
        kg_timings.ms_dp_dlog_2_tate_R;
    kg_timings_total.ms_dp_biscalar4 += kg_timings.ms_dp_biscalar4;
    kg_timings_total.ms_dp_eval_even3 += kg_timings.ms_dp_eval_even3;
    // ms_eval_even_total += kg_timings.ms_eval_even;
    // ms_eval_three_total += kg_timings.ms_eval_three;
    // total_isog_length_even_kg += kg_timings.total_isog_length_even;
    // total_isog_length_three_kg += kg_timings.total_isog_length_three;
  }
  printf("finished\n");
  printf("Testing sign for %d iterations\n", repeat);
  for (int i = 0; i < repeat; ++i) {
#if 0
    count_ec_dlog_2 = 0;
    count_ec_dlog_2_tate_R = 0;
    count_ec_dlog_2_tate = 0;
    count_ec_dlog_3 = 0;
    count_ec_dlog_3_tate_R = 0;
    count_ec_dlog_3_tate = 0;
    memset(&count_ec_dlog_2_tate_R_fp2, 0, sizeof(fp2_counters_t));
    memset(&count_ec_dlog_2_tate_fp2, 0, sizeof(fp2_counters_t));
    memset(&count_ec_dlog_3_tate_R_fp2, 0, sizeof(fp2_counters_t));
    memset(&count_ec_dlog_3_tate_fp2, 0, sizeof(fp2_counters_t));
#endif

    t0 = rdtsc();
    time1 = clock();
    protocols_sign(&sigs[i], &pks[i], &sks[i], msg, 32, 0, &sg_timings);
    time2 = clock();
    t1 = rdtsc();
    ms_sig_samples[i] = 1000.0 * (double)(time2 - time1) / CLOCKS_PER_SEC;
    ms_sign += ms_sig_samples[i];
    dt_sig += (t1 - t0);

#if 0
    total_sg_dlog_2 += count_ec_dlog_2;
    total_sg_dlog_2_tate_R += count_ec_dlog_2_tate_R;
    total_sg_dlog_2_tate += count_ec_dlog_2_tate;
    total_sg_dlog_3 += count_ec_dlog_3;
    total_sg_dlog_3_tate_R += count_ec_dlog_3_tate_R;
    total_sg_dlog_3_tate += count_ec_dlog_3_tate;

    sg_dlog_2_tate_R_fp2.add += count_ec_dlog_2_tate_R_fp2.add;
    sg_dlog_2_tate_R_fp2.sqr += count_ec_dlog_2_tate_R_fp2.sqr;
    sg_dlog_2_tate_R_fp2.mul += count_ec_dlog_2_tate_R_fp2.mul;
    sg_dlog_2_tate_R_fp2.inv += count_ec_dlog_2_tate_R_fp2.inv;
    sg_dlog_2_tate_R_fp2.sqrt += count_ec_dlog_2_tate_R_fp2.sqrt;

    sg_dlog_2_tate_fp2.add += count_ec_dlog_2_tate_fp2.add;
    sg_dlog_2_tate_fp2.sqr += count_ec_dlog_2_tate_fp2.sqr;
    sg_dlog_2_tate_fp2.mul += count_ec_dlog_2_tate_fp2.mul;
    sg_dlog_2_tate_fp2.inv += count_ec_dlog_2_tate_fp2.inv;
    sg_dlog_2_tate_fp2.sqrt += count_ec_dlog_2_tate_fp2.sqrt;

    sg_dlog_3_tate_R_fp2.add += count_ec_dlog_3_tate_R_fp2.add;
    sg_dlog_3_tate_R_fp2.sqr += count_ec_dlog_3_tate_R_fp2.sqr;
    sg_dlog_3_tate_R_fp2.mul += count_ec_dlog_3_tate_R_fp2.mul;
    sg_dlog_3_tate_R_fp2.inv += count_ec_dlog_3_tate_R_fp2.inv;
    sg_dlog_3_tate_R_fp2.sqrt += count_ec_dlog_3_tate_R_fp2.sqrt;

    sg_dlog_3_tate_fp2.add += count_ec_dlog_3_tate_fp2.add;
    sg_dlog_3_tate_fp2.sqr += count_ec_dlog_3_tate_fp2.sqr;
    sg_dlog_3_tate_fp2.mul += count_ec_dlog_3_tate_fp2.mul;
    sg_dlog_3_tate_fp2.inv += count_ec_dlog_3_tate_fp2.inv;
    sg_dlog_3_tate_fp2.sqrt += count_ec_dlog_3_tate_fp2.sqrt;
#endif

    add_sign_timings(&sg_timings_total, &sg_timings);
  }
  printf("finished\n");
  printf("Testing verify for %d iterations\n", repeat);
  for (int i = 0; i < repeat; ++i) {
#if 0
    count_ec_dlog_2 = 0;
    count_ec_dlog_2_tate_R = 0;
    count_ec_dlog_2_tate = 0;
    count_ec_dlog_3 = 0;
    count_ec_dlog_3_tate_R = 0;
    count_ec_dlog_3_tate = 0;
    memset(&count_ec_dlog_2_tate_R_fp2, 0, sizeof(fp2_counters_t));
    memset(&count_ec_dlog_2_tate_fp2, 0, sizeof(fp2_counters_t));
    memset(&count_ec_dlog_3_tate_R_fp2, 0, sizeof(fp2_counters_t));
    memset(&count_ec_dlog_3_tate_fp2, 0, sizeof(fp2_counters_t));
#endif

    t0 = rdtsc();
    time1 = clock();
    int v = protocols_verify(&sigs[i], &pks[i], msg, 32, &vr_timings);
    time2 = clock();
    t1 = rdtsc();
    ms_ver_samples[i] = 1000.0 * (double)(time2 - time1) / CLOCKS_PER_SEC;
    ms_verify += ms_ver_samples[i];
    dt_ver += (t1 - t0);

#if 0
    total_vr_dlog_2 += count_ec_dlog_2;
    total_vr_dlog_2_tate_R += count_ec_dlog_2_tate_R;
    total_vr_dlog_2_tate += count_ec_dlog_2_tate;
    total_vr_dlog_3 += count_ec_dlog_3;
    total_vr_dlog_3_tate_R += count_ec_dlog_3_tate_R;
    total_vr_dlog_3_tate += count_ec_dlog_3_tate;

    vr_dlog_2_tate_R_fp2.add += count_ec_dlog_2_tate_R_fp2.add;
    vr_dlog_2_tate_R_fp2.sqr += count_ec_dlog_2_tate_R_fp2.sqr;
    vr_dlog_2_tate_R_fp2.mul += count_ec_dlog_2_tate_R_fp2.mul;
    vr_dlog_2_tate_R_fp2.inv += count_ec_dlog_2_tate_R_fp2.inv;
    vr_dlog_2_tate_R_fp2.sqrt += count_ec_dlog_2_tate_R_fp2.sqrt;

    vr_dlog_2_tate_fp2.add += count_ec_dlog_2_tate_fp2.add;
    vr_dlog_2_tate_fp2.sqr += count_ec_dlog_2_tate_fp2.sqr;
    vr_dlog_2_tate_fp2.mul += count_ec_dlog_2_tate_fp2.mul;
    vr_dlog_2_tate_fp2.inv += count_ec_dlog_2_tate_fp2.inv;
    vr_dlog_2_tate_fp2.sqrt += count_ec_dlog_2_tate_fp2.sqrt;

    vr_dlog_3_tate_R_fp2.add += count_ec_dlog_3_tate_R_fp2.add;
    vr_dlog_3_tate_R_fp2.sqr += count_ec_dlog_3_tate_R_fp2.sqr;
    vr_dlog_3_tate_R_fp2.mul += count_ec_dlog_3_tate_R_fp2.mul;
    vr_dlog_3_tate_R_fp2.inv += count_ec_dlog_3_tate_R_fp2.inv;
    vr_dlog_3_tate_R_fp2.sqrt += count_ec_dlog_3_tate_R_fp2.sqrt;

    vr_dlog_3_tate_fp2.add += count_ec_dlog_3_tate_fp2.add;
    vr_dlog_3_tate_fp2.sqr += count_ec_dlog_3_tate_fp2.sqr;
    vr_dlog_3_tate_fp2.mul += count_ec_dlog_3_tate_fp2.mul;
    vr_dlog_3_tate_fp2.inv += count_ec_dlog_3_tate_fp2.inv;
    vr_dlog_3_tate_fp2.sqrt += count_ec_dlog_3_tate_fp2.sqrt;
#endif

    add_verify_timings(&vr_timings_total, &vr_timings);

    // Accumulate per-step times for verify (commented out)
    // total_isog_length_even_vr += vr_timings.total_isog_length_even;
    // total_isog_length_three_vr += vr_timings.total_isog_length_three;
    // total_isog_length_two_two_vr += vr_timings.total_isog_length_two_two;

    if (!v) {
      printf("verify %d: failed!\n", i);
      print_public_key(&pks[i]);
      print_secret_key(&sks[i]);

      print_signature(&sigs[i]);
      res = 0;
      goto cleanup;
    } else {
      // printf("verify %d: ok\n", i);
    }
  }
  printf("finished\n");

  const double ms_total = (double)ms_keygen + (double)ms_sign + (double)ms_verify;
  const double cycles_total = (double)dt_kg + (double)dt_sig + (double)dt_ver;
  const double mcycles_per_ms = (ms_total > 0.0) ? cycles_total / ms_total / 1e6
                                                 : 0.0;

  printf("\nAverage keygen time [%.5f ms]\n", (float)(ms_keygen / repeat));
  print_time_stats("keygen", ms_kg_samples, repeat, mcycles_per_ms);
  const double avg_keygen = (double)ms_keygen / (double)repeat;
  const double keygen_isog =
      (double)(kg_timings_total.ms_dp_eval_even1 +
               kg_timings_total.ms_dp_eval_even2 +
               kg_timings_total.ms_dp_eval_even3 +
               kg_timings_total.ms_dp_eval_three1 +
               kg_timings_total.ms_dp_eval_three2 +
               kg_timings_total.ms_dp_eval_three3 +
               kg_timings_total.ms_dp_isog_init_three +
               kg_timings_total.ms_dp_isog_init_two) /
      (double)repeat;
  const double keygen_ec_non_isog =
      (double)(kg_timings_total.ms_dp_biscalar1 +
               kg_timings_total.ms_dp_biscalar2 +
               kg_timings_total.ms_dp_biscalar3 +
               kg_timings_total.ms_dp_biscalar4 +
               kg_timings_total.ms_dp_complete_three_basis +
               kg_timings_total.ms_dp_complete_two_basis +
               kg_timings_total.ms_dp_curve_to_basis_2 +
               kg_timings_total.ms_dp_curve_to_basis_3 +
               kg_timings_total.ms_dp_dlog_3_tate_R +
               kg_timings_total.ms_dp_dlog_2_tate_R) /
          (double)repeat +
      (double)(ms_basis_two_hint_total + ms_change_basis_two_total +
               ms_basis_three_hint_total + ms_change_basis_three_total) /
          (double)repeat;
  const double keygen_quat =
      (double)(kg_timings_total.ms_dp_represent_integer +
               kg_timings_total.ms_dp_lideal_create +
               kg_timings_total.ms_dp_quat_to_isog_two +
               kg_timings_total.ms_dp_quat_to_kernel_three +
               kg_timings_total.ms_dp_quat_to_isog_three +
               kg_timings_total.ms_dp_quat_to_kernel_two) /
      (double)repeat;
  double keygen_other =
      avg_keygen - keygen_isog - keygen_ec_non_isog - keygen_quat;
  if (keygen_other < 0.0) {
    keygen_other = 0.0;
  }
  printf("  \x1b[35mKeygen category ratios (denominator: avg keygen)\x1b[0m\n");
  print_category_ratio("Isogeny computations:", keygen_isog, avg_keygen);
  print_category_ratio("EC/basis except isogeny:", keygen_ec_non_isog,
                       avg_keygen);
  print_category_ratio("Quaternion algorithms:", keygen_quat, avg_keygen);
  print_category_ratio("Other:", keygen_other, avg_keygen);
#if 0
  // printf("  \x1b[33mAvg Keygen Isogeny:        %.5f ms\x1b[0m\n",
  //        ms_keygen_isog_total / repeat);
  printf("  \x1b[33mAvg doublepath:                  %.5f ms\x1b[0m\n",
         ms_doublepath_total / repeat);
  printf("    \x1b[36mAvg represent_integer:             %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_represent_integer / repeat);
  printf("    \x1b[36mAvg quat_lideal_create_from_primitive: %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_lideal_create / repeat);
  printf("    \x1b[36mAvg quat_to_isog_power_of_two:    %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_quat_to_isog_two / repeat);
  printf("    \x1b[36mAvg quat_to_kernel_power_of_three: %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_quat_to_kernel_three / repeat);
  printf("    \x1b[36mAvg quat_to_isog_power_of_three:  %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_quat_to_isog_three / repeat);
  printf("    \x1b[36mAvg quat_to_kernel_power_of_two:  %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_quat_to_kernel_two / repeat);
  printf("    \x1b[36mAvg ec_eval_even1:                %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_eval_even1 / repeat);
  printf("    \x1b[36mAvg isog_init_three:             %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_isog_init_three / repeat);
  printf("    \x1b[36mAvg ec_biscalar_mul_ibz1:         %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_biscalar1 / repeat);
  printf("    \x1b[36mAvg complete_three_basis:         %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_complete_three_basis / repeat);
  printf("    \x1b[36mAvg ec_curve_to_basis_2f_hint:    %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_curve_to_basis_2 / repeat);
  printf("    \x1b[36mAvg ec_eval_three1:               %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_eval_three1 / repeat);
  printf("    \x1b[36mAvg ec_eval_three2:               %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_eval_three2 / repeat);
  printf("    \x1b[36mAvg ec_biscalar_mul_ibz2:         %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_biscalar2 / repeat);
  printf("    \x1b[36mAvg isog_init_two:               %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_isog_init_two / repeat);
  printf("    \x1b[36mAvg complete_two_basis:           %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_complete_two_basis / repeat);
  printf("    \x1b[36mAvg ec_curve_to_basis_3f_hint:    %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_curve_to_basis_3 / repeat);
  printf("    \x1b[36mAvg ec_eval_even2:                %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_eval_even2 / repeat);
  printf("    \x1b[36mAvg ec_dlog_3_tate_R:             %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_dlog_3_tate_R / repeat);
  printf("    \x1b[36mAvg ec_biscalar_mul_ibz3:         %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_biscalar3 / repeat);
  printf("    \x1b[36mAvg ec_eval_three3:               %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_eval_three3 / repeat);
  printf("    \x1b[36mAvg ec_dlog_2_tate_R:             %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_dlog_2_tate_R / repeat);
  printf("    \x1b[36mAvg ec_biscalar_mul_ibz4:         %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_biscalar4 / repeat);
  printf("    \x1b[36mAvg ec_eval_even3:                %.5f ms\x1b[0m\n",
         kg_timings_total.ms_dp_eval_even3 / repeat);
  printf("  \x1b[33mAvg ec_curve_to_basis_2f_hint:   %.5f ms\x1b[0m\n",
         ms_basis_two_hint_total / repeat);
  // printf("    \x1b[36mAvg ec_eval_even  (in doublepath): %.5f ms\x1b[0m\n",
  //        ms_eval_even_total / repeat);
  // printf("    \x1b[36mAvg ec_eval_three (in doublepath): %.5f ms\x1b[0m\n",
  //        ms_eval_three_total / repeat);
  printf("  \x1b[33mAvg change_basis_two:            %.5f ms\x1b[0m\n",
         ms_change_basis_two_total / repeat);
  printf("  \x1b[33mAvg ec_curve_to_basis_3f_hint:   %.5f ms\x1b[0m\n",
         ms_basis_three_hint_total / repeat);
  printf("  \x1b[33mAvg change_basis_three:          %.5f ms\x1b[0m\n",
         ms_change_basis_three_total / repeat);
  // printf("    \x1b[36mAvg total isog length (even):      %.5f\x1b[0m\n",
  //        (float)total_isog_length_even_kg / (float)repeat);
  // printf("    \x1b[36mAvg total isog length (three):     %.5f\x1b[0m\n",
  //        (float)total_isog_length_three_kg / (float)repeat);
#endif

  printf("average signing time [%.5f ms]\n", (float)(ms_sign / repeat));
  print_time_stats("signing", ms_sig_samples, repeat, mcycles_per_ms);
  const double avg_sign = (double)ms_sign / (double)repeat;
  const double sign_isog =
      (double)(sg_timings_total.ms_fc_eval_even1 +
               sg_timings_total.ms_fc_eval_three1 +
               sg_timings_total.ms_fc_eval_three2 +
               sg_timings_total.ms_fc_eval_even2 +
               sg_timings_total.ms_fc_eval_three3 +
               sg_timings_total.ms_challenge_eval_three +
               sg_timings_total.ms_cri_eval_even_delta +
               sg_timings_total.ms_cri_eval_three_first +
               sg_timings_total.ms_cri_theta_chain +
               sg_timings_total.ms_cri_eval_three_final +
               sg_timings_total.ms_pri_theta_chain +
               sg_timings_total.ms_pri_eval_three) /
      (double)repeat;
  const double sign_ec_non_isog =
      (double)(sg_timings_total.ms_fc_biscalar1 +
               sg_timings_total.ms_fc_complete_three_basis +
               sg_timings_total.ms_fc_quat_to_ec +
               sg_timings_total.ms_fc_curve_to_basis_3 +
               sg_timings_total.ms_fc_biscalar2 +
               sg_timings_total.ms_fc_dlog_3_tate_R +
               sg_timings_total.ms_fc_biscalar3 +
               sg_timings_total.ms_challenge_biscalar +
               sg_timings_total.ms_response_matrix_apply +
               sg_timings_total.ms_response_basis_hint +
               sg_timings_total.ms_response_change_basis +
               sg_timings_total.ms_cri_basis_even_doubles +
               sg_timings_total.ms_cri_matrix_application +
               sg_timings_total.ms_cri_kernel_three +
               sg_timings_total.ms_cri_kernel_two +
               sg_timings_total.ms_cri_kernel_two_doubles +
               sg_timings_total.ms_cri_basis_three +
               sg_timings_total.ms_cri_copy_kernel +
               sg_timings_total.ms_cri_dlog_3_tate_R +
               sg_timings_total.ms_cri_biscalar_final +
               sg_timings_total.ms_pri_delta_doubles +
               sg_timings_total.ms_pri_copy_kernel +
               sg_timings_total.ms_pri_weil +
               sg_timings_total.ms_pri_select_codomain +
               sg_timings_total.ms_aux_odd_adjust +
               sg_timings_total.ms_aux_basis_hint +
               sg_timings_total.ms_aux_change_basis) /
      (double)repeat;
  const double sign_quat =
      (double)(sg_timings_total.ms_fc_represent_integer +
               sg_timings_total.ms_fc_lideal_create +
               sg_timings_total.ms_fc_quat_to_quat +
               sg_timings_total.ms_challenge_ideal +
               sg_timings_total.ms_challenge_lideal_inter +
               sg_timings_total.ms_challenge_lideal_mul +
               sg_timings_total.ms_generator_coprime +
               sg_timings_total.ms_lideal_mul_tmp +
               sg_timings_total.ms_lideal_isom +
               sg_timings_total.ms_lideal_conjugate_lattice +
               sg_timings_total.ms_lattice_intersect +
               sg_timings_total.ms_sample_response +
               sg_timings_total.ms_response_quat_mul +
               sg_timings_total.ms_response_matrix +
               sg_timings_total.ms_aux_norm +
               sg_timings_total.ms_cri_represent_integer +
               sg_timings_total.ms_cri_endomorphism_matrix) /
      (double)repeat;
  double sign_other = avg_sign - sign_isog - sign_ec_non_isog - sign_quat;
  if (sign_other < 0.0) {
    sign_other = 0.0;
  }
  printf("  \x1b[35mSigning category ratios (denominator: avg signing)\x1b[0m\n");
  print_category_ratio("Isogeny computations:", sign_isog, avg_sign);
  print_category_ratio("EC/basis except isogeny:", sign_ec_non_isog, avg_sign);
  print_category_ratio("Quaternion algorithms:", sign_quat, avg_sign);
  print_category_ratio("Other:", sign_other, avg_sign);
#if 0
  printf("  \x1b[33mAvg commit:                %.5f ms\x1b[0m\n",
         sg_timings_total.ms_commit / repeat);
  printf("    \x1b[36mAvg commit isogeny:      %.5f ms\x1b[0m\n",
         sg_timings_total.ms_commit_isog / repeat);
  printf("    \x1b[36mAvg represent_integer:          %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_represent_integer / repeat);
  printf("    \x1b[36mAvg quat_lideal_create_from_primitive: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_lideal_create / repeat);
  printf("    \x1b[36mAvg quat_to_isog_power_of_two:  %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_quat_to_isog_two / repeat);
  printf("    \x1b[36mAvg quat_to_kernel_power_of_three: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_quat_to_kernel_three / repeat);
  printf("    \x1b[36mAvg quat_to_isog_power_of_three: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_quat_to_isog_three / repeat);
  printf("    \x1b[36mAvg quat_to_kernel_power_of_two: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_quat_to_kernel_two / repeat);
  printf("    \x1b[36m  quat_to_* split, quat:         %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_quat_to_quat / repeat);
  printf("    \x1b[36m  quat_to_* split, ec:           %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_quat_to_ec / repeat);
  printf("    \x1b[36m  quat_to_* split, other:        %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_quat_to_other / repeat);
  printf("    \x1b[36mAvg ec_biscalar_mul_ibz1:       %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_biscalar1 / repeat);
  printf("    \x1b[36mAvg complete_three_basis:       %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_complete_three_basis / repeat);
  printf("    \x1b[36mAvg ec_eval_even1:              %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_eval_even1 / repeat);
  printf("    \x1b[36mAvg ec_eval_three1:             %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_eval_three1 / repeat);
  printf("    \x1b[36mAvg ec_eval_three2:             %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_eval_three2 / repeat);
  printf("    \x1b[36mAvg ec_curve_to_basis_3f_hint:  %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_curve_to_basis_3 / repeat);
  printf("    \x1b[36mAvg ec_biscalar_mul_ibz2:       %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_biscalar2 / repeat);
  printf("    \x1b[36mAvg ec_eval_even2:              %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_eval_even2 / repeat);
  printf("    \x1b[36mAvg ec_dlog_3_tate_R:           %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_dlog_3_tate_R / repeat);
  printf("    \x1b[36mAvg ec_biscalar_mul_ibz3:       %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_biscalar3 / repeat);
  printf("    \x1b[36mAvg ec_eval_three3:             %.5f ms\x1b[0m\n",
         sg_timings_total.ms_fc_eval_three3 / repeat);
  printf("  \x1b[33mAvg challenge:             %.5f ms\x1b[0m\n",
         sg_timings_total.ms_challenge / repeat);
  printf("    \x1b[36mAvg hash_to_challenge:          %.5f ms\x1b[0m\n",
         sg_timings_total.ms_hash_to_challenge / repeat);
  printf("    \x1b[36mAvg ec_biscalar_mul_ibz:        %.5f ms\x1b[0m\n",
         sg_timings_total.ms_challenge_biscalar / repeat);
  printf("    \x1b[36mAvg ec_eval_three:              %.5f ms\x1b[0m\n",
         sg_timings_total.ms_challenge_eval_three / repeat);
  printf("    \x1b[36mAvg id2iso_kernel_dlogs_to_ideal_three: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_challenge_ideal / repeat);
  printf("    \x1b[36mAvg quat_lideal_inter:          %.5f ms\x1b[0m\n",
         sg_timings_total.ms_challenge_lideal_inter / repeat);
  printf("    \x1b[36mAvg quat_lideal_mul:            %.5f ms\x1b[0m\n",
         sg_timings_total.ms_challenge_lideal_mul / repeat);
  printf("    \x1b[36mAvg quat_lideal_generator_coprime: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_generator_coprime / repeat);
  printf("    \x1b[36mAvg quat_lideal_mul tmp:        %.5f ms\x1b[0m\n",
         sg_timings_total.ms_lideal_mul_tmp / repeat);
  printf("    \x1b[36mAvg quat_lideal_isom:           %.5f ms\x1b[0m\n",
         sg_timings_total.ms_lideal_isom / repeat);
  printf("    \x1b[36mAvg quat_lideal_conjugate_lattice: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_lideal_conjugate_lattice / repeat);
  printf("    \x1b[36mAvg quat_lattice_intersect:     %.5f ms\x1b[0m\n",
         sg_timings_total.ms_lattice_intersect / repeat);
  printf("    \x1b[36mAvg sample_response:            %.5f ms\x1b[0m\n",
         sg_timings_total.ms_sample_response / repeat);
  printf("    \x1b[36mAvg sample_response trials:     %.5f\x1b[0m\n",
         (float)sg_timings_total.total_sample_response_trials /
             (float)repeat);
  printf("  \x1b[33mAvg response:              %.5f ms\x1b[0m\n",
         sg_timings_total.ms_response / repeat);
  printf("    \x1b[36mAvg quat_alg_mul:               %.5f ms\x1b[0m\n",
         sg_timings_total.ms_response_quat_mul / repeat);
  printf("    \x1b[36mAvg matrix_of_endomorphism_even: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_response_matrix / repeat);
  printf("    \x1b[36mAvg matrix_application_even_basis: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_response_matrix_apply / repeat);
  printf("    \x1b[36mAvg ec_curve_to_basis_2f_to_hint: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_response_basis_hint / repeat);
  printf("    \x1b[36mAvg change_of_basis_matrix_two: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_response_change_basis / repeat);
  printf("    \x1b[36mAvg aux (in response):          %.5f ms\x1b[0m\n",
         sg_timings_total.ms_aux / repeat);
  printf("      \x1b[36mAvg aux norm setup:           %.5f ms\x1b[0m\n",
         sg_timings_total.ms_aux_norm / repeat);
  printf("      \x1b[36mAvg pushrandisog:             %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pushrandisog / repeat);
  printf("        \x1b[36mAvg pushrandisog setup:     %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_setup / repeat);
  printf("        \x1b[36mAvg choose e/delta:         %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_choose_e / repeat);
  printf("        \x1b[36mAvg composed_rand_isog:     %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_composed_rand_isog / repeat);
  printf("          \x1b[36mAvg cri setup:            %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_setup / repeat);
  printf("          \x1b[36mAvg basis even doubles:   %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_basis_even_doubles / repeat);
  printf("          \x1b[36mAvg inv scalar:           %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_inv_scalar / repeat);
  printf("          \x1b[36mAvg represent_integer:    %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_represent_integer / repeat);
  printf("          \x1b[36mAvg endomorphism matrix:  %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_endomorphism_matrix / repeat);
  printf("          \x1b[36mAvg matrix_application_even_basis: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_matrix_application / repeat);
  printf("          \x1b[36mAvg K3 kernel:            %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_kernel_three / repeat);
  printf("          \x1b[36mAvg K2 kernel:            %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_kernel_two / repeat);
  printf("          \x1b[36mAvg K2 kernel doubles:    %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_kernel_two_doubles / repeat);
  printf("          \x1b[36mAvg ec_eval_even delta:   %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_eval_even_delta / repeat);
  printf("          \x1b[36mAvg first ec_eval_three:  %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_eval_three_first / repeat);
  printf("          \x1b[36mAvg ec_curve_to_basis_3:  %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_basis_three / repeat);
  printf("          \x1b[36mAvg copy_bases_to_kernel: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_copy_kernel / repeat);
  printf("          \x1b[36mAvg theta setup:          %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_theta_setup / repeat);
  printf("          \x1b[36mAvg theta_chain_compute_and_eval: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_theta_chain / repeat);
  printf("          \x1b[36mAvg ec_dlog_3_tate_R:     %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_dlog_3_tate_R / repeat);
  printf("          \x1b[36mAvg final ec_biscalar_mul_ibz: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_biscalar_final / repeat);
  printf("          \x1b[36mAvg final ec_eval_three:  %.5f ms\x1b[0m\n",
         sg_timings_total.ms_cri_eval_three_final / repeat);
  printf("        \x1b[36mAvg delta doublings:        %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_delta_doubles / repeat);
  printf("        \x1b[36mAvg copy_bases_to_kernel:   %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_copy_kernel / repeat);
  printf("        \x1b[36mAvg theta setup:            %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_theta_setup / repeat);
  printf("        \x1b[36mAvg theta_chain_compute_and_eval: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_theta_chain / repeat);
  printf("        \x1b[36mAvg Weil pairing check:     %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_weil / repeat);
  printf("        \x1b[36mAvg select codomain:        %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_select_codomain / repeat);
  printf("        \x1b[36mAvg final ec_eval_three:    %.5f ms\x1b[0m\n",
         sg_timings_total.ms_pri_eval_three / repeat);
  printf("      \x1b[36mAvg aux odd adjustment:       %.5f ms\x1b[0m\n",
         sg_timings_total.ms_aux_odd_adjust / repeat);
  printf("      \x1b[36mAvg ec_curve_to_basis_2f_to_hint: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_aux_basis_hint / repeat);
  printf("      \x1b[36mAvg change_of_basis_matrix_two: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_aux_change_basis / repeat);
  printf("      \x1b[36mAvg response matrix finalize: %.5f ms\x1b[0m\n",
         sg_timings_total.ms_aux_matrix_finalize / repeat);
#endif

  printf("average verification time [%.5f ms]\n", (float)(ms_verify / repeat));
  print_time_stats("verify", ms_ver_samples, repeat, mcycles_per_ms);
  const double avg_verify = (double)ms_verify / (double)repeat;
  const double verify_isog =
      (double)(vr_timings_total.ms_eval_three_prev +
               vr_timings_total.ms_eval_even +
               vr_timings_total.ms_theta_chain_verify) /
      (double)repeat;
  const double verify_ec_non_isog =
      (double)(vr_timings_total.ms_basis_three_hint +
               vr_timings_total.ms_challenge_biscalar +
               vr_timings_total.ms_three_isogenous_coeff_to_kernel +
               vr_timings_total.ms_challenge_basis_hint +
               vr_timings_total.ms_matrix_application +
               vr_timings_total.ms_aux_basis_hint +
               vr_timings_total.ms_aux_doubles +
               vr_timings_total.ms_copy_bases_to_kernel) /
      (double)repeat;
  const double verify_quat = 0.0;
  double verify_other =
      avg_verify - verify_isog - verify_ec_non_isog - verify_quat;
  if (verify_other < 0.0) {
    verify_other = 0.0;
  }
  printf("  \x1b[35mVerify category ratios (denominator: avg verify)\x1b[0m\n");
  print_category_ratio("Isogeny computations:", verify_isog, avg_verify);
  print_category_ratio("EC/basis except isogeny:", verify_ec_non_isog,
                       avg_verify);
  print_category_ratio("Quaternion algorithms:", verify_quat, avg_verify);
  print_category_ratio("Other:", verify_other, avg_verify);
#if 0
  printf("  \x1b[33mAvg ec_curve_to_basis_3f_from_hint: %.5f ms\x1b[0m\n",
         vr_timings_total.ms_basis_three_hint / repeat);
  printf("  \x1b[33mAvg ec_biscalar_mul_ibz:            %.5f ms\x1b[0m\n",
         vr_timings_total.ms_challenge_biscalar / repeat);
  printf("  \x1b[33mAvg ec_eval_three_prev:             %.5f ms\x1b[0m\n",
         vr_timings_total.ms_eval_three_prev / repeat);
  printf("  \x1b[33mAvg three_isogenous_coeff_to_kernel: %.5f ms\x1b[0m\n",
         vr_timings_total.ms_three_isogenous_coeff_to_kernel / repeat);
  printf("  \x1b[33mAvg ec_curve_to_basis_2f_from_hint (challenge): %.5f ms\x1b[0m\n",
         vr_timings_total.ms_challenge_basis_hint / repeat);
  printf("  \x1b[33mAvg matrix_application_even_basis:  %.5f ms\x1b[0m\n",
         vr_timings_total.ms_matrix_application / repeat);
  printf("  \x1b[33mAvg ec_curve_to_basis_2f_from_hint (aux): %.5f ms\x1b[0m\n",
         vr_timings_total.ms_aux_basis_hint / repeat);
  printf("  \x1b[33mAvg kernel_iso2 setup:              %.5f ms\x1b[0m\n",
         vr_timings_total.ms_kernel_iso2 / repeat);
  printf("  \x1b[33mAvg ec_eval_even:                   %.5f ms\x1b[0m\n",
         vr_timings_total.ms_eval_even / repeat);
  printf("  \x1b[33mAvg aux/challenge doublings:        %.5f ms\x1b[0m\n",
         vr_timings_total.ms_aux_doubles / repeat);
  printf("  \x1b[33mAvg copy_bases_to_kernel:           %.5f ms\x1b[0m\n",
         vr_timings_total.ms_copy_bases_to_kernel / repeat);
  printf("  \x1b[33mAvg theta setup:                    %.5f ms\x1b[0m\n",
         vr_timings_total.ms_theta_setup / repeat);
  printf("  \x1b[33mAvg theta_chain_compute_and_eval_verify: %.5f ms\x1b[0m\n",
         vr_timings_total.ms_theta_chain_verify / repeat);
  printf("  \x1b[33mAvg hash_challenge:                 %.5f ms\x1b[0m\n",
         vr_timings_total.ms_hash_challenge / repeat);
  printf("  \x1b[33mAvg final check:                    %.5f ms\x1b[0m\n",
         vr_timings_total.ms_final_check / repeat);
  // printf("    \x1b[36mAvg total isog length (even):      %.5f\x1b[0m\n",
  //        (float)total_isog_length_even_vr / (float)repeat);
  // printf("    \x1b[36mAvg total isog length (three):     %.5f\x1b[0m\n",
  //        (float)total_isog_length_three_vr / (float)repeat);
  // printf("    \x1b[36mAvg total isog length (2,2):       %.5f\x1b[0m\n",
  //        (float)total_isog_length_two_two_vr / (float)repeat);
#endif

#if 0
  printf("\n--- Dlog Counts (average per run over %d repetitions) ---\n",
         repeat);
  printf("Keygen:\n");
  printf("  ec_dlog_2:        %.2f\n", (float)total_kg_dlog_2 / repeat);
  printf("  ec_dlog_2_tate_R: %.2f\n", (float)total_kg_dlog_2_tate_R / repeat);
  printf("  ec_dlog_2_tate:   %.2f\n", (float)total_kg_dlog_2_tate / repeat);
  printf("  ec_dlog_3:        %.2f\n", (float)total_kg_dlog_3 / repeat);
  printf("  ec_dlog_3_tate_R: %.2f\n", (float)total_kg_dlog_3_tate_R / repeat);
  printf("  ec_dlog_3_tate:   %.2f\n", (float)total_kg_dlog_3_tate / repeat);
  printf("Sign:\n");
  printf("  ec_dlog_2:        %.2f\n", (float)total_sg_dlog_2 / repeat);
  printf("  ec_dlog_2_tate_R: %.2f\n", (float)total_sg_dlog_2_tate_R / repeat);
  printf("  ec_dlog_2_tate:   %.2f\n", (float)total_sg_dlog_2_tate / repeat);
  printf("  ec_dlog_3:        %.2f\n", (float)total_sg_dlog_3 / repeat);
  printf("  ec_dlog_3_tate_R: %.2f\n", (float)total_sg_dlog_3_tate_R / repeat);
  printf("  ec_dlog_3_tate:   %.2f\n", (float)total_sg_dlog_3_tate / repeat);
  printf("Verify:\n");
  printf("  ec_dlog_2:        %.2f\n", (float)total_vr_dlog_2 / repeat);
  printf("  ec_dlog_2_tate_R: %.2f\n", (float)total_vr_dlog_2_tate_R / repeat);
  printf("  ec_dlog_2_tate:   %.2f\n", (float)total_vr_dlog_2_tate / repeat);
  printf("  ec_dlog_3:        %.2f\n", (float)total_vr_dlog_3 / repeat);
  printf("  ec_dlog_3_tate_R: %.2f\n", (float)total_vr_dlog_3_tate_R / repeat);
  printf("  ec_dlog_3_tate:   %.2f\n", (float)total_vr_dlog_3_tate / repeat);
  printf("--------------------------------------------------------\n");

  printf("\n--- GF(p^2) Operations in Tate Dlog Algorithms ---\n");

  // 1. ec_dlog_2_tate_R
  {
    uint64_t total_calls = total_kg_dlog_2_tate_R + total_sg_dlog_2_tate_R +
                           total_vr_dlog_2_tate_R;
    uint64_t total_add = kg_dlog_2_tate_R_fp2.add + sg_dlog_2_tate_R_fp2.add +
                         vr_dlog_2_tate_R_fp2.add;
    uint64_t total_sqr = kg_dlog_2_tate_R_fp2.sqr + sg_dlog_2_tate_R_fp2.sqr +
                         vr_dlog_2_tate_R_fp2.sqr;
    uint64_t total_mul = kg_dlog_2_tate_R_fp2.mul + sg_dlog_2_tate_R_fp2.mul +
                         vr_dlog_2_tate_R_fp2.mul;
    uint64_t total_inv = kg_dlog_2_tate_R_fp2.inv + sg_dlog_2_tate_R_fp2.inv +
                         vr_dlog_2_tate_R_fp2.inv;
    uint64_t total_sqrt = kg_dlog_2_tate_R_fp2.sqrt +
                          sg_dlog_2_tate_R_fp2.sqrt + vr_dlog_2_tate_R_fp2.sqrt;

    printf("ec_dlog_2_tate_R (called %" PRIu64 " times, %.2f per run):\n",
           total_calls, (float)total_calls / repeat);
    if (total_calls > 0) {
      printf("  add/sub: %.2f per call (%.2f per run)\n",
             (float)total_add / total_calls, (float)total_add / repeat);
      printf("  sqr:     %.2f per call (%.2f per run)\n",
             (float)total_sqr / total_calls, (float)total_sqr / repeat);
      printf("  mul:     %.2f per call (%.2f per run)\n",
             (float)total_mul / total_calls, (float)total_mul / repeat);
      printf("  inv:     %.2f per call (%.2f per run)\n",
             (float)total_inv / total_calls, (float)total_inv / repeat);
      printf("  sqrt:    %.2f per call (%.2f per run)\n",
             (float)total_sqrt / total_calls, (float)total_sqrt / repeat);
    } else {
      printf("  (not called)\n");
    }
  }

  // 2. ec_dlog_2_tate
  {
    uint64_t total_calls =
        total_kg_dlog_2_tate + total_sg_dlog_2_tate + total_vr_dlog_2_tate;
    uint64_t total_add = kg_dlog_2_tate_fp2.add + sg_dlog_2_tate_fp2.add +
                         vr_dlog_2_tate_fp2.add;
    uint64_t total_sqr = kg_dlog_2_tate_fp2.sqr + sg_dlog_2_tate_fp2.sqr +
                         vr_dlog_2_tate_fp2.sqr;
    uint64_t total_mul = kg_dlog_2_tate_fp2.mul + sg_dlog_2_tate_fp2.mul +
                         vr_dlog_2_tate_fp2.mul;
    uint64_t total_inv = kg_dlog_2_tate_fp2.inv + sg_dlog_2_tate_fp2.inv +
                         vr_dlog_2_tate_fp2.inv;
    uint64_t total_sqrt = kg_dlog_2_tate_fp2.sqrt + sg_dlog_2_tate_fp2.sqrt +
                          vr_dlog_2_tate_fp2.sqrt;

    printf("ec_dlog_2_tate (called %" PRIu64 " times, %.2f per run):\n",
           total_calls, (float)total_calls / repeat);
    if (total_calls > 0) {
      printf("  add/sub: %.2f per call (%.2f per run)\n",
             (float)total_add / total_calls, (float)total_add / repeat);
      printf("  sqr:     %.2f per call (%.2f per run)\n",
             (float)total_sqr / total_calls, (float)total_sqr / repeat);
      printf("  mul:     %.2f per call (%.2f per run)\n",
             (float)total_mul / total_calls, (float)total_mul / repeat);
      printf("  inv:     %.2f per call (%.2f per run)\n",
             (float)total_inv / total_calls, (float)total_inv / repeat);
      printf("  sqrt:    %.2f per call (%.2f per run)\n",
             (float)total_sqrt / total_calls, (float)total_sqrt / repeat);
    } else {
      printf("  (not called)\n");
    }
  }

  // 3. ec_dlog_3_tate_R
  {
    uint64_t total_calls = total_kg_dlog_3_tate_R + total_sg_dlog_3_tate_R +
                           total_vr_dlog_3_tate_R;
    uint64_t total_add = kg_dlog_3_tate_R_fp2.add + sg_dlog_3_tate_R_fp2.add +
                         vr_dlog_3_tate_R_fp2.add;
    uint64_t total_sqr = kg_dlog_3_tate_R_fp2.sqr + sg_dlog_3_tate_R_fp2.sqr +
                         vr_dlog_3_tate_R_fp2.sqr;
    uint64_t total_mul = kg_dlog_3_tate_R_fp2.mul + sg_dlog_3_tate_R_fp2.mul +
                         vr_dlog_3_tate_R_fp2.mul;
    uint64_t total_inv = kg_dlog_3_tate_R_fp2.inv + sg_dlog_3_tate_R_fp2.inv +
                         vr_dlog_3_tate_R_fp2.inv;
    uint64_t total_sqrt = kg_dlog_3_tate_R_fp2.sqrt +
                          sg_dlog_3_tate_R_fp2.sqrt + vr_dlog_3_tate_R_fp2.sqrt;

    printf("ec_dlog_3_tate_R (called %" PRIu64 " times, %.2f per run):\n",
           total_calls, (float)total_calls / repeat);
    if (total_calls > 0) {
      printf("  add/sub: %.2f per call (%.2f per run)\n",
             (float)total_add / total_calls, (float)total_add / repeat);
      printf("  sqr:     %.2f per call (%.2f per run)\n",
             (float)total_sqr / total_calls, (float)total_sqr / repeat);
      printf("  mul:     %.2f per call (%.2f per run)\n",
             (float)total_mul / total_calls, (float)total_mul / repeat);
      printf("  inv:     %.2f per call (%.2f per run)\n",
             (float)total_inv / total_calls, (float)total_inv / repeat);
      printf("  sqrt:    %.2f per call (%.2f per run)\n",
             (float)total_sqrt / total_calls, (float)total_sqrt / repeat);
    } else {
      printf("  (not called)\n");
    }
  }

  // 4. ec_dlog_3_tate
  {
    uint64_t total_calls =
        total_kg_dlog_3_tate + total_sg_dlog_3_tate + total_vr_dlog_3_tate;
    uint64_t total_add = kg_dlog_3_tate_fp2.add + sg_dlog_3_tate_fp2.add +
                         vr_dlog_3_tate_fp2.add;
    uint64_t total_sqr = kg_dlog_3_tate_fp2.sqr + sg_dlog_3_tate_fp2.sqr +
                         vr_dlog_3_tate_fp2.sqr;
    uint64_t total_mul = kg_dlog_3_tate_fp2.mul + sg_dlog_3_tate_fp2.mul +
                         vr_dlog_3_tate_fp2.mul;
    uint64_t total_inv = kg_dlog_3_tate_fp2.inv + sg_dlog_3_tate_fp2.inv +
                         vr_dlog_3_tate_fp2.inv;
    uint64_t total_sqrt = kg_dlog_3_tate_fp2.sqrt + sg_dlog_3_tate_fp2.sqrt +
                          vr_dlog_3_tate_fp2.sqrt;

    printf("ec_dlog_3_tate (called %" PRIu64 " times, %.2f per run):\n",
           total_calls, (float)total_calls / repeat);
    if (total_calls > 0) {
      printf("  add/sub: %.2f per call (%.2f per run)\n",
             (float)total_add / total_calls, (float)total_add / repeat);
      printf("  sqr:     %.2f per call (%.2f per run)\n",
             (float)total_sqr / total_calls, (float)total_sqr / repeat);
      printf("  mul:     %.2f per call (%.2f per run)\n",
             (float)total_mul / total_calls, (float)total_mul / repeat);
      printf("  inv:     %.2f per call (%.2f per run)\n",
             (float)total_inv / total_calls, (float)total_inv / repeat);
      printf("  sqrt:    %.2f per call (%.2f per run)\n",
             (float)total_sqrt / total_calls, (float)total_sqrt / repeat);
    } else {
      printf("  (not called)\n");
    }
  }
  printf("--------------------------------------------------\n");
#endif

cleanup:
  for (int i = 0; i < repeat; ++i) {
    secret_key_finalize(&sks[i]);
    secret_sig_finalize(&sigs[i]);
  }
  free(ms_kg_samples);
  free(ms_sig_samples);
  free(ms_ver_samples);
  free(pks);
  free(sks);
  free(sigs);

  return res;
}

// run all tests in module
int main() {
  int res = 1;

  randombytes_init((unsigned char *)"some", (unsigned char *)"string", 128);

  // printf("\nRunning encoding tests\n");
  // res &= test_encode();

  printf("\nRunning SQIsign2DPush tests\n \n");

  // printf("Format of printed data:\n\n");
  // printf("A_EA = Montgomery coefficient of public key,\n");
  // printf("xP3A, xQ3A, and xP3AmQ3A = cannonical differential basis of the "
  //        "power-of-3 torsion of the public key, \n");
  // printf("ker_phi_vect[0] and ker_phi_vect[1] = coefficients defining the "
  //        "challenge, \n");
  // printf("xPA, xQA, xPAmQA = cannonical differential basis of the power-of-2
  // "
  //        "torsion of the public key, \n");
  // printf("A_E1 = Montgomery coefficient of commitment, \n");
  // printf("xP1, xQ1, xP1mQ1 = cannonical differential basis of the power-of-2
  // "
  //        "torsion of the commitment, \n");
  // printf("j_EA, j_E1 = j-invariants of the public key and commitment, \n");
  // printf("M_sigma[ij] = matrix defining the response isogeny with respect to
  // "
  //        "the cannonical bases of the power-of-2 torsion\n\n");

  res &= test_sqisign(100);

  if (!res) {
    printf("\nSome tests failed!\n");
  } else {
    printf("All tests passed!\n");
  }
  return (!res);
}
