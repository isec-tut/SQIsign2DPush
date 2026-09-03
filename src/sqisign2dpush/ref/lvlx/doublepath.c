#include <curve_extras.h>
#include <inttypes.h>
#include <isog.h>
#include <sqisign2dpush.h>
#include <stdio.h>
#include <toolbox.h>

/* Interface */

#define EXPONENT_TWO TORSION_PLUS_EVEN_POWER
#define EXPONENT_THREE TORSION_PLUS_ODD_POWERS[0]
#define POWER_OF_TWO TORSION_PLUS_2POWER
#define POWER_OF_THREE TORSION_PLUS_3POWER

#define DOUBLEPATH_TIME_FIELD(field, ...)                                      \
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

// typedef struct {
//   uint64_t add;
//   uint64_t sqr;
//   uint64_t mul;
// } fp2_eval_ops_t;

// static fp2_eval_ops_t fp2_eval_ops_snapshot(void) {
//   fp2_eval_ops_t ops = {fp2_add_count, fp2_sqr_count, fp2_mul_count};
//   return ops;
// }

// static void fp2_eval_ops_print(const char *label, fp2_eval_ops_t before) {
//   printf("%s: fp2_add/sub=%llu, fp2_sqr=%llu, fp2_mul=%llu\n", label,
//          (unsigned long long)(fp2_add_count - before.add),
//          (unsigned long long)(fp2_sqr_count - before.sqr),
//          (unsigned long long)(fp2_mul_count - before.mul));
// }

static inline void print_deg(ec_degree_odd_t deg) {
#define NUMPP                                                                  \
  (sizeof(TORSION_ODD_PRIMEPOWERS) / sizeof(*TORSION_ODD_PRIMEPOWERS))

  for (int i = 0; i < NUMPP; i++)
    printf("%d,", deg[i]);
  printf("\n");
}

void fp2_print(char *name, fp2_t const a) {
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

static void curve_print(char *name, ec_curve_t E) {
  fp2_t a;
  fp2_copy(&a, &E.C);
  fp2_inv(&a);
  fp2_mul(&a, &a, &E.A);
  fp2_print(name, a);
}

void point_print(char *name, ec_point_t P) {
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

int sqisign2dpush_test_point_order_twof(const ec_point_t *P,
                                        const ec_curve_t *E) {
  ec_point_t test;
  copy_point(&test, P);
  if (fp2_is_zero(&test.z))
    return 0;
  for (int i = 0; i < TORSION_PLUS_EVEN_POWER - 1; i++) {
    ec_dbl(&test, E, &test);
  }
  if (fp2_is_zero(&test.z))
    return 0;
  ec_dbl(&test, E, &test);
  return (fp2_is_zero(&test.z));
}

int sqisign2dpush_test_point_order_threef(const ec_point_t *P,
                                          const ec_curve_t *E) {
  ec_point_t test;
  copy_point(&test, P);
  digit_t three[NWORDS_ORDER] = {0};
  three[0] = 3;
  if (fp2_is_zero(&test.z))
    return 0;
  for (int i = 0; i < EXPONENT_THREE - 1; i++) {
    ec_mul(&test, E, three, &test);
  }
  if (fp2_is_zero(&test.z))
    return 0;
  ec_mul(&test, E, three, &test);
  return (fp2_is_zero(&test.z));
}

static int test_point_order_odd_plus(const ec_point_t *P, const ec_curve_t *E) {
  ec_point_t test = *P;
  digit_t scalar[NWORDS_ORDER] = {0};
  // ibz_printf("TORSION_ODD_PLUS = %Zd\n", &TORSION_ODD_PLUS);
  ibz_to_digits(scalar, &TORSION_ODD_PLUS);
  ec_mul(&test, E, scalar, &test);
  return (fp2_is_zero(&test.z));
}

static int test_point_order_odd_minus(const ec_point_t *P,
                                      const ec_curve_t *E) {
  ec_point_t test = *P;
  digit_t scalar[NWORDS_ORDER] = {0};
  ibz_to_digits(scalar, &TORSION_ODD_MINUS);
  ec_mul(&test, E, scalar, &test);
  return (fp2_is_zero(&test.z));
}

void quat_to_isog_power_of_two(ec_isog_even_t *isog, ibz_vec_2_t *ker_dlog,
                               const quat_alg_elem_t *gamma) {
  quat_left_ideal_t ideal;
  quat_left_ideal_init(&ideal);

  quat_lideal_create_from_primitive(&ideal, gamma, &POWER_OF_TWO, &MAXORD_O0,
                                    &QUATALG_PINFTY);

  id2iso_ideal_to_isogeny_even_dlogs(isog, ker_dlog, &ideal);
  quat_left_ideal_finalize(&ideal);
  return;
}

void quat_to_kernel_power_of_two(ibz_vec_2_t *ker_dlog, const quat_alg_elem_t *gamma){
  quat_left_ideal_t ideal;
  quat_left_ideal_init(&ideal);

  quat_lideal_create_from_primitive(&ideal, gamma, &POWER_OF_TWO, &MAXORD_O0,
                                    &QUATALG_PINFTY);
  ibz_vec_2_t vec;
  ibz_vec_2_init(&vec);

  id2iso_ideal_to_kernel_dlogs_even(&vec, &ideal);

  ibz_copy(&(*ker_dlog)[0], &vec[0]);
  ibz_copy(&(*ker_dlog)[1], &vec[1]);

  ibz_vec_2_finalize(&vec);
  quat_left_ideal_finalize(&ideal);
  return;
}

void quat_to_isog_power_of_three(ec_isog_odd_t *isog, ibz_vec_2_t *ker_dlog,
                                 const quat_alg_elem_t *gamma) {
  quat_left_ideal_t ideal;
  quat_left_ideal_init(&ideal);

  quat_lideal_create_from_primitive(&ideal, gamma, &POWER_OF_THREE, &MAXORD_O0,
                                    &QUATALG_PINFTY);

  assert(ibz_cmp(&(ideal.norm), &POWER_OF_THREE) == 0);
  id2iso_ideal_to_isogeny_odd_plus(isog, ker_dlog, &CURVE_E0, &BASIS_ODD_PLUS,
                                   &ideal);

  assert(fp2_is_zero(&((isog->ker_minus).z)));
  assert(!fp2_is_zero(&((isog->ker_plus).z)));

  quat_left_ideal_finalize(&ideal);
  return;
}

void quat_to_kernel_power_of_three(ibz_vec_2_t *ker_dlog,
                                   const quat_alg_elem_t *gamma) {
  quat_left_ideal_t ideal;
  quat_left_ideal_init(&ideal);

  quat_lideal_create_from_primitive(&ideal, gamma, &POWER_OF_THREE, &MAXORD_O0,
                                    &QUATALG_PINFTY);
  ibz_vec_2_t vec;
  ibz_vec_2_init(&vec);
  ec_degree_odd_t deg;

  id2iso_ideal_to_kernel_dlogs_odd(&vec, &deg, &ideal);

  ibz_t tmp;
  ibz_init(&tmp);

  // multiply out unnecessary cofactor from T-torsion basis
  // assert(sizeof(deg) / sizeof(*deg) ==
  //        sizeof(TORSION_ODD_PRIMEPOWERS) / sizeof(*TORSION_ODD_PRIMEPOWERS));
  for (size_t i = 0; i < sizeof(deg) / sizeof(*deg); ++i) {
    assert(deg[i] <= TORSION_ODD_POWERS[i]);
    if (deg[i] == TORSION_ODD_POWERS[i])
      continue;
    ibz_set(&tmp, TORSION_ODD_PRIMES[i]);
    ibz_pow(&tmp, &tmp, TORSION_ODD_POWERS[i] - deg[i]);
    ibz_mul(&vec[0], &vec[0], &tmp);
    ibz_mul(&vec[1], &vec[1], &tmp);
  }
  ibz_mod(&tmp, &vec[0], &TORSION_ODD_PLUS);
  ibz_copy(&(*ker_dlog)[0], &tmp);
  ibz_mod(&tmp, &vec[1], &TORSION_ODD_PLUS);
  ibz_copy(&(*ker_dlog)[1], &tmp);

  ibz_finalize(&tmp);
  ibz_vec_2_finalize(&vec);
  quat_left_ideal_finalize(&ideal);
  return;
}

void isog_init_two(ec_isog_even_t *isog, const ec_curve_t *curve,
                   const ec_point_t *ker, int length) {
  copy_curve(&(isog->curve), curve);
  copy_point(&(isog->kernel), ker);
#ifdef DEBUG
  assert(sqisign2dpush_test_point_order_twof(ker, curve));
#endif
  isog->length = length;
  return;
}

void isog_init_three(ec_isog_odd_t *isog, const ec_curve_t *curve,
                     const ec_point_t *ker, int length) {
  copy_curve(&(isog->curve), curve);
  copy_point(&(isog->ker_plus), ker);
#ifdef DEBUG
  assert(sqisign2dpush_test_point_order_threef(ker, curve));
#endif
  ec_set_zero(&(isog->ker_minus));

#define NUMPP                                                                  \
  (sizeof(TORSION_ODD_PRIMEPOWERS) / sizeof(*TORSION_ODD_PRIMEPOWERS))

  for (size_t i = 0; i < NUMPP; ++i) {
    (isog->degree)[i] = 0;
    if (TORSION_ODD_PRIMES[i] == 3) {
      assert(length <= TORSION_ODD_POWERS[i]);
      (isog->degree)[i] = length;
    }
  }

  return;
}

// Finds a point P that is independant from vec[0]*basis.P + vec[1]*basis.Q
void complete_three_basis(ec_point_t *P, const ibz_vec_2_t *vec,
                          const ec_basis_t *basis) {
  if (ibz_mod_ui(&(*vec[0]), 3) == 0)
    copy_point(P, &(basis->P));
  else
    copy_point(P, &(basis->Q));
  return;
}

// Finds a point P that is independant from vec[0]*basis.P + vec[1]*basis.Q
void complete_two_basis(ec_point_t *P, const ibz_vec_2_t *vec,
                        const ec_basis_t *basis) {
  if (ibz_mod_ui(&(*vec[0]), 2) == 0)
    copy_point(P, &(basis->P));
  else
    copy_point(P, &(basis->Q));
  return;
}

// TODO(failure case): if intermediate curves (F1,F2,...) have automorphisms,
// result may be incorrect. Should not happen, but needs sanity check
void doublepath(quat_alg_elem_t *gamma, quat_left_ideal_t *lideal_even,
                quat_left_ideal_t *lideal_odd, ec_basis_t *basis_three_image,
                ec_basis_t *basis_two_image, ec_curve_t *E_target, int verbose,
                keygen_timings_t *timings) {

  ibz_t n_gamma, pow_two_square, pow_three_square;
  quat_alg_elem_t gamma_conj;
  ec_isog_even_t dual_two, dual_two_pushed, dual_two_pushed_dual;
  ec_isog_odd_t primal_three, primal_three_pushed, primal_three_pushed_dual;
  ec_point_t list_points[3];
  ec_point_t push_points[4];
  ec_curve_t F1, E1, F2, E2, F2_alt, F1_alt, E_final;
  ec_basis_t basis_two, basis_three;
  ec_isom_t norm_isom;
  ec_isog_even_t primal_two, primal_two_second_half;
  ec_isog_odd_t dual_three, dual_three_second_half;

  ibz_vec_2_t dual_two_ker_dlog, primal_two_ker_dlog, primal_three_ker_dlog,
      dual_three_ker_dlog;
  ibz_vec_2_init(&dual_two_ker_dlog);
  ibz_vec_2_init(&primal_two_ker_dlog);
  ibz_vec_2_init(&primal_three_ker_dlog);
  ibz_vec_2_init(&dual_three_ker_dlog);

  ibz_init(&n_gamma);
  ibz_init(&pow_two_square);
  ibz_init(&pow_three_square);
  quat_alg_elem_init(&gamma_conj);

  // FIND AN ENDOMORPHISM OF NORM n_gamma = (POWER_OF_TWO*POWER_OF_THREE)^2
  ibz_mul(&pow_two_square, &POWER_OF_TWO, &POWER_OF_TWO);
  ibz_mul(&pow_three_square, &POWER_OF_THREE, &POWER_OF_THREE);
  ibz_mul(&n_gamma, &pow_two_square, &pow_three_square);

  if (verbose) {
    TAC("represent_integer in");
  }
  int found;
  if (timings) {
    clock_t _ts = clock();
    found = represent_integer(gamma, &n_gamma, &QUATALG_PINFTY);
    timings->ms_dp_represent_integer +=
        (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
  } else {
    found = represent_integer(gamma, &n_gamma, &QUATALG_PINFTY);
  }
  assert(found);
  if (verbose) {
    TAC("represent_integer out");
  }
  // TODO(failure case): check that the solution is not divisible by an integer?
  assert(quat_alg_is_primitive(gamma, &MAXORD_O0, &QUATALG_PINFTY));

#ifndef NDEBUG
  ibz_t norm_debug;
  ibq_t norm_debug_q;
  ibz_init(&norm_debug);
  ibq_init(&norm_debug_q);
  quat_alg_norm(&norm_debug_q, gamma, &QUATALG_PINFTY);
  ibq_to_ibz(&norm_debug, &norm_debug_q);
  assert(ibz_cmp(&norm_debug, &n_gamma) == 0);
  ibz_finalize(&norm_debug);
  ibq_finalize(&norm_debug_q);
#endif

  // COMPUTE THE FIRST HALF OF GAMMA AND OF ITS DUAL
  quat_alg_conj(&gamma_conj, gamma);

  DOUBLEPATH_TIME_FIELD(ms_dp_lideal_create,
    quat_lideal_create_from_primitive(lideal_even, gamma, &pow_two_square,
                                      &MAXORD_O0, &QUATALG_PINFTY);
    quat_lideal_create_from_primitive(lideal_odd, &gamma_conj, &pow_three_square,
                                      &MAXORD_O0, &QUATALG_PINFTY));

  DOUBLEPATH_TIME_FIELD(ms_dp_quat_to_isog_two,
    quat_to_isog_power_of_two(&primal_two, &dual_two_ker_dlog, gamma));
  // quat_to_isog_power_of_three(&primal_three, &primal_three_ker_dlog, gamma);
  DOUBLEPATH_TIME_FIELD(ms_dp_quat_to_kernel_three,
    quat_to_kernel_power_of_three(&primal_three_ker_dlog, gamma));

  // quat_to_isog_power_of_two(&dual_two, &dual_two_ker_dlog, &gamma_conj);
  DOUBLEPATH_TIME_FIELD(ms_dp_quat_to_isog_three,
    quat_to_isog_power_of_three(&dual_three, &dual_three_ker_dlog, &gamma_conj));
  DOUBLEPATH_TIME_FIELD(ms_dp_quat_to_kernel_two,
    quat_to_kernel_power_of_two(&dual_two_ker_dlog, &gamma_conj));

  // TODO: if basis_three_image = NULL, then we dont need the image of
  // BASIS_THREE, and it is (slightly?) faster to compute the image of the
  // following two points instead copy_point(list_points + 0,
  // &(primal_three.ker_plus)); complete_three_basis_E0(list_points + 1,
  // &primal_three_ker_dlog);

  copy_point(list_points + 0, &BASIS_THREE.P);
  copy_point(list_points + 1, &BASIS_THREE.Q);
  copy_point(list_points + 2, &BASIS_THREE.PmQ);

  {
    clock_t _ts = clock();
    ec_eval_even(&F1, &primal_two, list_points, 3);

    if (timings) {
      float _dt = (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_eval_even += _dt;
      timings->ms_dp_eval_even1 += _dt;
      timings->total_isog_length_even += (unsigned long)EXPONENT_TWO;
    }
  }


#ifdef DEBUG
  assert(sqisign2dpush_test_point_order_threef(list_points + 0, &F1));
  assert(sqisign2dpush_test_point_order_threef(list_points + 1, &F1));
  assert(sqisign2dpush_test_point_order_threef(list_points + 2, &F1));
#endif

  copy_point(&(basis_three.P), list_points + 0);
  copy_point(&(basis_three.Q), list_points + 1);
  copy_point(&(basis_three.PmQ), list_points + 2);

  DOUBLEPATH_TIME_FIELD(ms_dp_biscalar1,
    ec_biscalar_mul_ibz(list_points + 0, &F1, &(primal_three_ker_dlog[0]),
                        &(primal_three_ker_dlog[1]), &basis_three));
  DOUBLEPATH_TIME_FIELD(ms_dp_isog_init_three,
    isog_init_three(&primal_three_pushed, &F1, list_points + 0,
                    EXPONENT_THREE));

  ec_point_t ker_primal_three_pushed_dual;
  DOUBLEPATH_TIME_FIELD(ms_dp_complete_three_basis,
    complete_three_basis(&ker_primal_three_pushed_dual, &primal_three_ker_dlog,
                         &basis_three));

  ec_basis_t basis2_F1;
  DOUBLEPATH_TIME_FIELD(ms_dp_curve_to_basis_2,
    (void)ec_curve_to_basis_2f_to_hint(&basis2_F1, &F1,
                                       TORSION_PLUS_EVEN_POWER));

  copy_point(push_points + 0, &basis2_F1.P);
  copy_point(push_points + 1, &basis2_F1.Q);
  copy_point(push_points + 2, &basis2_F1.PmQ);
  copy_point(push_points + 3, &ker_primal_three_pushed_dual);

  if (verbose)
    TAC("ec_eval_three in");
  {

    clock_t _ts = clock();
    ec_eval_three(&E1, &primal_three_pushed, push_points, 4);

    if (timings) {
      float _dt = (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_eval_three += _dt;
      timings->ms_dp_eval_three1 += _dt;
      timings->ms_keygen_isog += _dt;
      timings->total_isog_length_three += (unsigned long)EXPONENT_THREE;
    }
  }

  ec_basis_t basis2_E1;
  copy_point(&basis2_E1.P, push_points + 0);
  copy_point(&basis2_E1.Q, push_points + 1);
  copy_point(&basis2_E1.PmQ, push_points + 2);
  copy_point(&ker_primal_three_pushed_dual, push_points + 3);

  copy_point(list_points + 0, &BASIS_EVEN.P);
  copy_point(list_points + 1, &BASIS_EVEN.Q);
  copy_point(list_points + 2, &BASIS_EVEN.PmQ);

  {

    clock_t _ts = clock();
    ec_eval_three(&F2, &dual_three, list_points, 3);

    if (timings) {
      float _dt = (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_eval_three += _dt;
      timings->ms_dp_eval_three2 += _dt;
      timings->ms_keygen_isog += _dt;
      // printf(" ms_eval_three 2: %f\n", _dt);
      timings->total_isog_length_three += (unsigned long)EXPONENT_THREE;
    }
  }

  copy_point(&(basis_two.P), list_points + 0);
  copy_point(&(basis_two.Q), list_points + 1);
  copy_point(&(basis_two.PmQ), list_points + 2);

  DOUBLEPATH_TIME_FIELD(ms_dp_biscalar2,
    ec_biscalar_mul_ibz(list_points + 0, &F2, &(dual_two_ker_dlog[0]),
                        &(dual_two_ker_dlog[1]), &basis_two));
#ifdef DEBUG
  assert(sqisign2dpush_test_point_order_twof(list_points + 0, &F2));
#endif
  DOUBLEPATH_TIME_FIELD(ms_dp_isog_init_two,
    isog_init_two(&dual_two_pushed, &F2, list_points + 0, EXPONENT_TWO));

  ec_point_t ker_dual_two_pushed_dual;

  DOUBLEPATH_TIME_FIELD(ms_dp_complete_two_basis,
    complete_two_basis(&ker_dual_two_pushed_dual, &dual_two_ker_dlog,
                       &basis_two));

#ifdef DEBUG
  assert(sqisign2dpush_test_point_order_twof(&ker_dual_two_pushed_dual, &F2));
#endif
  ec_basis_t basis3_F2;
  DOUBLEPATH_TIME_FIELD(ms_dp_curve_to_basis_3,
    (void)ec_curve_to_basis_3f_to_hint(&basis3_F2, &F2));
  copy_point(push_points + 0, &basis3_F2.P);
  copy_point(push_points + 1, &basis3_F2.Q);
  copy_point(push_points + 2, &basis3_F2.PmQ);
  copy_point(push_points + 3, &ker_dual_two_pushed_dual);

  {
    clock_t _ts = clock();
    ec_eval_even(&E2, &dual_two_pushed, push_points, 4);

    if (timings) {
      float _dt = (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_eval_even += _dt;
      timings->ms_dp_eval_even2 += _dt;
      timings->ms_keygen_isog += _dt;
      timings->total_isog_length_even += (unsigned long)EXPONENT_TWO;
    }
  }

  ec_basis_t basis3_E2;
  copy_point(&basis3_E2.P, push_points + 0);
  copy_point(&basis3_E2.Q, push_points + 1);
  copy_point(&basis3_E2.PmQ, push_points + 2);
  copy_point(&ker_dual_two_pushed_dual, push_points + 3);

#ifdef DEBUG
  assert(sqisign2dpush_test_point_order_twof(&ker_dual_two_pushed_dual, &E2));
#endif

// FIRST HALF OF GAMMA AND FIRST HALF OF ITS DUAL HAVE ISOMORPHIC TARGETS
#ifndef NDEBUG
  fp2_t j_R, j_L;
  ec_j_inv(&j_R, &E1);
  ec_j_inv(&j_L, &E2);
  assert(fp2_is_equal(&j_R, &j_L));
#endif
  ec_isom_t isom_E1_E2;
  ec_isomorphism(&isom_E1_E2, &E1, &E2);

  // PUSH THINGS AROUND TO GET THE 3-PART OF GAMMA_DUAL
  if (basis_two_image) {
    ec_point_t ker_primal_three_pushed_dual_E2;
    copy_point(&ker_primal_three_pushed_dual_E2, &ker_primal_three_pushed_dual);
    ec_iso_eval(&ker_primal_three_pushed_dual_E2, &isom_E1_E2);

    digit_t a1[NWORDS_ORDER] = {0}, a2[NWORDS_ORDER] = {0};
    DOUBLEPATH_TIME_FIELD(ms_dp_dlog_3_tate_R,
      ec_dlog_3_tate_R(a1, a2, &basis3_E2, &ker_primal_three_pushed_dual_E2,
                       &E2, EXPONENT_THREE));
    ibz_t a1_ibz, a2_ibz;
    ibz_init(&a1_ibz);
    ibz_init(&a2_ibz);
    ibz_copy_digit_array(&a1_ibz, a1);
    ibz_copy_digit_array(&a2_ibz, a2);

    ec_point_t ker_dual_three_second_half;
    DOUBLEPATH_TIME_FIELD(ms_dp_biscalar3,
      ec_biscalar_mul_ibz(&ker_dual_three_second_half, &F2, &a1_ibz, &a2_ibz,
                          &basis3_F2));
    DOUBLEPATH_TIME_FIELD(ms_dp_isog_init_three,
      isog_init_three(&dual_three_second_half, &F2,
                      &ker_dual_three_second_half, EXPONENT_THREE));

    copy_point(list_points + 0, &basis_two.P);
    copy_point(list_points + 1, &basis_two.Q);
    copy_point(list_points + 2, &basis_two.PmQ);

    if (verbose)
      TAC("ec_eval_three in");
    {

      clock_t _ts = clock();
      ec_eval_three(&E_final, &dual_three_second_half, list_points, 3);

      if (timings) {
        float _dt = (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
        timings->ms_eval_three += _dt;
        timings->ms_dp_eval_three3 += _dt;
        timings->ms_keygen_isog += _dt;
        // printf(" ms_eval_three 3: %f\n", _dt);
        timings->total_isog_length_three += (unsigned long)EXPONENT_THREE;
      }
    }
    if (verbose)
      TAC("ec_eval_three out");

    ec_curve_normalize(&E_final, &norm_isom, &E_final);
    ec_iso_eval(list_points + 0, &norm_isom);
    ec_iso_eval(list_points + 1, &norm_isom);
    ec_iso_eval(list_points + 2, &norm_isom);

    copy_point(&(basis_two_image->P), list_points + 0);
    copy_point(&(basis_two_image->Q), list_points + 1);
    copy_point(&(basis_two_image->PmQ), list_points + 2);

    if (E_target) {
      // TODO: normalize E_target with ec_curve_normalize
      copy_curve(E_target, &E_final);
    }

    ibz_finalize(&a1_ibz);
    ibz_finalize(&a2_ibz);
  }

  if (basis_three_image) {
    // PUSH THINGS AROUND TO GET THE 2-PART OF GAMMA
    ec_point_t ker_dual_two_pushed_dual_E1;
    copy_point(&ker_dual_two_pushed_dual_E1, &ker_dual_two_pushed_dual);
    ec_iso_inv(&isom_E1_E2);
    ec_iso_eval(&ker_dual_two_pushed_dual_E1, &isom_E1_E2);

    digit_t a3[NWORDS_ORDER] = {0}, a4[NWORDS_ORDER] = {0};
    DOUBLEPATH_TIME_FIELD(ms_dp_dlog_2_tate_R,
      ec_dlog_2_tate_R(a3, a4, &basis2_E1, &ker_dual_two_pushed_dual_E1, &E1,
                       EXPONENT_TWO));
    ibz_t a3_ibz, a4_ibz;
    ibz_init(&a3_ibz);
    ibz_init(&a4_ibz);
    ibz_copy_digit_array(&a3_ibz, a3);
    ibz_copy_digit_array(&a4_ibz, a4);

    ec_point_t ker_primal_two_second_half;
    DOUBLEPATH_TIME_FIELD(ms_dp_biscalar4,
      ec_biscalar_mul_ibz(&ker_primal_two_second_half, &F1, &a3_ibz, &a4_ibz,
                          &basis2_F1));

    DOUBLEPATH_TIME_FIELD(ms_dp_isog_init_two,
      isog_init_two(&primal_two_second_half, &F1,
                    &ker_primal_two_second_half, EXPONENT_TWO));

    copy_point(list_points + 0, &basis_three.P);
    copy_point(list_points + 1, &basis_three.Q);
    copy_point(list_points + 2, &basis_three.PmQ);

    {
      clock_t _ts = clock();
      ec_eval_even(&E_final, &primal_two_second_half, list_points, 3);

      if (timings) {
        float _dt = (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
        timings->ms_eval_even += _dt;
        timings->ms_dp_eval_even3 += _dt;
        timings->ms_keygen_isog += _dt;
        timings->total_isog_length_even += (unsigned long)EXPONENT_TWO;
      }
    }

    ec_curve_normalize(&E_final, &norm_isom, &E_final);
    ec_iso_eval(list_points + 0, &norm_isom);
    ec_iso_eval(list_points + 1, &norm_isom);
    ec_iso_eval(list_points + 2, &norm_isom);

    copy_point(&(basis_three_image->P), list_points + 0);
    copy_point(&(basis_three_image->Q), list_points + 1);
    copy_point(&(basis_three_image->PmQ), list_points + 2);

#ifndef NDEBUG
    if (E_target && basis_two_image) {
      assert(fp2_is_equal(&E_final.A, &(E_target->A)));
      assert(fp2_is_equal(&E_final.C, &(E_target->C)));
    }
#endif

    if (E_target && !basis_two_image) {
      copy_curve(E_target, &E_final);
    }
    ibz_finalize(&a3_ibz);
    ibz_finalize(&a4_ibz);
  }

  quat_alg_elem_finalize(&gamma_conj);
  ibz_finalize(&n_gamma);
  ibz_finalize(&pow_two_square);
  ibz_finalize(&pow_three_square);
  ibz_vec_2_finalize(&dual_two_ker_dlog);
  ibz_vec_2_finalize(&primal_two_ker_dlog);
  ibz_vec_2_finalize(&primal_three_ker_dlog);
  ibz_vec_2_finalize(&dual_three_ker_dlog);
  return;
}

// TODO(failure case): if intermediate curves (F1,F2,...) have automorphisms,
// result may be incorrect. Should not happen, but needs sanity check
int fastcommit(quat_alg_elem_t *gamma, quat_left_ideal_t *lideal_even,
               quat_left_ideal_t *lideal_odd, ec_basis_t *basis_two_image,
               ec_curve_t *E_target, ec_point_t *kernel_iso_three1,
               ec_point_t *kernel_iso_three2, ec_curve_t *E_com_mid,
               ec_basis_t *basis_two_mid, int verbose,
               sign_timings_t *timings) {

  ibz_t n_gamma, pow_two_square, pow_three_square;
  quat_alg_elem_t gamma_conj;
  ec_isog_even_t dual_two, dual_two_pushed, dual_two_pushed_dual;
  ec_isog_odd_t primal_three, primal_three_pushed, primal_three_pushed_dual;
  ec_point_t list_points[3];
  ec_point_t push_points[4];
  ec_curve_t F1, E1, F2, E2, F2_alt, F1_alt, E_final;
  ec_basis_t basis_two, basis_three;
  ec_isom_t norm_isom;
  ec_isog_even_t primal_two, primal_two_second_half;
  ec_isog_odd_t dual_three, dual_three_second_half;

  ibz_vec_2_t dual_two_ker_dlog, primal_two_ker_dlog, primal_three_ker_dlog,
      dual_three_ker_dlog;
  ibz_vec_2_init(&dual_two_ker_dlog);
  ibz_vec_2_init(&primal_two_ker_dlog);
  ibz_vec_2_init(&primal_three_ker_dlog);
  ibz_vec_2_init(&dual_three_ker_dlog);

  ibz_init(&n_gamma);
  ibz_init(&pow_two_square);
  ibz_init(&pow_three_square);
  quat_alg_elem_init(&gamma_conj);

  // FIND AN ENDOMORPHISM OF NORM n_gamma = (POWER_OF_TWO*POWER_OF_THREE)^2
  ibz_mul(&pow_two_square, &POWER_OF_TWO, &POWER_OF_TWO);
  ibz_mul(&pow_three_square, &POWER_OF_THREE, &POWER_OF_THREE);
  ibz_mul(&n_gamma, &pow_two_square, &pow_three_square);

  if (verbose)
    TAC("represent_integer in");
  int found;
  DOUBLEPATH_TIME_FIELD(ms_fc_represent_integer,
                        found =
                            represent_integer(gamma, &n_gamma, &QUATALG_PINFTY));
  assert(found);
  if (verbose)
    TAC("represent_integer out");
  // TODO(failure case): check that the solution is not divisible by an integer?
  assert(quat_alg_is_primitive(gamma, &MAXORD_O0, &QUATALG_PINFTY));

#ifndef NDEBUG
  ibz_t norm_debug;
  ibq_t norm_debug_q;
  ibz_init(&norm_debug);
  ibq_init(&norm_debug_q);
  quat_alg_norm(&norm_debug_q, gamma, &QUATALG_PINFTY);
  ibq_to_ibz(&norm_debug, &norm_debug_q);
  assert(ibz_cmp(&norm_debug, &n_gamma) == 0);
  ibz_finalize(&norm_debug);
  ibq_finalize(&norm_debug_q);
#endif

  // COMPUTE THE FIRST HALF OF GAMMA AND OF ITS DUAL
  quat_alg_conj(&gamma_conj, gamma);

  DOUBLEPATH_TIME_FIELD(ms_fc_lideal_create, {
    quat_lideal_create_from_primitive(lideal_even, gamma, &pow_two_square,
                                      &MAXORD_O0, &QUATALG_PINFTY);
    quat_lideal_create_from_primitive(lideal_odd, &gamma_conj,
                                      &pow_three_square, &MAXORD_O0,
                                      &QUATALG_PINFTY);
  });

  DOUBLEPATH_TIME_FIELD(ms_fc_quat_to_isog_two, {
    quat_left_ideal_t ideal;
    quat_left_ideal_init(&ideal);

    clock_t _ts = clock();
    quat_lideal_create_from_primitive(&ideal, gamma, &POWER_OF_TWO,
                                      &MAXORD_O0, &QUATALG_PINFTY);
    if (timings) {
      timings->ms_fc_quat_to_quat +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    _ts = clock();
    id2iso_ideal_to_isogeny_even_dlogs(&primal_two, &dual_two_ker_dlog,
                                       &ideal);
    if (timings) {
      timings->ms_fc_quat_to_ec +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    quat_left_ideal_finalize(&ideal);
  });
  // quat_to_isog_power_of_three(&primal_three, &primal_three_ker_dlog, gamma);
  DOUBLEPATH_TIME_FIELD(ms_fc_quat_to_kernel_three, {
    quat_left_ideal_t ideal;
    quat_left_ideal_init(&ideal);

    clock_t _ts = clock();
    quat_lideal_create_from_primitive(&ideal, gamma, &POWER_OF_THREE,
                                      &MAXORD_O0, &QUATALG_PINFTY);
    if (timings) {
      timings->ms_fc_quat_to_quat +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    ibz_vec_2_t vec;
    ibz_vec_2_init(&vec);
    ec_degree_odd_t deg;
    _ts = clock();
    id2iso_ideal_to_kernel_dlogs_odd(&vec, &deg, &ideal);
    if (timings) {
      timings->ms_fc_quat_to_quat +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    ibz_t tmp;
    ibz_init(&tmp);
    _ts = clock();
    for (size_t i = 0; i < sizeof(deg) / sizeof(*deg); ++i) {
      assert(deg[i] <= TORSION_ODD_POWERS[i]);
      if (deg[i] == TORSION_ODD_POWERS[i])
        continue;
      ibz_set(&tmp, TORSION_ODD_PRIMES[i]);
      ibz_pow(&tmp, &tmp, TORSION_ODD_POWERS[i] - deg[i]);
      ibz_mul(&vec[0], &vec[0], &tmp);
      ibz_mul(&vec[1], &vec[1], &tmp);
    }
    ibz_mod(&tmp, &vec[0], &TORSION_ODD_PLUS);
    ibz_copy(&primal_three_ker_dlog[0], &tmp);
    ibz_mod(&tmp, &vec[1], &TORSION_ODD_PLUS);
    ibz_copy(&primal_three_ker_dlog[1], &tmp);
    if (timings) {
      timings->ms_fc_quat_to_other +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    ibz_finalize(&tmp);
    ibz_vec_2_finalize(&vec);
    quat_left_ideal_finalize(&ideal);
  });

  // quat_to_isog_power_of_two(&dual_two, &dual_two_ker_dlog, &gamma_conj);
  DOUBLEPATH_TIME_FIELD(ms_fc_quat_to_isog_three, {
    quat_left_ideal_t ideal;
    quat_left_ideal_init(&ideal);

    clock_t _ts = clock();
    quat_lideal_create_from_primitive(&ideal, &gamma_conj, &POWER_OF_THREE,
                                      &MAXORD_O0, &QUATALG_PINFTY);
    if (timings) {
      timings->ms_fc_quat_to_quat +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    assert(ibz_cmp(&(ideal.norm), &POWER_OF_THREE) == 0);
    _ts = clock();
    id2iso_ideal_to_isogeny_odd_plus(&dual_three, &dual_three_ker_dlog,
                                     &CURVE_E0, &BASIS_ODD_PLUS, &ideal);
    if (timings) {
      timings->ms_fc_quat_to_ec +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    assert(fp2_is_zero(&((dual_three.ker_minus).z)));
    assert(!fp2_is_zero(&((dual_three.ker_plus).z)));
    quat_left_ideal_finalize(&ideal);
  });
  DOUBLEPATH_TIME_FIELD(ms_fc_quat_to_kernel_two, {
    quat_left_ideal_t ideal;
    quat_left_ideal_init(&ideal);

    clock_t _ts = clock();
    quat_lideal_create_from_primitive(&ideal, &gamma_conj, &POWER_OF_TWO,
                                      &MAXORD_O0, &QUATALG_PINFTY);
    if (timings) {
      timings->ms_fc_quat_to_quat +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    ibz_vec_2_t vec;
    ibz_vec_2_init(&vec);
    _ts = clock();
    id2iso_ideal_to_kernel_dlogs_even(&vec, &ideal);
    if (timings) {
      timings->ms_fc_quat_to_quat +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    _ts = clock();
    ibz_copy(&dual_two_ker_dlog[0], &vec[0]);
    ibz_copy(&dual_two_ker_dlog[1], &vec[1]);
    if (timings) {
      timings->ms_fc_quat_to_other +=
          (float)(clock() - _ts) * 1000.f / (float)CLOCKS_PER_SEC;
    }

    ibz_vec_2_finalize(&vec);
    quat_left_ideal_finalize(&ideal);
  });

  // TODO: if basis_three_image = NULL, then we dont need the image of
  // BASIS_THREE, and it is (slightly?) faster to compute the image of the
  // following two points instead copy_point(list_points + 0,
  // &(primal_three.ker_plus)); complete_three_basis_E0(list_points + 1,
  // &primal_three_ker_dlog);

  DOUBLEPATH_TIME_FIELD(ms_fc_biscalar1,
                        ec_biscalar_mul_ibz(
                            list_points + 0, &CURVE_E0,
                            &(primal_three_ker_dlog[0]),
                            &(primal_three_ker_dlog[1]), &BASIS_THREE));
  DOUBLEPATH_TIME_FIELD(ms_fc_complete_three_basis,
                        complete_three_basis(list_points + 1,
                                             &primal_three_ker_dlog,
                                             &BASIS_THREE));

  {
    clock_t _t_isog = clock();
    ec_eval_even(&F1, &primal_two, list_points, 2);
    if (timings) {
      timings->ms_commit_isog +=
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_fc_eval_even1 +=
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
    }
  }
  if (timings) {
    timings->total_isog_length_even_commit += (unsigned long)EXPONENT_TWO;
  }

#ifdef DEBUG
  assert(sqisign2dpush_test_point_order_threef(list_points + 0, &F1));
  assert(sqisign2dpush_test_point_order_threef(list_points + 1, &F1));
  assert(sqisign2dpush_test_point_order_threef(list_points + 2, &F1));
#endif

  isog_init_three(&primal_three_pushed, &F1, list_points + 0, EXPONENT_THREE);

  ec_point_t ker_primal_three_pushed_dual;
  copy_point(&ker_primal_three_pushed_dual, list_points + 1);

  {
    clock_t _t_isog = clock();
    ec_eval_three(&E1, &primal_three_pushed, &ker_primal_three_pushed_dual,
                  1); // point generates the dual of primal_three_pushed
    if (timings) {
      timings->ms_commit_isog +=
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_fc_eval_three1 +=
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
    }
  }

  if (timings) {
    timings->total_isog_length_three_commit += (unsigned long)EXPONENT_THREE;
  }

  copy_point(list_points + 0, &BASIS_EVEN.P);
  copy_point(list_points + 1, &BASIS_EVEN.Q);
  copy_point(list_points + 2, &BASIS_EVEN.PmQ);

  {
    clock_t _t_isog = clock();
    ec_eval_three(&F2, &dual_three, list_points, 3);
    if (timings) {
      timings->ms_commit_isog +=
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_fc_eval_three2 +=
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
    }
  }

  if (timings) {
    timings->total_isog_length_three_commit += (unsigned long)EXPONENT_THREE;
  }

  copy_curve(E_com_mid, &F2);
  copy_point(kernel_iso_three1, &dual_three.ker_plus);
  copy_point(&(basis_two_mid->P), list_points + 0);
  copy_point(&(basis_two_mid->Q), list_points + 1);
  copy_point(&(basis_two_mid->PmQ), list_points + 2);


  copy_point(&(basis_two.P), list_points + 0);
  copy_point(&(basis_two.Q), list_points + 1);
  copy_point(&(basis_two.PmQ), list_points + 2);

  DOUBLEPATH_TIME_FIELD(ms_fc_biscalar2,
                        ec_biscalar_mul_ibz(list_points + 0, &F2,
                                            &(dual_two_ker_dlog[0]),
                                            &(dual_two_ker_dlog[1]),
                                            &basis_two));
  isog_init_two(&dual_two_pushed, &F2, list_points + 0, EXPONENT_TWO);

  ec_basis_t basis3_F2;
  DOUBLEPATH_TIME_FIELD(ms_fc_curve_to_basis_3,
                        (void)ec_curve_to_basis_3f_to_hint(&basis3_F2, &F2));
  copy_point(push_points + 0, &basis3_F2.P);
  copy_point(push_points + 1, &basis3_F2.Q);
  copy_point(push_points + 2, &basis3_F2.PmQ);

  {
    clock_t _t_isog = clock();
    ec_eval_even(&E2, &dual_two_pushed, push_points, 3);
    if (timings) {
      timings->ms_commit_isog +=
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      timings->ms_fc_eval_even2 +=
          (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
    }
  }

  if (timings) {
    timings->total_isog_length_even_commit += (unsigned long)EXPONENT_TWO;
  }

  ec_basis_t basis3_E2;
  copy_point(&basis3_E2.P, push_points + 0);
  copy_point(&basis3_E2.Q, push_points + 1);
  copy_point(&basis3_E2.PmQ, push_points + 2);

// FIRST HALF OF GAMMA AND FIRST HALF OF ITS DUAL HAVE ISOMORPHIC TARGETS
#ifndef NDEBUG
  fp2_t j_R, j_L;
  ec_j_inv(&j_R, &E1);
  ec_j_inv(&j_L, &E2);
  assert(fp2_is_equal(&j_R, &j_L));
#endif
  ec_isom_t isom_E1_E2;
  ec_isomorphism(&isom_E1_E2, &E1, &E2);

  // PUSH THINGS AROUND TO GET THE 3-PART OF GAMMA_DUAL
  if (basis_two_image) {
    ec_point_t ker_primal_three_pushed_dual_E2;
    copy_point(&ker_primal_three_pushed_dual_E2, &ker_primal_three_pushed_dual);
    ec_iso_eval(&ker_primal_three_pushed_dual_E2, &isom_E1_E2);

    digit_t a1[NWORDS_ORDER] = {0}, a2[NWORDS_ORDER] = {0};
    DOUBLEPATH_TIME_FIELD(ms_fc_dlog_3_tate_R,
                          ec_dlog_3_tate_R(
                              a1, a2, &basis3_E2,
                              &ker_primal_three_pushed_dual_E2, &E2,
                              EXPONENT_THREE));
    ibz_t a1_ibz, a2_ibz;
    ibz_init(&a1_ibz);
    ibz_init(&a2_ibz);
    ibz_copy_digit_array(&a1_ibz, a1);
    ibz_copy_digit_array(&a2_ibz, a2);

    ec_point_t ker_dual_three_second_half;
    DOUBLEPATH_TIME_FIELD(ms_fc_biscalar3,
                          ec_biscalar_mul_ibz(&ker_dual_three_second_half, &F2,
                                              &a1_ibz, &a2_ibz, &basis3_F2));
    isog_init_three(&dual_three_second_half, &F2, &ker_dual_three_second_half,
                    EXPONENT_THREE);

    copy_point(list_points + 0, &basis_two.P);
    copy_point(list_points + 1, &basis_two.Q);
    copy_point(list_points + 2, &basis_two.PmQ);

    {
      clock_t _t_isog = clock();
      ec_eval_three(&E_final, &dual_three_second_half, list_points, 3);
      if (timings) {
      timings->ms_commit_isog +=
            (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
        timings->ms_fc_eval_three3 +=
            (float)(clock() - _t_isog) * 1000.f / (float)CLOCKS_PER_SEC;
      }
    }

    if (timings) {
      timings->total_isog_length_three_commit += (unsigned long)EXPONENT_THREE;
    }

    copy_point(kernel_iso_three2, &ker_dual_three_second_half);
    // point_print("kernel_iso_three2", *kernel_iso_three2);

    ec_curve_normalize(&E_final, &norm_isom, &E_final);
    ec_iso_eval(list_points + 0, &norm_isom);
    ec_iso_eval(list_points + 1, &norm_isom);
    ec_iso_eval(list_points + 2, &norm_isom);

    copy_point(&(basis_two_image->P), list_points + 0);
    copy_point(&(basis_two_image->Q), list_points + 1);
    copy_point(&(basis_two_image->PmQ), list_points + 2);

    if (E_target) {
      // TODO: normalize E_target with ec_curve_normalize
      copy_curve(E_target, &E_final);
    }

    ibz_finalize(&a1_ibz);
    ibz_finalize(&a2_ibz);
  }

  quat_alg_elem_finalize(&gamma_conj);
  ibz_finalize(&n_gamma);
  ibz_finalize(&pow_two_square);
  ibz_finalize(&pow_three_square);
  ibz_vec_2_finalize(&dual_two_ker_dlog);
  ibz_vec_2_finalize(&primal_two_ker_dlog);
  ibz_vec_2_finalize(&primal_three_ker_dlog);
  ibz_vec_2_finalize(&dual_three_ker_dlog);
  return 1;
}
