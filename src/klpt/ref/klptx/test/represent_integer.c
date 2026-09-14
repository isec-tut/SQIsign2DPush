#include <klpt.h>
#include <rng.h>

/* Checks stay active in Release builds. */
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
    ok = 0; goto cleanup; } } while (0)

static int test_two_squares(void)
{
    int ok = 1;
    ibz_t n, saved, x, y, sum, temp;
    ibz_init(&n); ibz_init(&saved); ibz_init(&x); ibz_init(&y);
    ibz_init(&sum); ibz_init(&temp);
    /* Independent exhaustive oracle, below the first unhandled large square. */
    for (int value = -1; value <= 1000; value++) {
        int expected = 0;
        for (int a = 0; a*a <= value && value > 0; a++)
            for (int b = 0; a*a+b*b <= value; b++)
                expected |= a*a+b*b == value;
        ibz_set(&n, value);
        CHECK(ibz_sum_of_two_squares(&x, &y, &n, 32) == expected);
        CHECK(ibz_get(&n) == value);
        if (expected) {
            ibz_mul(&sum, &x, &x);
            ibz_mul(&temp, &y, &y);
            ibz_add(&sum, &sum, &temp);
            CHECK(ibz_cmp(&sum, &n) == 0);
        } else {
            CHECK(ibz_is_zero(&x) && ibz_is_zero(&y));
        }
    }
    /* Residual composite 101^2 is deliberately outside this bounded solver. */
    ibz_set(&n, 10201);
    CHECK(!ibz_sum_of_two_squares(&x, &y, &n, 32));
    CHECK(ibz_is_zero(&x) && ibz_is_zero(&y));
    /* A large smooth part, including even valuations at primes 3 mod 4. */
    ibz_set(&n, 3);
    ibz_pow(&n, &n, 40);
    ibz_set(&temp, 5*101);
    ibz_mul(&n, &n, &temp);
    ibz_mul_2exp(&n, &n, 81);
    ibz_copy(&saved, &n);
    CHECK(ibz_sum_of_two_squares(&x, &y, &n, 32));
    ibz_mul(&sum, &x, &x);
    ibz_mul(&temp, &y, &y);
    ibz_add(&sum, &sum, &temp);
    CHECK(ibz_cmp(&sum, &saved) == 0 && ibz_cmp(&n, &saved) == 0);
    ibz_set(&temp, 3);
    ibz_mul(&n, &n, &temp);
    CHECK(!ibz_sum_of_two_squares(&x, &y, &n, 32));
cleanup:
    ibz_finalize(&n); ibz_finalize(&saved); ibz_finalize(&x); ibz_finalize(&y);
    ibz_finalize(&sum); ibz_finalize(&temp);
    return ok;
}

static int test_representation(void)
{
    int ok = 1;
    const int small_norms[] = {1, 2, 5, 13, 65};
    ibz_t n, saved, norm;
    ibq_t rational_norm;
    quat_alg_elem_t gamma;
    quat_alg_coord_t coords;
    ibz_init(&n); ibz_init(&saved); ibz_init(&norm);
    ibq_init(&rational_norm); quat_alg_elem_init(&gamma);
    quat_alg_coord_init(&coords);
    for (int trial = 0; trial < 37; trial++) {
        if (trial < 5) {
            ibz_set(&n, small_norms[trial]);
        } else if (trial % 2) {
            /* The norm used by DoublePath. */
            ibz_mul(&n, &TORSION_PLUS_2POWER, &TORSION_PLUS_3POWER);
            ibz_mul(&n, &n, &n);
        } else {
            /* Large odd norms, independent of the smooth DoublePath norm. */
            ibz_mul(&n, &QUATALG_PINFTY.p, &QUATALG_PINFTY.p);
            ibz_set(&norm, 2*trial);
            ibz_add(&n, &n, &norm);
        }
        ibz_copy(&saved, &n);
        CHECK(represent_integer(&gamma, &n, &QUATALG_PINFTY));
        CHECK(ibz_cmp(&n, &saved) == 0);
        quat_alg_norm(&rational_norm, &gamma, &QUATALG_PINFTY);
        CHECK(ibq_to_ibz(&norm, &rational_norm));
        CHECK(ibz_cmp(&norm, &saved) == 0);
        CHECK(quat_lattice_contains(&coords, &STANDARD_EXTREMAL_ORDER.order, &gamma, &QUATALG_PINFTY));
        CHECK(quat_alg_is_primitive(&gamma, &STANDARD_EXTREMAL_ORDER.order, &QUATALG_PINFTY));
    }
    for (int value = -1; value <= 0; value++) {
        ibz_set(&n, value);
        quat_alg_scalar(&gamma, &ibz_const_one, &ibz_const_one);
        CHECK(!represent_integer(&gamma, &n, &QUATALG_PINFTY));
        CHECK(ibz_get(&n) == value);
        for (int i = 0; i < 4; i++) CHECK(ibz_is_zero(&gamma.coord[i]));
        CHECK(ibz_is_one(&gamma.denom));
    }
    /* 9 has only nonprimitive representations when 4N < p. */
    ibz_set(&n, 9);
    CHECK(!represent_integer(&gamma, &n, &QUATALG_PINFTY));
    CHECK(ibz_get(&n) == 9);
    for (int i = 0; i < 4; i++) CHECK(ibz_is_zero(&gamma.coord[i]));
cleanup:
    ibz_finalize(&n); ibz_finalize(&saved); ibz_finalize(&norm);
    ibq_finalize(&rational_norm); quat_alg_elem_finalize(&gamma);
    quat_alg_coord_finalize(&coords);
    return ok;
}

int main(void)
{
    unsigned char entropy[48], personalization[48] = {0};
    for (int i = 0; i < 48; i++) entropy[i] = (unsigned char)i;
    randombytes_init(entropy, personalization, 256);
    if (!test_two_squares() || !test_representation()) return 1;
    puts("Two-square and exact primitive representation tests passed");
    return 0;
}
