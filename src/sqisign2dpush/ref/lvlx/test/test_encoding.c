#include <sqisign2dpush.h>
#include <string.h>
#include <stdlib.h>

#define CHECK(c) do { if (!(c)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c); \
    ok = 0; goto done; } } while (0)

static void integer_bytes(unsigned char *dst, size_t size, const ibz_t *x)
{
    memset(dst, 0, size);
    mpz_export(dst, NULL, -1, 1, 0, 0, *x);
}

static int curve_equal(const ec_curve_t *a, const ec_curve_t *b)
{
    fp2_t lhs, rhs;
    fp2_mul(&lhs, &a->A, &b->C);
    fp2_mul(&rhs, &b->A, &a->C);
    return fp2_is_equal(&lhs, &rhs);
}

static int test_signature_format(void)
{
    int ok = 1;
    signature_t sig, decoded;
    secret_sig_init(&sig); secret_sig_init(&decoded);
    unsigned char buf[SIGNATURE_BYTES+1], saved[SIGNATURE_BYTES], bad[SIGNATURE_BYTES+1];
    unsigned char expected[SIGNATURE_BYTES];
    memset(expected, 0, sizeof(expected));
    memset(&sig.E_aux, 0, sizeof(sig.E_aux));
    fp_set(sig.E_aux.A.re, 6); fp_set(sig.E_aux.A.im, 7);
    fp2_set_one(&sig.E_aux.C);
    sig.n1 = TORSION_PLUS_EVEN_POWER > 256 ? 256 : 1;
    sig.challenge[0] = 5;
    sig.hint_aux = 0x12; sig.hint_chall = 0x34;
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) ibz_set(&sig.mat_rsp[i][j], 1+2*i+j);
    /* A hand-constructed byte vector checks ordinary (not Montgomery) field
     * values, row-major ordering, unaligned output, and little endian n1. */
    fp_encode(expected, &sig.E_aux.A.re);
    fp_encode(expected + FP_ENCODED_BYTES, &sig.E_aux.A.im);
    size_t pos = FP2_ENCODED_BYTES;
    expected[pos] = sig.n1 & 255;
    if (SIGNATURE_N1_BYTES == 2) expected[pos+1] = sig.n1 >> 8;
    pos += SIGNATURE_N1_BYTES;
    for (int i = 0; i < 4; i++) expected[pos+i*TORSION_2POWER_BYTES] = i+1;
    pos += 4*TORSION_2POWER_BYTES;
    expected[pos] = 5;
    pos += TORSION_3POWER_BYTES;
    expected[pos++] = 0x12; expected[pos++] = 0x34;
    CHECK(signature_to_bytes(buf+1, SIGNATURE_BYTES, &sig));
    CHECK(!memcmp(buf+1, expected, sizeof(expected)));
    CHECK(signature_from_bytes(&decoded, buf+1, SIGNATURE_BYTES));
    CHECK(decoded.n1 == sig.n1 && curve_equal(&decoded.E_aux, &sig.E_aux));
    CHECK(signature_to_bytes(saved, sizeof(saved), &decoded));
    for (size_t len = 0; len < SIGNATURE_BYTES; len++) CHECK(!signature_from_bytes(&decoded, saved, len));
    CHECK(!signature_from_bytes(&decoded, buf, SIGNATURE_BYTES+1));
    CHECK(!signature_to_bytes(buf, SIGNATURE_BYTES-1, &sig));

    memcpy(bad, saved, SIGNATURE_BYTES);
    memset(bad, 0xff, FP_ENCODED_BYTES);
    CHECK(!signature_from_bytes(&decoded, bad, SIGNATURE_BYTES));
    memcpy(bad, saved, SIGNATURE_BYTES);
    memset(bad, 0, FP2_ENCODED_BYTES); bad[0] = 2;
    CHECK(!signature_from_bytes(&decoded, bad, SIGNATURE_BYTES));
    memcpy(bad, saved, SIGNATURE_BYTES);
    bad[FP2_ENCODED_BYTES] = TORSION_PLUS_EVEN_POWER & 255;
    if (SIGNATURE_N1_BYTES == 2) bad[FP2_ENCODED_BYTES+1] = TORSION_PLUS_EVEN_POWER >> 8;
    CHECK(!signature_from_bytes(&decoded, bad, SIGNATURE_BYTES));
    memcpy(bad, saved, SIGNATURE_BYTES);
    integer_bytes(bad+FP2_ENCODED_BYTES+SIGNATURE_N1_BYTES, TORSION_2POWER_BYTES, &TORSION_PLUS_2POWER);
    CHECK(!signature_from_bytes(&decoded, bad, SIGNATURE_BYTES));
    memcpy(bad, saved, SIGNATURE_BYTES);
    integer_bytes(bad+FP2_ENCODED_BYTES+SIGNATURE_N1_BYTES+4*TORSION_2POWER_BYTES,
                  TORSION_3POWER_BYTES, &TORSION_PLUS_3POWER);
    CHECK(!signature_from_bytes(&decoded, bad, SIGNATURE_BYTES));
    CHECK(signature_to_bytes(buf, SIGNATURE_BYTES, &decoded));
    CHECK(!memcmp(buf, saved, SIGNATURE_BYTES)); /* Failed decodes are atomic. */

    ibz_set(&sig.mat_rsp[0][0], -1);
    memset(buf, 0x5a, sizeof(buf));
    CHECK(!signature_to_bytes(buf, SIGNATURE_BYTES, &sig));
    for (size_t i = 0; i < sizeof(buf); i++) CHECK(buf[i] == 0x5a);
    ibz_sub(&sig.mat_rsp[0][0], &TORSION_PLUS_2POWER, &ibz_const_one);
    ibz_t chl;
    ibz_init(&chl);
    ibz_sub(&chl, &TORSION_PLUS_3POWER, &ibz_const_one);
    ibz_to_digit_array(sig.challenge, &chl);
    ibz_finalize(&chl);
    sig.n1 = TORSION_PLUS_EVEN_POWER-1;
    CHECK(signature_to_bytes(buf, SIGNATURE_BYTES, &sig));
    CHECK(signature_from_bytes(&decoded, buf, SIGNATURE_BYTES));
    CHECK(decoded.n1 == sig.n1 && !ibz_cmp(&decoded.mat_rsp[0][0], &sig.mat_rsp[0][0]));
done:
    secret_sig_finalize(&sig); secret_sig_finalize(&decoded);
    return ok;
}

static int test_key_roundtrip(void)
{
    int ok = 1;
    public_key_t pk, decoded_pk, sentinel;
    secret_key_t sk, decoded_sk;
    secret_key_init(&sk); secret_key_init(&decoded_sk);
    unsigned char pkbuf[PUBLICKEY_BYTES], pkagain[PUBLICKEY_BYTES];
    unsigned char skbuf[SECRETKEY_BYTES], again[SECRETKEY_BYTES], bad[SECRETKEY_BYTES+1];
    for (int run = 0; run < 3; run++) {
        protocols_keygen(&pk, &sk, NULL);
        CHECK(public_key_to_bytes(pkbuf, sizeof(pkbuf), &pk));
        CHECK(public_key_from_bytes(&decoded_pk, pkbuf, sizeof(pkbuf)));
        CHECK(curve_equal(&pk.curve, &decoded_pk.curve));
        CHECK(pk.hint_pk_three == decoded_pk.hint_pk_three);
        CHECK(secret_key_to_bytes(skbuf, sizeof(skbuf), &sk, &pk));
        CHECK(secret_key_from_bytes(&decoded_sk, &decoded_pk, skbuf, sizeof(skbuf)));
        CHECK(quat_lideal_equals(&sk.secret_ideal_two, &decoded_sk.secret_ideal_two, &QUATALG_PINFTY));
        CHECK(ec_is_equal(&sk.phi_sk2_three.P, &decoded_sk.phi_sk2_three.P));
        CHECK(ec_is_equal(&sk.phi_sk2_three.Q, &decoded_sk.phi_sk2_three.Q));
        CHECK(ec_is_equal(&sk.phi_sk2_three.PmQ, &decoded_sk.phi_sk2_three.PmQ));
        CHECK(secret_key_to_bytes(again, sizeof(again), &decoded_sk, &decoded_pk));
        CHECK(!memcmp(again, skbuf, sizeof(skbuf)));
    }
    sentinel = decoded_pk;
    for (size_t len = 0; len < PUBLICKEY_BYTES; len++) CHECK(!public_key_from_bytes(&decoded_pk, pkbuf, len));
    for (size_t len = 0; len < SECRETKEY_BYTES; len++) CHECK(!secret_key_from_bytes(&decoded_sk, &decoded_pk, skbuf, len));
    CHECK(!secret_key_from_bytes(&decoded_sk, &decoded_pk, skbuf, SECRETKEY_BYTES+1));
    CHECK(secret_key_to_bytes(again, sizeof(again), &decoded_sk, &decoded_pk));
    CHECK(!memcmp(again, skbuf, sizeof(skbuf)));
    CHECK(curve_equal(&sentinel.curve, &decoded_pk.curve));
    memcpy(bad, pkbuf, sizeof(pkbuf));
    integer_bytes(bad, FP_ENCODED_BYTES, &CHARACTERISTIC);
    CHECK(!public_key_from_bytes(&decoded_pk, bad, PUBLICKEY_BYTES));
    memcpy(bad, pkbuf, sizeof(pkbuf)); bad[FP2_ENCODED_BYTES] = 0;
    CHECK(!public_key_from_bytes(&decoded_pk, bad, PUBLICKEY_BYTES));
    CHECK(public_key_to_bytes(pkagain, sizeof(pkagain), &decoded_pk));
    CHECK(!memcmp(pkagain, pkbuf, sizeof(pkbuf)));
    /* Projective scaling and cached A24 do not affect the encoded curve. */
    fp2_t scale;
    fp2_set(&scale, 9);
    fp2_mul(&pk.curve.A, &pk.curve.A, &scale);
    fp2_mul(&pk.curve.C, &pk.curve.C, &scale);
    CHECK(public_key_to_bytes(pkagain, sizeof(pkagain), &pk));
    CHECK(!memcmp(pkagain, pkbuf, sizeof(pkbuf)));
done:
    secret_key_finalize(&sk); secret_key_finalize(&decoded_sk);
    return ok;
}

int main(void)
{
    randombytes_init((unsigned char *)"some", (unsigned char *)"string", 128);
    if (!test_signature_format() || !test_key_roundtrip()) return 1;
    printf("Encoding tests passed: pk=%d, sk=%d, signature=%d bytes\n",
           PUBLICKEY_BYTES, SECRETKEY_BYTES, SIGNATURE_BYTES);
    return 0;
}
