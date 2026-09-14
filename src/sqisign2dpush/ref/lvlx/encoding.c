/* SQIsign2DPush binary format. See docs/encoding-ja.md.
 * Component conventions follow SQIsign v2.0 section 4.6; the object layout is
 * specific to 2DPush. No Montgomery limbs, GMP storage, or pointers are written.
 */
#include <sqisign2dpush.h>
#include <string.h>

/* Fixed-width little-endian integers; signed values use two's complement.
 * Range checks precede mpz_export, which otherwise has no buffer-size argument.
 */
static int put_integer(unsigned char *out, const ibz_t *x, size_t size, int sign)
{
    ibz_t value, bound;
    ibz_init(&value); ibz_init(&bound);
    ibz_mul_2exp(&bound, &ibz_const_one, 8*size - sign);
    int ok = ibz_cmp(x, &bound) < 0;
    if (sign) ibz_neg(&bound, &bound);
    else ibz_set(&bound, 0);
    ok &= ibz_cmp(x, &bound) >= 0;
    if (ok) {
        ibz_copy(&value, x);
        if (ibz_cmp(x, &ibz_const_zero) < 0) {
            ibz_mul_2exp(&bound, &ibz_const_one, 8*size);
            ibz_add(&value, &value, &bound);
        }
        memset(out, 0, size);
        mpz_export(out, NULL, -1, 1, 0, 0, value);
    }
    ibz_finalize(&value); ibz_finalize(&bound);
    return ok;
}

static void get_integer(ibz_t *x, const unsigned char *in, size_t size, int sign)
{
    mpz_import(*x, size, -1, 1, 0, 0, in);
    if (sign && (in[size-1] & 128)) {
        ibz_t bound;
        ibz_init(&bound);
        ibz_mul_2exp(&bound, &ibz_const_one, 8*size);
        ibz_sub(x, x, &bound);
        ibz_finalize(&bound);
    }
}

static int residue(const ibz_t *x, const ibz_t *modulus)
{
    return ibz_cmp(x, &ibz_const_zero) >= 0 && ibz_cmp(x, modulus) < 0;
}

static int put_affine(unsigned char *out, const fp2_t *x, const fp2_t *z)
{
    if (fp2_is_zero(z)) return 0;
    fp2_t a = *z;
    fp2_inv(&a);
    fp2_mul(&a, x, &a);
    fp2_encode(out, &a);
    return 1;
}

static int nonsingular(const fp2_t *a)
{
    fp2_t square, four;
    fp2_sqr(&square, a);
    fp2_set_one(&four);
    fp2_add(&four, &four, &four);
    fp2_add(&four, &four, &four);
    return !fp2_is_equal(&square, &four);
}

static int get_curve(ec_curve_t *curve, const unsigned char *in)
{
    memset(curve, 0, sizeof(*curve));
    if (!fp2_decode(&curve->A, in) || !nonsingular(&curve->A)) return 0;
    fp2_set_one(&curve->C);
    ec_normalize_curve_and_A24(curve);
    return 1;
}

static int put_curve(unsigned char *out, const ec_curve_t *curve)
{
    ec_curve_t normalized;
    return put_affine(out, &curve->A, &curve->C) && get_curve(&normalized, out);
}

static int same_curve(const ec_curve_t *a, const ec_curve_t *b)
{
    fp2_t lhs, rhs;
    if (fp2_is_zero(&a->C) || fp2_is_zero(&b->C)) return 0;
    fp2_mul(&lhs, &a->A, &b->C);
    fp2_mul(&rhs, &b->A, &a->C);
    return fp2_is_equal(&lhs, &rhs);
}

int public_key_to_bytes(unsigned char *out, size_t size, const public_key_t *pk)
{
    unsigned char buf[PUBLICKEY_BYTES];
    if (!out || !pk || size != sizeof(buf) || !put_curve(buf, &pk->curve)) return 0;
    /* Both replay counters start at one. */
    if (!(pk->hint_pk_three >> 8) || !(pk->hint_pk_three & 255)) return 0;
    buf[FP2_ENCODED_BYTES] = pk->hint_pk_three & 255;
    buf[FP2_ENCODED_BYTES+1] = pk->hint_pk_three >> 8;
    memcpy(out, buf, sizeof(buf));
    return 1;
}

int public_key_from_bytes(public_key_t *pk, const unsigned char *in, size_t size)
{
    public_key_t tmp;
    if (!pk || !in || size != PUBLICKEY_BYTES || !get_curve(&tmp.curve, in)) return 0;
    tmp.hint_pk_three = (uint16_t)in[FP2_ENCODED_BYTES] |
                       ((uint16_t)in[FP2_ENCODED_BYTES+1] << 8);
    if (!(tmp.hint_pk_three >> 8) || !(tmp.hint_pk_three & 255)) return 0;
    *pk = tmp;
    return 1;
}

int signature_to_bytes(unsigned char *out, size_t size, const signature_t *sig)
{
    unsigned char buf[SIGNATURE_BYTES];
    if (!out || !sig || size != sizeof(buf) || sig->n1 >= TORSION_PLUS_EVEN_POWER ||
        !put_curve(buf, &sig->E_aux)) return 0;
    size_t pos = FP2_ENCODED_BYTES;
    for (size_t i = 0; i < SIGNATURE_N1_BYTES; i++) buf[pos++] = sig->n1 >> (8*i);
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) {
        if (!residue(&sig->mat_rsp[i][j], &TORSION_PLUS_2POWER) ||
            !put_integer(buf+pos, &sig->mat_rsp[i][j], TORSION_2POWER_BYTES, 0)) return 0;
        pos += TORSION_2POWER_BYTES;
    }
    ibz_t challenge;
    ibz_init(&challenge);
    ibz_copy_digit_array(&challenge, sig->challenge);
    int ok = residue(&challenge, &TORSION_PLUS_3POWER) &&
             put_integer(buf+pos, &challenge, TORSION_3POWER_BYTES, 0);
    ibz_finalize(&challenge);
    if (!ok) return 0;
    pos += TORSION_3POWER_BYTES;
    buf[pos++] = sig->hint_aux;
    buf[pos++] = sig->hint_chall;
    assert(pos == sizeof(buf));
    memcpy(out, buf, sizeof(buf));
    return 1;
}

int signature_from_bytes(signature_t *sig, const unsigned char *in, size_t size)
{
    if (!sig || !in || size != SIGNATURE_BYTES) return 0;
    int ok = 0;
    signature_t tmp;
    secret_sig_init(&tmp);
    ibz_t challenge;
    ibz_init(&challenge);
    if (!get_curve(&tmp.E_aux, in)) goto done;
    size_t pos = FP2_ENCODED_BYTES;
    tmp.n1 = 0;
    for (size_t i = 0; i < SIGNATURE_N1_BYTES; i++) tmp.n1 |= (uint16_t)in[pos++] << (8*i);
    if (tmp.n1 >= TORSION_PLUS_EVEN_POWER) goto done;
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) {
        get_integer(&tmp.mat_rsp[i][j], in+pos, TORSION_2POWER_BYTES, 0);
        if (!residue(&tmp.mat_rsp[i][j], &TORSION_PLUS_2POWER)) goto done;
        pos += TORSION_2POWER_BYTES;
    }
    get_integer(&challenge, in+pos, TORSION_3POWER_BYTES, 0);
    if (!residue(&challenge, &TORSION_PLUS_3POWER)) goto done;
    ibz_to_digit_array(tmp.challenge, &challenge);
    pos += TORSION_3POWER_BYTES;
    tmp.hint_aux = in[pos++];
    tmp.hint_chall = in[pos++];
    assert(pos == size);
    sig->E_aux = tmp.E_aux;
    sig->n1 = tmp.n1;
    ibz_mat_2x2_copy(&sig->mat_rsp, &tmp.mat_rsp);
    memcpy(sig->challenge, tmp.challenge, sizeof(sig->challenge));
    sig->hint_aux = tmp.hint_aux;
    sig->hint_chall = tmp.hint_chall;
    ok = 1;
done:
    ibz_finalize(&challenge);
    secret_sig_finalize(&tmp);
    return ok;
}

/* Recover I = O0*gamma + O0*2^(2e1). Its norm and gamma's norm are fixed
 * by DoublePath. In particular, gamma's denominator cannot be discarded:
 * unlike v2's odd ideal norm, this ideal norm is even.
 */
static int recover_ideal(quat_left_ideal_t *ideal, const quat_alg_elem_t *gamma)
{
    if (ibz_cmp(&gamma->denom, &ibz_const_zero) <= 0) return 0;
    if (!ibz_is_one(&gamma->denom) &&
        ibz_cmp(&gamma->denom, &ibz_const_two) != 0) return 0;
    if (!quat_lattice_contains(NULL, &MAXORD_O0, gamma, &QUATALG_PINFTY) ||
        !quat_alg_is_primitive(gamma, &MAXORD_O0, &QUATALG_PINFTY)) return 0;
    int ok;
    ibz_t norm, expected, ideal_norm, content;
    ibq_t rational;
    ibz_init(&norm); ibz_init(&expected); ibz_init(&ideal_norm); ibz_init(&content);
    ibq_init(&rational);
    ibz_content(&content, &gamma->coord);
    ibz_gcd(&content, &content, &gamma->denom);
    if (ibz_is_zero(&content)) {
        ibz_finalize(&norm); ibz_finalize(&expected); ibz_finalize(&ideal_norm);
        ibz_finalize(&content); ibq_finalize(&rational);
        return 0;
    }
    quat_alg_norm(&rational, gamma, &QUATALG_PINFTY);
    ibz_mul(&expected, &TORSION_PLUS_2POWER, &TORSION_PLUS_3POWER);
    ibz_mul(&expected, &expected, &expected);
    ok = ibz_is_one(&content) && ibq_to_ibz(&norm, &rational) && !ibz_cmp(&norm, &expected);
    if (ok) {
        ibz_mul(&ideal_norm, &TORSION_PLUS_2POWER, &TORSION_PLUS_2POWER);
        quat_lideal_create_from_primitive(ideal, gamma, &ideal_norm, &MAXORD_O0, &QUATALG_PINFTY);
        ok = !ibz_cmp(&ideal->norm, &ideal_norm);
    }
    ibz_finalize(&norm); ibz_finalize(&expected); ibz_finalize(&ideal_norm); ibz_finalize(&content);
    ibq_finalize(&rational);
    return ok;
}

static int invertible_three(const ibz_mat_2x2_t *matrix)
{
    ibz_t det, tmp;
    ibz_init(&det); ibz_init(&tmp);
    ibz_mul(&det, &(*matrix)[0][0], &(*matrix)[1][1]);
    ibz_mul(&tmp, &(*matrix)[0][1], &(*matrix)[1][0]);
    ibz_sub(&det, &det, &tmp);
    int ok = ibz_mod_ui(&det, 3) != 0;
    ibz_finalize(&det); ibz_finalize(&tmp);
    return ok;
}

/* Validate an affine differential basis of full 2^e1 torsion, including
 * independence and x(P-Q). This field's historical name is phi_sk2_three. */
static int valid_basis(const ec_basis_t *basis, const ec_curve_t *curve)
{
    const ec_point_t *points[3] = {&basis->P, &basis->Q, &basis->PmQ};
    ec_point_t tops[3], zero;
    fp2_t f[2], delta, lhs, rhs, tmp;
    for (int i = 0; i < 3; i++) {
        if (!ec_is_on_curve(curve, points[i])) return 0;
        tops[i] = *points[i];
        for (unsigned j = 1; j < TORSION_PLUS_EVEN_POWER; j++) ec_dbl(&tops[i], curve, &tops[i]);
        if (fp2_is_zero(&tops[i].z)) return 0;
        ec_dbl(&zero, curve, &tops[i]);
        if (!fp2_is_zero(&zero.z)) return 0;
    }
    if (ec_is_equal(&tops[0], &tops[1])) return 0;
    for (int i = 0; i < 2; i++) {
        fp2_add(&f[i], &points[i]->x, &curve->A);
        fp2_mul(&f[i], &f[i], &points[i]->x);
        fp2_set_one(&tmp);
        fp2_add(&f[i], &f[i], &tmp);
        fp2_mul(&f[i], &f[i], &points[i]->x);
    }
    fp2_sub(&delta, &basis->P.x, &basis->Q.x);
    fp2_sqr(&delta, &delta);
    fp2_add(&lhs, &basis->PmQ.x, &curve->A);
    fp2_add(&lhs, &lhs, &basis->P.x);
    fp2_add(&lhs, &lhs, &basis->Q.x);
    fp2_mul(&lhs, &lhs, &delta);
    fp2_sub(&lhs, &lhs, &f[0]);
    fp2_sub(&lhs, &lhs, &f[1]);
    fp2_sqr(&lhs, &lhs);
    fp2_mul(&rhs, &f[0], &f[1]);
    fp2_add(&rhs, &rhs, &rhs);
    fp2_add(&rhs, &rhs, &rhs);
    return fp2_is_equal(&lhs, &rhs);
}

int secret_key_to_bytes(unsigned char *out, size_t size, const secret_key_t *sk,
                        const public_key_t *pk)
{
    unsigned char buf[SECRETKEY_BYTES];
    if (!out || !sk || !pk || size != sizeof(buf) || !same_curve(&sk->curve, &pk->curve) ||
        !public_key_to_bytes(buf, PUBLICKEY_BYTES, pk) ||
        ibz_cmp(&sk->two_to_three_transporter.denom, &ibz_const_zero) <= 0) return 0;
    int ok = 0;
    quat_alg_elem_t gamma;
    quat_left_ideal_t ideal;
    quat_alg_elem_init(&gamma); quat_left_ideal_init(&ideal);
    quat_alg_conj(&gamma, &sk->two_to_three_transporter);
    for (int i = 0; i < 4; i++) ibz_mul(&gamma.coord[i], &gamma.coord[i], &sk->secret_ideal_two.norm);
    quat_alg_normalize(&gamma);
    if (!recover_ideal(&ideal, &gamma) ||
        sk->secret_ideal_two.parent_order != &MAXORD_O0 ||
        !quat_lideal_equals(&ideal, &sk->secret_ideal_two, &QUATALG_PINFTY)) goto done;
    size_t pos = PUBLICKEY_BYTES;
    buf[pos++] = ibz_get(&gamma.denom);
    for (int i = 0; i < 4; i++) {
        if (!put_integer(buf+pos, &gamma.coord[i], SECRET_GAMMA_COORD_BYTES, 1)) goto done;
        pos += SECRET_GAMMA_COORD_BYTES;
    }
    if (!invertible_three(&sk->mat_BAcan_to_BA0_three)) goto done;
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) {
        if (!residue(&sk->mat_BAcan_to_BA0_three[i][j], &TORSION_PLUS_3POWER) ||
            !put_integer(buf+pos, &sk->mat_BAcan_to_BA0_three[i][j], TORSION_3POWER_BYTES, 0)) goto done;
        pos += TORSION_3POWER_BYTES;
    }
    const ec_point_t *points[3] = {&sk->phi_sk2_three.P, &sk->phi_sk2_three.Q, &sk->phi_sk2_three.PmQ};
    for (int i = 0; i < 3; i++) {
        if (!put_affine(buf+pos, &points[i]->x, &points[i]->z)) goto done;
        pos += FP2_ENCODED_BYTES;
    }
    assert(pos == sizeof(buf));
    /* Share all semantic checks with the decoder before committing output. */
    secret_key_t checked;
    public_key_t checked_pk;
    secret_key_init(&checked);
    ok = secret_key_from_bytes(&checked, &checked_pk, buf, sizeof(buf));
    secret_key_finalize(&checked);
    if (ok) memcpy(out, buf, sizeof(buf));
done:
    quat_alg_elem_finalize(&gamma); quat_left_ideal_finalize(&ideal);
    return ok;
}

int secret_key_from_bytes(secret_key_t *sk, public_key_t *pk,
                          const unsigned char *in, size_t size)
{
    if (!sk || !pk || !in || size != SECRETKEY_BYTES) return 0;
    int ok = 0;
    secret_key_t tmp;
    public_key_t tmp_pk;
    secret_key_init(&tmp);
    quat_alg_elem_t gamma;
    quat_alg_elem_init(&gamma);
    if (!public_key_from_bytes(&tmp_pk, in, PUBLICKEY_BYTES)) goto done;
    tmp.curve = tmp_pk.curve;
    size_t pos = PUBLICKEY_BYTES;
    ibz_set(&gamma.denom, in[pos++]);
    for (int i = 0; i < 4; i++) {
        get_integer(&gamma.coord[i], in+pos, SECRET_GAMMA_COORD_BYTES, 1);
        pos += SECRET_GAMMA_COORD_BYTES;
    }
    if (!recover_ideal(&tmp.secret_ideal_two, &gamma)) goto done;
    quat_alg_conj(&tmp.two_to_three_transporter, &gamma);
    ibz_mul(&tmp.two_to_three_transporter.denom, &tmp.two_to_three_transporter.denom,
            &tmp.secret_ideal_two.norm);
    quat_alg_normalize(&tmp.two_to_three_transporter);
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) {
        get_integer(&tmp.mat_BAcan_to_BA0_three[i][j], in+pos, TORSION_3POWER_BYTES, 0);
        if (!residue(&tmp.mat_BAcan_to_BA0_three[i][j], &TORSION_PLUS_3POWER)) goto done;
        pos += TORSION_3POWER_BYTES;
    }
    if (!invertible_three(&tmp.mat_BAcan_to_BA0_three)) goto done;
    ec_point_t *points[3] = {&tmp.phi_sk2_three.P, &tmp.phi_sk2_three.Q, &tmp.phi_sk2_three.PmQ};
    for (int i = 0; i < 3; i++) {
        if (!fp2_decode(&points[i]->x, in+pos)) goto done;
        fp2_set_one(&points[i]->z);
        pos += FP2_ENCODED_BYTES;
    }
    if (!valid_basis(&tmp.phi_sk2_three, &tmp.curve)) goto done;
    assert(pos == size);
    sk->curve = tmp.curve;
    quat_left_ideal_copy(&sk->secret_ideal_two, &tmp.secret_ideal_two);
    quat_alg_elem_copy(&sk->two_to_three_transporter, &tmp.two_to_three_transporter);
    ibz_mat_2x2_copy(&sk->mat_BAcan_to_BA0_three, &tmp.mat_BAcan_to_BA0_three);
    sk->phi_sk2_three = tmp.phi_sk2_three;
    *pk = tmp_pk;
    ok = 1;
done:
    quat_alg_elem_finalize(&gamma);
    secret_key_finalize(&tmp);
    return ok;
}
