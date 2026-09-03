#include <curve_extras.h>
#include <ec.h>
#include <fips202.h>
#include <fp2.h>
#include <hd.h>
#include <isog.h>
#include <sqisign2dpush.h>
#include <stdio.h>
#include <string.h>
#include <tedwards.h>
#include <time.h>
#include <toolbox.h>

#define EXPONENT_TWO TORSION_PLUS_EVEN_POWER
#define EXPONENT_THREE TORSION_PLUS_ODD_POWERS[0]

#define VERIFY_TIME_FIELD(field, ...)                                          \
  do {                                                                         \
    if (timings) {                                                             \
      clock_t _ts = clock();                                                   \
      __VA_ARGS__;                                                             \
      timings->field +=                                                        \
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;             \
    } else {                                                                   \
      __VA_ARGS__;                                                             \
    }                                                                          \
  } while (0)

static void fp2_print(char *name, fp2_t const a) {
  fp2_t b;
  // fp2_frommont(&b, a);
  fp2_set(&b, 1);
  fp2_mul(&b, &b, &a);
  printf("%s0x", name);
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016llx", (unsigned long long)b.re[i]);
  printf(" + i*0x");
  for (int i = NWORDS_FIELD - 1; i >= 0; i--)
    printf("%016llx", (unsigned long long)b.im[i]);
  printf(", ");
}

static void point_print(char *name, ec_point_t P) {
  fp2_t a;
  if (fp2_is_zero(&P.z)) {
    printf("%s = INF, ", name);
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

void three_isogenous_coeff_to_kernel(ec_point_t *kernel,
                                     const ec_curve_t *curve,
                                     const ec_curve_t *curve_prev) {
  fp2_t t0, t1, t2, t3, t4, t5;
  const fp2_t *AX = &curve->A;
  const fp2_t *AZ = &curve->C;
  const fp2_t *AdX = &curve_prev->A;
  const fp2_t *AdZ = &curve_prev->C;

  // X = AX^3*AdZ^2 + 18*AX^2*AdX*AZ*AdZ - 3*AX*AdX^2*AZ^2 + 48*AX*AZ^2*AdZ^2 -
  // 112*AdX*AZ^3*AdZ
  fp2_sqr(&t0, AX);              // AX^2
  fp2_mul(&t1, &t0, AX);         // AX^3
  fp2_sqr(&t2, AdZ);             // AdZ^2
  fp2_mul(&kernel->x, &t1, &t2); // X = AX^3 * AdZ^2

  fp2_mul(&t1, &t0, AdX); // AX^2 * AdX
  fp2_mul(&t1, &t1, AZ);  // AX^2 * AdX * AZ
  fp2_mul(&t1, &t1, AdZ); // AX^2 * AdX * AZ * AdZ
  fp2_add(&t3, &t1, &t1); // 2
  fp2_add(&t4, &t3, &t3); // 4
  fp2_add(&t4, &t4, &t4); // 8
  fp2_add(&t4, &t4, &t4); // 16
  fp2_add(&t4, &t4, &t3); // 18
  fp2_add(&kernel->x, &kernel->x, &t4);

  fp2_sqr(&t1, AdX);      // AdX^2
  fp2_sqr(&t3, AZ);       // AZ^2
  fp2_mul(&t4, &t1, &t3); // AdX^2 * AZ^2
  fp2_mul(&t4, &t4, AX);  // AX * AdX^2 * AZ^2
  fp2_add(&t5, &t4, &t4); // 2
  fp2_add(&t5, &t5, &t4); // 3
  fp2_sub(&kernel->x, &kernel->x, &t5);

  fp2_mul(&t4, AX, &t3);  // AX * AZ^2
  fp2_mul(&t4, &t4, &t2); // AX * AZ^2 * AdZ^2
  fp2_add(&t5, &t4, &t4); // 2
  fp2_add(&t5, &t5, &t5); // 4
  fp2_add(&t5, &t5, &t5); // 8
  fp2_add(&t5, &t5, &t5); // 16
  fp2_copy(&t0, &t5);     // 16
  fp2_add(&t5, &t5, &t5); // 32
  fp2_add(&t5, &t5, &t0); // 48
  fp2_add(&kernel->x, &kernel->x, &t5);

  fp2_mul(&t4, AdX, &t3); // AdX * AZ^2
  fp2_mul(&t4, &t4, AZ);  // AdX * AZ^3
  fp2_mul(&t4, &t4, AdZ); // AdX * AZ^3 * AdZ
  fp2_add(&t5, &t4, &t4); // 2
  fp2_add(&t5, &t5, &t5); // 4
  fp2_add(&t5, &t5, &t5); // 8
  fp2_add(&t5, &t5, &t5); // 16
  fp2_copy(&t0, &t5);     // 16
  fp2_add(&t5, &t5, &t5); // 32
  fp2_copy(&t1, &t5);     // 32
  fp2_add(&t5, &t5, &t5); // 64
  fp2_add(&t5, &t5, &t1); // 96
  fp2_add(&t5, &t5, &t0); // 112
  fp2_sub(&kernel->x, &kernel->x, &t5);

  // Z = 4*AX^3*AdX*AdZ + 114*AX^2*AZ*AdZ^2 + 12*AX*AdX*AZ^2*AdZ + 2*AdX^2*AZ^3
  // - 576*AZ^3*AdZ^2
  fp2_sqr(&t0, AX);                            // AX^2
  fp2_mul(&t1, &t0, AX);                       // AX^3
  fp2_mul(&t1, &t1, AdX);                      // AX^3 * AdX
  fp2_mul(&t1, &t1, AdZ);                      // AX^3 * AdX * AdZ
  fp2_add(&kernel->z, &t1, &t1);               // 2
  fp2_add(&kernel->z, &kernel->z, &kernel->z); // 4

  fp2_mul(&t1, &t0, AZ);  // AX^2 * AZ
  fp2_mul(&t1, &t1, &t2); // AX^2 * AZ * AdZ^2 (t2 = AdZ^2)
  fp2_add(&t3, &t1, &t1); // 2
  fp2_copy(&t4, &t3);     // 2
  fp2_add(&t3, &t3, &t3); // 4
  fp2_add(&t3, &t3, &t3); // 8
  fp2_add(&t3, &t3, &t3); // 16
  fp2_copy(&t5, &t3);     // 16
  fp2_add(&t3, &t3, &t3); // 32
  fp2_copy(&t0, &t3);     // 32
  fp2_add(&t3, &t3, &t3); // 64
  fp2_add(&t3, &t3, &t0); // 96
  fp2_add(&t3, &t3, &t5); // 112
  fp2_add(&t3, &t3, &t4); // 114
  fp2_add(&kernel->z, &kernel->z, &t3);

  fp2_sqr(&t0, AZ);       // AZ^2
  fp2_mul(&t1, AX, AdX);  // AX * AdX
  fp2_mul(&t1, &t1, &t0); // AX * AdX * AZ^2
  fp2_mul(&t1, &t1, AdZ); // AX * AdX * AZ^2 * AdZ
  fp2_add(&t3, &t1, &t1); // 2
  fp2_add(&t3, &t3, &t3); // 4
  fp2_copy(&t4, &t3);     // 4
  fp2_add(&t3, &t3, &t3); // 8
  fp2_add(&t3, &t3, &t4); // 12
  fp2_add(&kernel->z, &kernel->z, &t3);

  fp2_sqr(&t1, AdX);      // AdX^2
  fp2_mul(&t3, &t0, AZ);  // AZ^3 (t0 = AZ^2)
  fp2_mul(&t1, &t1, &t3); // AdX^2 * AZ^3
  fp2_add(&t4, &t1, &t1); // 2
  fp2_add(&kernel->z, &kernel->z, &t4);

  fp2_mul(&t1, &t3, &t2); // AZ^3 * AdZ^2 (t2 = AdZ^2)
  fp2_add(&t3, &t1, &t1); // 2
  fp2_add(&t3, &t3, &t3); // 4
  fp2_add(&t3, &t3, &t3); // 8
  fp2_add(&t3, &t3, &t3); // 16
  fp2_add(&t3, &t3, &t3); // 32
  fp2_add(&t3, &t3, &t3); // 64
  fp2_copy(&t4, &t3);     // 64
  fp2_add(&t3, &t3, &t3); // 128
  fp2_add(&t3, &t3, &t3); // 256
  fp2_add(&t3, &t3, &t3); // 512
  fp2_add(&t3, &t3, &t4); // 576
  fp2_sub(&kernel->z, &kernel->z, &t3);
}

// int point_order_2f(const ec_point_t *P, const ec_curve_t *E, int t) {
//   ec_point_t test;
//   copy_point(&test, P);
//   if (fp2_is_zero(&test.z))
//     return 0;
//   for (int i = 0; i < t - 1; i++) {
//     ec_dbl(&test, E, &test);
//   }
//   if (fp2_is_zero(&test.z))
//     return 0;
//   ec_dbl(&test, E, &test);
//   return (fp2_is_zero(&test.z));
// }

// int point_order_3f(const ec_point_t *P, const ec_curve_t *E, int t) {
//   ec_point_t test;
//   copy_point(&test, P);
//   digit_t three[NWORDS_ORDER] = {0};
//   three[0] = 3;
//   if (fp2_is_zero(&test.z))
//     return 0;
//   for (int i = 0; i < t - 1; i++) {
//     ec_mul(&test, E, three, &test);
//   }
//   if (fp2_is_zero(&test.z))
//     return 0;
//   ec_mul(&test, E, three, &test);
//   return (fp2_is_zero(&test.z));
// }

// void isog_init_two_f(ec_isog_even_t *isog, const ec_curve_t *curve,
//                      const ec_point_t *ker, int length) {
//   copy_curve(&(isog->curve), curve);
//   copy_point(&(isog->kernel), ker);
//   assert(point_order_2f(ker, curve, length));
//   isog->length = length;
//   return;
// }

// void isog_init_3(ec_isog_odd_t *isog, const ec_curve_t *curve,
//                  const ec_point_t *ker) {
//   copy_curve(&(isog->curve), curve);
//   copy_point(&(isog->ker_plus), ker);
//   ec_set_zero(&(isog->ker_minus));
//   assert(point_order_3f(ker, curve, 1));

// #define NUMPP                                                                  \
//   (sizeof(TORSION_ODD_PRIMEPOWERS) / sizeof(*TORSION_ODD_PRIMEPOWERS))
//   for (size_t i = 0; i < NUMPP; ++i) {
//     (isog->degree)[i] = 0;
//     if (TORSION_ODD_PRIMES[i] == 3) {
//       (isog->degree)[i] = 1;
//     }
//   }
//   return;
// }

void hash_challenge(ibz_t *hash, const ec_curve_t *curve,
                    const unsigned char *m, const public_key_t *pk,
                    size_t length) {
  ibz_t pow3;
  ibz_init(&pow3);
  ibz_pow(&pow3, &ibz_const_three, EXPONENT_THREE);
  unsigned char *buf = malloc(sizeof(fp2_t) + sizeof(fp2_t) + length);
  {
    fp2_t j1, j2;
    ec_j_inv(&j1, curve);
    ec_j_inv(&j2, &pk->curve);
    memcpy(buf, &j1, sizeof(j1));
    memcpy(buf + sizeof(j1), &j2, sizeof(j2));
    memcpy(buf + sizeof(j1) + sizeof(j2), m, length);
  }

  {
    digit_t digits[NWORDS_FIELD] = {0};
    // SHAKE256((void *)digits, sizeof(digits), buf,
    //          sizeof(fp2_t) + sizeof(fp2_t) + length);
    sha3_256((void *)digits, buf, sizeof(fp2_t) + sizeof(fp2_t) + length);

    for (int i = 2; i < HASH_ITERATIONS; i++) {
      // SHAKE256((void *)digits, sizeof(digits), (void *)digits,
      // sizeof(digits));
      sha3_256((void *)digits, (void *)digits, sizeof(digits));
    }

    ibz_set(hash, 1);
    ibz_copy_digit_array(hash, digits);
    ibz_mod(hash, hash, &pow3);
  }

  ibz_finalize(&pow3);
  free(buf);
}

int protocols_verify(signature_t *sig, const public_key_t *pk,
                     const unsigned char *m, size_t l,
                     verify_timings_t *timings) {
  // extern uint64_t fp2_add_count;
  // extern uint64_t fp2_mul_count;
  // extern uint64_t fp2_inv_count;
  // extern uint64_t fp2_sqr_count;
  // extern uint64_t fp2_sqrt_count;

  // uint64_t start_add = fp2_add_count;
  // uint64_t start_mul = fp2_mul_count;
  // uint64_t start_sqr = fp2_sqr_count;
  // uint64_t start_inv = fp2_inv_count;
  // uint64_t start_sqrt = fp2_sqrt_count;

  if (timings) {
    timings->ms_eval_three = 0.f;
    timings->ms_basis_three_hint = 0.f;
    timings->ms_challenge_biscalar = 0.f;
    timings->ms_eval_three_prev = 0.f;
    timings->ms_three_isogenous_coeff_to_kernel = 0.f;
    timings->ms_challenge_basis_hint = 0.f;
    timings->ms_matrix_application = 0.f;
    timings->ms_aux_basis_hint = 0.f;
    timings->ms_kernel_iso2 = 0.f;
    timings->ms_eval_even = 0.f;
    timings->ms_aux_doubles = 0.f;
    timings->ms_copy_bases_to_kernel = 0.f;
    timings->ms_theta_setup = 0.f;
    timings->ms_theta_chain_verify = 0.f;
    timings->ms_hash_challenge = 0.f;
    timings->ms_final_check = 0.f;
    timings->total_isog_length_three = 0UL;
    timings->total_isog_length_even = 0UL;
    timings->total_isog_length_two_two = 0UL;
  }

  int verify;

  ec_basis_t Bpk_verify;
  int ok;
  VERIFY_TIME_FIELD(ms_basis_three_hint,
                    ok = ec_curve_to_basis_3f_from_hint(
                        &Bpk_verify, &pk->curve, pk->hint_pk_three));
  assert(ok);
  // ec_curve_to_basis_3(&Bpk_verify, &(pk->curve));

  ec_point_t kernel_chl_verify, Kbt;

  ibz_vec_2_t vec_can;
  ibz_vec_2_init(&vec_can);

  ibz_set(&vec_can[0], 1);
  ibz_copy_digit_array(&(vec_can[1]), sig->challenge);
  VERIFY_TIME_FIELD(ms_challenge_biscalar,
                    ec_biscalar_mul_ibz(&kernel_chl_verify, &pk->curve,
                                        &vec_can[0], &vec_can[1],
                                        &Bpk_verify));
  // complete_three_basis(&kernel_dual_chl_verify, &vec_can, &Bpk_verify);

  ec_isog_odd_t iso_chl_verify;
  isog_init_three(&iso_chl_verify, &pk->curve, &kernel_chl_verify,
                  EXPONENT_THREE);
  // ec_curve_t E_chl_verify;
  // ec_eval_three(&E_chl_verify, &iso_chl_verify, &kernel_dual_chl_verify, 1);
  // fp2_print("E_chl_verify.A: ", E_chl_verify.A);
  // printf("\n");
  // fp2_print("E_chl_verify.C: ", E_chl_verify.C);
  // printf("\n");
  // curve_print("E_chl_verify: ", E_chl_verify);
  // printf("\n");
  // point_print("kernel_dual_chl_verify: ", kernel_dual_chl_verify);
  // printf("\n");

  ec_curve_t E_chl_verify, E_chl_prev_verify;
  {
    clock_t _ts = clock();
    // isog_measure_reset();
    ec_eval_three_prev(&E_chl_verify, &E_chl_prev_verify, &iso_chl_verify);
    // printf("ec_eval_three_prev call 1: TPL=%lu, ISOG3=%lu, EVAL3=%lu\n",
    //        global_isog_counters.tpl_count, global_isog_counters.isog3_count,
    //        global_isog_counters.eval3_count);
    if (timings) {
      timings->ms_eval_three +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_eval_three_prev +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->total_isog_length_three += (unsigned long)EXPONENT_THREE;
    }
  }
  // point_print("E_chl_verify.A24: ", E_chl_verify.A24);
  // point_print("E_chl_prev_verify.A24: ", E_chl_prev_verify.A24);

  VERIFY_TIME_FIELD(ms_three_isogenous_coeff_to_kernel,
                    three_isogenous_coeff_to_kernel(&Kbt, &E_chl_verify,
                                                    &E_chl_prev_verify));
#ifdef DEBUG
  assert(point_order_3f(&Kbt, &E_chl_verify, 1));
#endif

  // digit_t three[NWORDS_ORDER] = {0};
  // three[0] = 3;
  // ibz_to_digits(three, &ibz_const_three);
  // for (int i = 0; i < (int)EXPONENT_THREE - 1; i++) {
  //   ec_mul(&kernel_dual_chl_verify, &E_chl_verify, three,
  //          &kernel_dual_chl_verify);
  // }
  // point_print("kernel_dual_chl_verify: ", kernel_dual_chl_verify);
  // printf("\n");

  ec_basis_t Bchl_verify;
  VERIFY_TIME_FIELD(ms_challenge_basis_hint,
                    ok = ec_curve_to_basis_2f_from_hint(
                        &Bchl_verify, &E_chl_verify, TORSION_PLUS_EVEN_POWER,
                        sig->hint_chall));
  assert(ok);
  // ec_curve_to_basis_2(&Bchl_verify, &E_chl_verify);
  VERIFY_TIME_FIELD(ms_matrix_application,
                    matrix_application_even_basis(&Bchl_verify, &E_chl_verify,
                                                  &sig->mat_rsp, EXPONENT_TWO));
  ec_basis_t Baux_verify;
  VERIFY_TIME_FIELD(ms_aux_basis_hint,
                    ok = ec_curve_to_basis_2f_from_hint(
                        &Baux_verify, &sig->E_aux, TORSION_PLUS_EVEN_POWER,
                        sig->hint_aux));
  assert(ok);
  // ec_curve_to_basis_2(&Baux_verify, &(sig->E_aux));

  ec_point_t kernel_iso2_verify;

  if (sig->n1 > 0) {
    VERIFY_TIME_FIELD(ms_kernel_iso2, {
      ibz_t gcd_verify;
      ibz_init(&gcd_verify);
      ibz_gcd(&gcd_verify, &sig->mat_rsp[0][0], &ibz_const_two);
      int check1 = ibz_is_one(&gcd_verify);
      ibz_gcd(&gcd_verify, &sig->mat_rsp[1][0], &ibz_const_two);
      int check2 = ibz_is_one(&gcd_verify);
      if (check1 || check2) {
        copy_point(&kernel_iso2_verify, &Bchl_verify.P);
      } else {
        copy_point(&kernel_iso2_verify, &Bchl_verify.Q);
      }
      ibz_finalize(&gcd_verify);
    });

    VERIFY_TIME_FIELD(ms_aux_doubles, {
      for (int i = 0; i < (int)(EXPONENT_TWO - sig->n1); i++) {
        ec_dbl(&kernel_iso2_verify, &E_chl_verify, &kernel_iso2_verify);
      }
    });

    ec_isog_even_t iso2_verify;
    // curve_print("E_chl_verify: ", E_chl_verify);
    // printf("\n");
    // point_print("kernel_iso2_verify: ", kernel_iso2_verify);
    // printf("\n");
    isog_init_two_f(&iso2_verify, &E_chl_verify, &kernel_iso2_verify, sig->n1);
    ec_curve_t E_chl2_verify;

    ec_point_t pts_verify[4];
    copy_point(&pts_verify[0], &Bchl_verify.P);
    copy_point(&pts_verify[1], &Bchl_verify.Q);
    copy_point(&pts_verify[2], &Bchl_verify.PmQ);
    copy_point(&pts_verify[3], &Kbt);
    // isog_measure_reset();
    {
      clock_t _ts = clock();
      ec_eval_even(&E_chl2_verify, &iso2_verify, pts_verify, 4);
      if (timings) {
        timings->ms_eval_even +=
            (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
      }
    }
    //     printf("ec_eval_even call 1: DBL=%llu, ISOG2=%llu, ISOG2_SING=%llu, "
    //            "ISOG4=%llu, "
    //            "ISOG4_SING=%llu, EVAL2=%llu, EVAL2_SING=%llu, EVAL4=%llu, "
    //            "EVAL4_SING=%llu\n",
    //            (unsigned long long)global_isog_counters.dbl_count,
    //            (unsigned long long)global_isog_counters.isog2_count,
    //            (unsigned long long)global_isog_counters.isog2_sing_count,
    //            (unsigned long long)global_isog_counters.isog4_count,
    //            (unsigned long long)global_isog_counters.isog4_sing_count,
    //            (unsigned long long)global_isog_counters.eval2_ker_count,
    //            (unsigned long long)global_isog_counters.eval2_sing_ker_count,
    //            (unsigned long long)global_isog_counters.eval4_ker_count,
    //            (unsigned long long)global_isog_counters.eval4_sing_ker_count);

    // #define PRINT_AVG(NAME, OPS, COUNT)                                            \
//   if (COUNT > 0)                                                               \
//     printf(NAME ": avg_add=%.2f, avg_mul=%.2f, avg_inv=%.2f, avg_sqr=%.2f, "   \
//                 "avg_sqrt=%.2f\n",                                             \
//            (double)(OPS).add / (COUNT), (double)(OPS).mul / (COUNT),           \
//            (double)(OPS).inv / (COUNT), (double)(OPS).sqr / (COUNT),           \
//            (double)(OPS).sqrt / (COUNT));

    //     PRINT_AVG("DBL", global_isog_counters.dbl_ops,
    //               global_isog_counters.dbl_count);
    //     PRINT_AVG("ISOG2", global_isog_counters.isog2_ops,
    //               global_isog_counters.isog2_count);
    //     PRINT_AVG("ISOG2_SING", global_isog_counters.isog2_sing_ops,
    //               global_isog_counters.isog2_sing_count);
    //     PRINT_AVG("ISOG4", global_isog_counters.isog4_ops,
    //               global_isog_counters.isog4_count);
    //     PRINT_AVG("ISOG4_SING", global_isog_counters.isog4_sing_ops,
    //               global_isog_counters.isog4_sing_count);
    //     PRINT_AVG("EVAL2", global_isog_counters.eval2_ops,
    //               global_isog_counters.eval2_ker_count);
    //     PRINT_AVG("EVAL2_SING", global_isog_counters.eval2_sing_ops,
    //               global_isog_counters.eval2_sing_ker_count);
    //     PRINT_AVG("EVAL4", global_isog_counters.eval4_ops,
    //               global_isog_counters.eval4_ker_count);
    //     PRINT_AVG("EVAL4_SING", global_isog_counters.eval4_sing_ops,
    //               global_isog_counters.eval4_sing_ker_count);
    // #undef PRINT_AVG
    if (timings) {
      timings->total_isog_length_even += (unsigned long)sig->n1;
    }

    copy_curve(&E_chl_verify, &E_chl2_verify);
    copy_point(&Bchl_verify.P, &pts_verify[0]);
    copy_point(&Bchl_verify.Q, &pts_verify[1]);
    copy_point(&Bchl_verify.PmQ, &pts_verify[2]);
    copy_point(&Kbt, &pts_verify[3]);

    VERIFY_TIME_FIELD(ms_aux_doubles, {
      for (int i = 0; i < sig->n1; i++) {
        ec_dbl(&Baux_verify.P, &sig->E_aux, &Baux_verify.P);
        ec_dbl(&Baux_verify.Q, &sig->E_aux, &Baux_verify.Q);
        ec_dbl(&Baux_verify.PmQ, &sig->E_aux, &Baux_verify.PmQ);
      }
    });
  }

  theta_couple_curve_t domain_E_verify;
  domain_E_verify.E1 = sig->E_aux;
  domain_E_verify.E2 = E_chl_verify;
  // curve_print("domain_E1_verify: ", domain_E_verify.E1);
  // printf("\n");
  // curve_print("domain_E2_verify: ", domain_E_verify.E2);
  // printf("\n");

  theta_kernel_couple_points_t dim_two_ker_verify;
  VERIFY_TIME_FIELD(ms_copy_bases_to_kernel,
                    copy_bases_to_kernel(&dim_two_ker_verify, &Baux_verify,
                                         &Bchl_verify));
  // point_print("dim_two_ker_verify.T1.P1: ", dim_two_ker_verify.T1.P1);
  // printf("\n");
  // point_print("dim_two_ker_verify.T1.P2: ", dim_two_ker_verify.T1.P2);
  // printf("\n");
  // point_print("dim_two_ker_verify.T2.P1: ", dim_two_ker_verify.T2.P1);
  // printf("\n");
  // point_print("dim_two_ker_verify.T2.P2: ", dim_two_ker_verify.T2.P2);
  // printf("\n");
  // point_print("dim_two_ker_verify.T1m2.P1: ", dim_two_ker_verify.T1m2.P1);
  // printf("\n");
  // point_print("dim_two_ker_verify.T1m2.P2: ", dim_two_ker_verify.T1m2.P2);
  // printf("\n");

  // assert(
  //     point_order_2f(&Baux_verify.P, &sig->E_aux, (int)EXPONENT_TWO -
  //     sig->n1));
  // assert(
  //     point_order_2f(&Baux_verify.Q, &sig->E_aux, (int)EXPONENT_TWO -
  //     sig->n1));
  // assert(point_order_2f(&Baux_verify.PmQ, &sig->E_aux,
  //                       (int)EXPONENT_TWO - sig->n1));
  // assert(point_order_2f(&Bchl_verify.P, &E_chl_verify,
  //                       (int)EXPONENT_TWO - sig->n1));
  // assert(point_order_2f(&Bchl_verify.Q, &E_chl_verify,
  //                       (int)EXPONENT_TWO - sig->n1));
  // assert(point_order_2f(&Bchl_verify.PmQ, &E_chl_verify,
  //                       (int)EXPONENT_TWO - sig->n1));

  clock_t t_theta_setup = clock();
  bool extra_torsion = false;
  theta_couple_curve_t codomain_E_verify;
  theta_couple_point_t coupleP_verify;
  ec_point_init(&coupleP_verify.P1);
  copy_point(&coupleP_verify.P2, &Kbt);

  unsigned length_verify = (unsigned)EXPONENT_TWO - (unsigned)sig->n1;
  if (timings) {
    timings->ms_theta_setup +=
        (float)(clock() - t_theta_setup) * 1000.f / (float)CLOCKS_PER_SEC;
  }

  int verify1 = 0;
  int verify2 = 0;
  int verify3 = 0;
  {
    clock_t _ts = clock();
    int ret = theta_chain_compute_and_eval_verify(
        length_verify, &domain_E_verify, &dim_two_ker_verify, extra_torsion,
        &codomain_E_verify, &coupleP_verify, 1);
    if (timings) {
      timings->ms_theta_chain_verify +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }
    if (timings && ret) {
      timings->total_isog_length_two_two += length_verify;
    }
    verify1 = ret;
  }

  // assert(verify1);

  // printf("ret: %d\n", ret);
  ec_point_t K_com;
  if (verify1) {
    VERIFY_TIME_FIELD(ms_hash_challenge, {
      fp2_t j_codomain_E1_verify, j_codomain_E2_verify;
      ec_j_inv(&j_codomain_E1_verify, &codomain_E_verify.E1);
      ec_j_inv(&j_codomain_E2_verify, &codomain_E_verify.E2);

      ibz_t chl, chl1, chl2;
      ibz_init(&chl);
      ibz_init(&chl1);
      ibz_init(&chl2);
      ibz_copy_digit_array(&chl, sig->challenge);
      hash_challenge(&chl1, &codomain_E_verify.E1, m, pk, l);
      hash_challenge(&chl2, &codomain_E_verify.E2, m, pk, l);

      if (ibz_cmp(&chl, &chl1) == 0) {
        verify2 = 1;
        copy_point(&K_com, &coupleP_verify.P1);
      }
      if (ibz_cmp(&chl, &chl2) == 0) {
        verify2 = 1;
        copy_point(&K_com, &coupleP_verify.P2);
      }
      ibz_finalize(&chl);
      ibz_finalize(&chl1);
      ibz_finalize(&chl2);
    });
  }
  // assert(verify2);

  VERIFY_TIME_FIELD(ms_final_check, {
    if (verify2) {
      verify3 = !ec_is_zero(&K_com);
    }
  });

  // assert(verify3);

  verify = verify1 && verify2 && verify3;

  // printf("protocols_verify fp2 operations:\n");
  // printf("  fp2_add/sub: %llu\n", (unsigned long long)(fp2_add_count - start_add));
  // printf("  fp2_mul:     %llu\n", (unsigned long long)(fp2_mul_count - start_mul));
  // printf("  fp2_sqr:     %llu\n", (unsigned long long)(fp2_sqr_count - start_sqr));
  // printf("  fp2_inv:     %llu\n", (unsigned long long)(fp2_inv_count - start_inv));
  // printf("  fp2_sqrt:    %llu\n", (unsigned long long)(fp2_sqrt_count - start_sqrt));

  ibz_vec_2_finalize(&vec_can);
  return verify;
}
