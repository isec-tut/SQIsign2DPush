/* Parameter-independent EC tests.
 *
 * The old tests embedded coordinates and orders for the former lvl1
 * parameter set. These checks generate torsion bases with the active
 * per-level parameters. */

/* Avoid including endomorphism_action.h here: it also exposes quaternion
 * declarations, which are intentionally not a dependency of the EC tests. */
static void ec_test_curve_a_zero(ec_curve_t *curve) {
  ec_curve_init(curve);
  fp2_set_zero(&curve->A);
  AC_to_A24(&curve->A24, curve);
}

static int ec_test_same_x(const ec_point_t *left, const ec_point_t *right) {
  fp2_t lhs, rhs;
  fp2_mul(&lhs, &left->x, &right->z);
  fp2_mul(&rhs, &right->x, &left->z);
  return fp2_is_equal(&lhs, &rhs);
}

static void ec_test_scalar(digit_t *scalar, digit_t value) {
  memset(scalar, 0, sizeof(digit_t) * NWORDS_ORDER);
  scalar[0] = value;
}

bool ec_test(void) {
  ec_curve_t curve;
  ec_point_t p, q, p_minus_q, doubled, expected, sum, combined, ac;
  ec_basis_t basis;
  digit_t two[NWORDS_ORDER], seven[NWORDS_ORDER];
  const char *failure = NULL;

  printf("\n--------------------------------------------------------------------------------------------------------\n\n");
  printf("Testing ecc functions: \n\n");

  ec_test_curve_a_zero(&curve);
  ec_curve_to_basis_2(&basis, &curve);
  copy_point(&p, &basis.P);
  copy_point(&q, &basis.Q);
  copy_point(&p_minus_q, &basis.PmQ);
  basis.P = p;
  basis.Q = q;
  basis.PmQ = p_minus_q;
  fp2_copy(&ac.x, &curve.A);
  fp2_copy(&ac.z, &curve.C);
  ec_test_scalar(two, 2);
  ec_test_scalar(seven, 7);

  /* xDBL(P) = [2]P and xADD(P,Q,P-Q) = P+Q. */
  xDBL(&doubled, &p, &ac);
  xMUL(&expected, &p, two, &curve);
  if (!ec_test_same_x(&doubled, &expected)) { failure = "xDBL"; goto failed; }

  xADD(&sum, &p, &q, &p_minus_q);
  /* P+Q from differential addition also agrees with the generic API. */
  ec_test_scalar(two, 1);
  xDBLMUL(&expected, &p, two, &q, two, &p_minus_q, &curve);
  if (!ec_test_same_x(&sum, &expected)) { failure = "xADD"; goto failed; }

  /* xDBLMUL uses its own recoding; exercise it with nonzero scalars. */
  ec_test_scalar(two, 3);
  ec_test_scalar(seven, 7);
  ec_biscalar_mul(&combined, &curve, seven, two, &basis);
  xDBLMUL(&expected, &p, seven, &q, two, &p_minus_q, &curve);
  if (!ec_test_same_x(&combined, &expected)) { failure = "xDBLMUL"; goto failed; }

  printf("  ECC arithmetic tests ............................................ PASSED\n");
  return true;

failed:
  printf("  ECC arithmetic tests... FAILED (%s)\n", failure);
  return false;
}

static bool ec_test_dlog(int is_two_power) {
  ec_curve_t curve;
  ec_point_t p, q, p_minus_q, result;
  ec_basis_t basis;
  digit_t k[NWORDS_ORDER], l[NWORDS_ORDER], got_k[NWORDS_ORDER], got_l[NWORDS_ORDER];

  ec_test_curve_a_zero(&curve);
  if (is_two_power)
    ec_curve_to_basis_2(&basis, &curve);
  else
    ec_curve_to_basis_3(&basis, &curve);
  copy_point(&p, &basis.P);
  copy_point(&q, &basis.Q);
  copy_point(&p_minus_q, &basis.PmQ);

  for (int iteration = 0; iteration < TEST_LOOPS; ++iteration) {
    digit_t i = (digit_t)(iteration % 8 + 1);
    ec_test_scalar(k, i);
    ec_test_scalar(l, i + 1);
    xDBLMUL(&result, &p, k, &q, l, &p_minus_q, &curve);
    if (is_two_power)
      ec_dlog_2(got_k, got_l, &basis, &result, &curve);
    else
      ec_dlog_3(got_k, got_l, &basis, &result, &curve);
    if (memcmp(k, got_k, sizeof(k)) != 0 ||
        memcmp(l, got_l, sizeof(l)) != 0)
      return false;
  }
  return true;
}

bool dlog_test(void) {
  printf("\n--------------------------------------------------------------------------------------------------------\n\n");
  printf("Testing dlog functions: \n\n");

  if (!ec_test_dlog(1)) {
    printf("  dlog2 tests... FAILED\n");
    return false;
  }
  printf("  dlog2 tests ..................................................... PASSED\n");

  if (!ec_test_dlog(0)) {
    printf("  dlog3 tests... FAILED\n");
    return false;
  }
  printf("  dlog3 tests ..................................................... PASSED\n");
  return true;
}
