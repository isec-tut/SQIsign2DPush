/* SPDX-License-Identifier: Apache-2.0
 * Shared API for the generated unsaturated Montgomery primitives.
 * All arithmetic inputs and outputs have value in [0,2p), with fully
 * propagated FP_RADIX-bit limbs. All operations permit exact input aliasing.
 * RADIX remains the machine/integer word width (64).
 */
#if RADIX != 64 || FP_RADIX != Radix || NWORDS_FIELD != Nlimbs
#error "Inconsistent reference field representation"
#endif

void fp_add(digit_t *out, const digit_t *a, const digit_t *b) { modadd(a, b, out); }
void fp_sub(digit_t *out, const digit_t *a, const digit_t *b) { modsub(a, b, out); }
void fp_neg(digit_t *out, const digit_t *a) { modneg(a, out); }
void fp_mul(digit_t *out, const digit_t *a, const digit_t *b) { modmul(a, b, out); }
void fp_sqr(digit_t *out, const digit_t *a) { modsqr(a, out); }
void fp_div3(digit_t *out, const digit_t *a) { modmul(a, FP_THREE_INV, out); }
void fp_set_zero(digit_t *out) { memset(out, 0, sizeof(fp_t)); }
void fp_set_one(digit_t *out) { memcpy(out, FP_ONE, sizeof(fp_t)); }
void fp_mont_setone(digit_t *out) { fp_set_one(out); }

/* Pack/unpack only at the legacy integer API boundary. The temporary makes
 * conversions safe in place. High padding words of ordinary outputs are zero. */
static void fp_pack(digit_t *out, const digit_t *a)
{
    fp_t t;
    const digit_t mask = (UINT64_C(1) << FP_RADIX) - 1;
    for (unsigned i = 0; i < NWORDS_FIELD; i++) {
        unsigned bit = i * FP_RADIX, word = bit / 64, shift = bit % 64;
        digit_t v = word < NWORDS_ORDER ? a[word] >> shift : 0;
        if (shift && word + 1 < NWORDS_ORDER)
            v |= a[word + 1] << (64 - shift);
        t[i] = v & mask;
    }
    memcpy(out, t, sizeof(t));
}

static void fp_unpack(digit_t *out, const digit_t *a)
{
    fp_t t = {0};
    for (unsigned i = 0; i < NWORDS_FIELD; i++) {
        unsigned bit = i * FP_RADIX, word = bit / 64, shift = bit % 64;
        t[word] |= a[i] << shift;
        if (shift && word + 1 < NWORDS_FIELD)
            t[word + 1] |= a[i] >> (64 - shift);
    }
    memcpy(out, t, sizeof(t));
}

void fp_tomont(digit_t *out, const digit_t *a)
{
    fp_t t;
    fp_pack(t, a); /* a is a canonical saturated ordinary integer. */
    nres(t, out);
}

void fp_frommont(digit_t *out, const digit_t *a)
{
    fp_t t;
    redc(a, t);
    fp_unpack(out, t);
}

void fp_set_small(digit_t *out, const digit_t val)
{
    fp_t t = {0};
    t[0] = val;
    fp_tomont(out, t);
}

bool fp_is_equal(const digit_t *a, const digit_t *b)
{
    fp_t x, y;
    digit_t v = 0;
    memcpy(x, a, sizeof(x));
    memcpy(y, b, sizeof(y));
    (void)modfsb(x);
    (void)modfsb(y);
    for (unsigned i = 0; i < NWORDS_FIELD; i++) v |= x[i] ^ y[i];
    return is_digit_zero_ct(v);
}

bool fp_is_zero(const digit_t *a)
{
    fp_t x;
    digit_t v = 0;
    memcpy(x, a, sizeof(x));
    (void)modfsb(x);
    for (unsigned i = 0; i < NWORDS_FIELD; i++) v |= x[i];
    return is_digit_zero_ct(v);
}

void fp_encode(void *dst, const fp_t *a)
{
    uint8_t *bytes = dst;
    fp_t t;
    fp_frommont(t, *a);
    for (unsigned i = 0; i < FP_ENCODED_BYTES; i++)
        bytes[i] = (uint8_t)(t[i / 8] >> (8 * (i % 8)));
}

uint32_t fp_decode(fp_t *out, const void *src)
{
    const uint8_t *bytes = src;
    extern const digit_t p[NWORDS_FIELD];
    fp_t t = {0};
    digit_t borrow = 0;
    for (unsigned i = 0; i < FP_ENCODED_BYTES; i++)
        t[i / 8] |= (digit_t)bytes[i] << (8 * (i % 8));
    for (unsigned i = 0; i < NWORDS_ORDER; i++) {
        borrow = (t[i] < p[i]) | ((t[i] == p[i]) & borrow);
    }
    digit_t mask = (digit_t)0 - borrow;
    for (unsigned i = 0; i < NWORDS_FIELD; i++) t[i] &= mask;
    fp_tomont(*out, t);
    return (uint32_t)mask;
}

/* Arbitrary-length little-endian reduction; fixed work per public byte.
 * Horner avoids assumptions about unused bits in a full encoded block. */
void fp_decode_reduce(fp_t *out, const void *src, size_t len)
{
    const uint8_t *bytes = src;
    fp_t acc = {0}, digit;
    while (len) {
        fp_set_small(digit, bytes[--len]);
        modmul(acc, FP_BYTE_BASE, acc);
        modadd(acc, digit, acc);
    }
    memcpy(*out, acc, sizeof(acc));
}

/* The protocol previously hashed saturated Montgomery memory. Normalize and
 * explicitly encode the same old R=2^(64*NWORDS_ORDER) residue in LE order.
 * This also makes hashing independent of redundant representatives and ABI. */
void fp_encode_legacy_hash(void *dst, const digit_t *a)
{
    fp_t t;
    modmul(a, FP_LEGACY_R, t);
    fp_encode(dst, &t);
}
