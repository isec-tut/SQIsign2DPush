#include "include/fp.h"
#include <encoded_sizes.h>

/* Saturated ordinary modulus for integer conversion and binary GCD. */
const uint64_t p[NWORDS_FIELD] = {0xffffffffffffffff, 0xffffffffffffffff,
                                  0xa7ecc14ec3fa83c7, 0x62d7f37f9815e5fc};

void fp_set(
    digit_t *x,
    const digit_t val) { // Set field element x = val, where val has wordsize

  x[0] = val;
  for (unsigned int i = 1; i < NWORDS_FIELD; i++) {
    x[i] = 0;
  }
}

bool fp_is_equal(const digit_t *a, const digit_t *b) {
  digit_t r = 0;
  for (unsigned int i = 0; i < NWORDS_FIELD; i++) r |= a[i] ^ b[i];
  return (bool)is_digit_zero_ct(r);
}

bool fp_is_zero(const digit_t *a) {
  digit_t r = 0;
  for (unsigned int i = 0; i < NWORDS_FIELD; i++) r |= a[i];
  return (bool)is_digit_zero_ct(r);
}

void fp_set_zero(digit_t *x) { memset(x, 0, sizeof(fp_t)); }
void fp_set_small(digit_t *x, const digit_t val) { fp_set(x, val); fp_tomont(x, x); }
void fp_set_one(digit_t *x) { fp_set_small(x, 1); }

/* The reference backend has no precomputed Montgomery inverse of 3. */
void fp_div3(digit_t *out, const digit_t *in) {
  fp_t inv3;

  fp_set_small(inv3, 3);
  fp_inv(inv3);
  fp_mul(out, in, inv3);
}

void fp_copy(digit_t *out, const digit_t *a) {
  memcpy(out, a, NWORDS_FIELD * RADIX / 8);
}

void fp_select(digit_t *out, const digit_t *a, const digit_t *b, uint32_t c) {
  digit_t mask = (digit_t)0 - (digit_t)(c != 0);
  for (int i = 0; i < NWORDS_FIELD; i++) {
    out[i] = a[i] ^ (mask & (a[i] ^ b[i]));
  }
}

void MUL(digit_t *out, const digit_t a,
         const digit_t b) { // Digit multiplication, digit*digit -> 2-digit
                            // result
  register digit_t al, ah, bl, bh, temp;
  digit_t albl, albh, ahbl, ahbh, res1, res2, res3, carry;
  digit_t mask_low = (digit_t)(-1) >> (sizeof(digit_t) * 4),
          mask_high = (digit_t)(-1) << (sizeof(digit_t) * 4);

  al = a & mask_low;               // Low part
  ah = a >> (sizeof(digit_t) * 4); // High part
  bl = b & mask_low;
  bh = b >> (sizeof(digit_t) * 4);

  albl = al * bl;
  albh = al * bh;
  ahbl = ah * bl;
  ahbh = ah * bh;
  out[0] = albl & mask_low; // out00

  res1 = albl >> (sizeof(digit_t) * 4);
  res2 = ahbl & mask_low;
  res3 = albh & mask_low;
  temp = res1 + res2 + res3;
  carry = temp >> (sizeof(digit_t) * 4);
  out[0] ^= temp << (sizeof(digit_t) * 4); // out01

  res1 = ahbl >> (sizeof(digit_t) * 4);
  res2 = albh >> (sizeof(digit_t) * 4);
  res3 = ahbh & mask_low;
  temp = res1 + res2 + res3 + carry;
  out[1] = temp & mask_low; // out10
  carry = temp & mask_high;
  out[1] ^= (ahbh & mask_high) + carry; // out11
}

digit_t mp_shiftr(digit_t *x, const unsigned int shift,
                  const unsigned int nwords) { // Multiprecision right shift
  digit_t bit_out = x[0] & 1;

  for (unsigned int i = 0; i < nwords - 1; i++) {
    SHIFTR(x[i + 1], x[i], shift, x[i], RADIX);
  }
  x[nwords - 1] >>= shift;
  return bit_out;
}

void mp_shiftl(digit_t *x, const unsigned int shift,
               const unsigned int nwords) { // Multiprecision left shift

  for (int i = nwords - 1; i > 0; i--) {
    SHIFTL(x[i], x[i - 1], shift, x[i], RADIX);
  }
  x[0] <<= shift;
}

void fp_inv(digit_t *a) { // Modular inversion: a^(p-2)
                          // a^(p-2) = (a^((p-3)/4))^4 * a
  fp_t t;

  fp_exp3div4(t, a);
  fp_sqr(t, t);
  fp_sqr(t, t);
  fp_mul(a, t, a); // a^(p-2)
}

// helper macros and functions for binary GCD Legendre symbol calculation
static inline uint64_t sgnw(uint64_t x) {
  return (uint64_t)((int64_t)x >> 63);
}

static inline uint64_t lzcnt(uint64_t x) {
#if defined __LZCNT__
  return _lzcnt_u64(x);
#else
  uint64_t m, s;
  m = sgnw((x >> 32) - 1);
  s = m & 32;
  x = (x >> 32) ^ (m & (x ^ (x >> 32)));
  m = sgnw((x >> 16) - 1);
  s |= m & 16;
  x = (x >> 16) ^ (m & (x ^ (x >> 16)));
  m = sgnw((x >> 8) - 1);
  s |= m & 8;
  x = (x >> 8) ^ (m & (x ^ (x >> 8)));
  m = sgnw((x >> 4) - 1);
  s |= m & 4;
  x = (x >> 4) ^ (m & (x ^ (x >> 4)));
  m = sgnw((x >> 2) - 1);
  s |= m & 2;
  x = (x >> 2) ^ (m & (x ^ (x >> 2)));
  s += (2 - x) & ((x - 3) >> 2);
  return s;
#endif
}

static inline unsigned char inner_fp_sbb(unsigned char cc, uint64_t a, uint64_t b, uint64_t *d) {
  uint64_t tempReg = a - b;
  unsigned int borrowReg = (a < b) | ((cc != 0) & (tempReg == 0));
  *d = tempReg - (uint64_t)(cc != 0);
  return (unsigned char)borrowReg;
}

static inline unsigned char inner_fp_adc(unsigned char cc, uint64_t a, uint64_t b, uint64_t *d) {
  uint64_t tempReg = a + (uint64_t)(cc != 0);
  *d = b + tempReg;
  return (unsigned char)((tempReg < (uint64_t)(cc != 0)) | (*d < tempReg));
}

#define inner_fp_umul(lo, hi, x, y)                                                                                \
    do {                                                                                                               \
        unsigned __int128 umul_tmp;                                                                                    \
        umul_tmp = (unsigned __int128)(x) * (unsigned __int128)(y);                                                    \
        (lo) = (uint64_t)umul_tmp;                                                                                     \
        (hi) = (uint64_t)(umul_tmp >> 64);                                                                             \
    } while (0)

#define inner_fp_umul_x2(lo, hi, x1, y1, x2, y2)                                                                   \
    do {                                                                                                               \
        unsigned __int128 umul_tmp;                                                                                    \
        umul_tmp =                                                                                                     \
            (unsigned __int128)(x1) * (unsigned __int128)(y1) + (unsigned __int128)(x2) * (unsigned __int128)(y2);     \
        (lo) = (uint64_t)umul_tmp;                                                                                     \
        (hi) = (uint64_t)(umul_tmp >> 64);                                                                             \
    } while (0)

#define inner_fp_umul_x2_add(lo, hi, x1, y1, x2, y2, z)                                                            \
    do {                                                                                                               \
        unsigned __int128 umul_tmp;                                                                                    \
        umul_tmp = (unsigned __int128)(x1) * (unsigned __int128)(y1) +                                                 \
                   (unsigned __int128)(x2) * (unsigned __int128)(y2) + (unsigned __int128)(uint64_t)(z);               \
        (lo) = (uint64_t)umul_tmp;                                                                                     \
        (hi) = (uint64_t)(umul_tmp >> 64);                                                                             \
    } while (0)

static uint64_t lindiv31abs(digit_t *d, const digit_t *a, const digit_t *b, uint64_t f, uint64_t g) {
  uint64_t sf = sgnw(f);
  f = (f ^ sf) - sf;
  uint64_t sg = sgnw(g);
  g = (g ^ sg) - sg;

  uint64_t a0, a1, a2, a3, a4;
  uint64_t b0, b1, b2, b3, b4;
  unsigned char cc;

  cc = inner_fp_sbb(0, a[0] ^ sf, sf, &a0);
  cc = inner_fp_sbb(cc, a[1] ^ sf, sf, &a1);
  cc = inner_fp_sbb(cc, a[2] ^ sf, sf, &a2);
  cc = inner_fp_sbb(cc, a[3] ^ sf, sf, &a3);
  (void)inner_fp_sbb(cc, 0, 0, &a4);

  cc = inner_fp_sbb(0, b[0] ^ sg, sg, &b0);
  cc = inner_fp_sbb(cc, b[1] ^ sg, sg, &b1);
  cc = inner_fp_sbb(cc, b[2] ^ sg, sg, &b2);
  cc = inner_fp_sbb(cc, b[3] ^ sg, sg, &b3);
  (void)inner_fp_sbb(cc, 0, 0, &b4);

  uint64_t d0, d1, d2, d3, d4, t;
  inner_fp_umul_x2(d0, t, a0, f, b0, g);
  inner_fp_umul_x2_add(d1, t, a1, f, b1, g, t);
  inner_fp_umul_x2_add(d2, t, a2, f, b2, g, t);
  inner_fp_umul_x2_add(d3, t, a3, f, b3, g, t);
  d4 = t - (a4 & f) - (b4 & g);

  d0 = (d0 >> 31) | (d1 << 33);
  d1 = (d1 >> 31) | (d2 << 33);
  d2 = (d2 >> 31) | (d3 << 33);
  d3 = (d3 >> 31) | (d4 << 33);

  t = sgnw(d4);
  cc = inner_fp_sbb(0, d0 ^ t, t, &d0);
  cc = inner_fp_sbb(cc, d1 ^ t, t, &d1);
  cc = inner_fp_sbb(cc, d2 ^ t, t, &d2);
  (void)inner_fp_sbb(cc, d3 ^ t, t, &d3);

  d[0] = d0;
  d[1] = d1;
  d[2] = d2;
  d[3] = d3;
  return t;
}

bool fp_is_square(const digit_t *x) {
  fp_t a = {0}, b = {0};
  uint64_t xa, xb, f0, g0, f1, g1, ls;

  fp_frommont(a, x); /* GCD operates on saturated ordinary integers. */
  
  b[0] = p[0];
  b[1] = p[1];
  b[2] = p[2];
  b[3] = p[3];
  
  ls = 0;

  for (int i = 0; i < 15; i++) {
    uint64_t m3 = a[3] | b[3];
    uint64_t m2 = a[2] | b[2];
    uint64_t m1 = a[1] | b[1];
    uint64_t tnz3 = sgnw(m3 | -m3);
    uint64_t tnz2 = sgnw(m2 | -m2) & ~tnz3;
    uint64_t tnz1 = sgnw(m1 | -m1) & ~tnz3 & ~tnz2;
    uint64_t tnzm = (m3 & tnz3) | (m2 & tnz2) | (m1 & tnz1);
    uint64_t tnza = (a[3] & tnz3) | (a[2] & tnz2) | (a[1] & tnz1);
    uint64_t tnzb = (b[3] & tnz3) | (b[2] & tnz2) | (b[1] & tnz1);
    uint64_t snza = (a[2] & tnz3) | (a[1] & tnz2) | (a[0] & tnz1);
    uint64_t snzb = (b[2] & tnz3) | (b[1] & tnz2) | (b[0] & tnz1);

    int64_t s = lzcnt(tnzm);
    uint64_t sm = (uint64_t)((31 - s) >> 63);
    tnza ^= sm & (tnza ^ ((tnza << 32) | (snza >> 32)));
    tnzb ^= sm & (tnzb ^ ((tnzb << 32) | (snzb >> 32)));
    s -= 32 & sm;
    tnza <<= s;
    tnzb <<= s;

    uint64_t tzx = ~(tnz1 | tnz2 | tnz3);
    tnza |= a[0] & tzx;
    tnzb |= b[0] & tzx;
    xa = (a[0] & 0x7FFFFFFF) | (tnza & 0xFFFFFFFF80000000);
    xb = (b[0] & 0x7FFFFFFF) | (tnzb & 0xFFFFFFFF80000000);

    uint64_t fg0 = (uint64_t)1;
    uint64_t fg1 = (uint64_t)1 << 32;
    for (int j = 0; j < 29; j++) {
      uint64_t a_odd, swap, t0, t1, t2;
      unsigned char cc;
      a_odd = -(xa & 1);
      cc = inner_fp_sbb(0, xa, xb, &t0);
      (void)inner_fp_sbb(cc, 0, 0, &swap);
      swap &= a_odd;
      ls ^= swap & xa & xb;
      t1 = swap & (xa ^ xb);
      xa ^= t1;
      xb ^= t1;
      t2 = swap & (fg0 ^ fg1);
      fg0 ^= t2;
      fg1 ^= t2;
      xa -= a_odd & xb;
      fg0 -= a_odd & fg1;
      xa >>= 1;
      fg1 <<= 1;
      ls ^= (xb + 2) >> 1;
    }

    uint64_t fg0z = fg0 + 0x7FFFFFFF7FFFFFFF;
    uint64_t fg1z = fg1 + 0x7FFFFFFF7FFFFFFF;
    f0 = (fg0z & 0xFFFFFFFF) - (uint64_t)0x7FFFFFFF;
    g0 = (fg0z >> 32) - (uint64_t)0x7FFFFFFF;
    f1 = (fg1z & 0xFFFFFFFF) - (uint64_t)0x7FFFFFFF;
    g1 = (fg1z >> 32) - (uint64_t)0x7FFFFFFF;
    uint64_t a0 = (a[0] * f0 + b[0] * g0) >> 29;
    uint64_t b0 = (a[0] * f1 + b[0] * g1) >> 29;
    for (int j = 0; j < 2; j++) {
      uint64_t a_odd, swap, t0, t1, t2, t3;
      unsigned char cc;
      a_odd = -(xa & 1);
      cc = inner_fp_sbb(0, xa, xb, &t0);
      (void)inner_fp_sbb(cc, 0, 0, &swap);
      swap &= a_odd;
      ls ^= swap & a0 & b0;
      t1 = swap & (xa ^ xb);
      xa ^= t1;
      xb ^= t1;
      t2 = swap & (fg0 ^ fg1);
      fg0 ^= t2;
      fg1 ^= t2;
      t3 = swap & (a0 ^ b0);
      a0 ^= t3;
      b0 ^= t3;
      xa -= a_odd & xb;
      fg0 -= a_odd & fg1;
      a0 -= a_odd & b0;
      xa >>= 1;
      fg1 <<= 1;
      a0 >>= 1;
      ls ^= (b0 + 2) >> 1;
    }

    fg0 += 0x7FFFFFFF7FFFFFFF;
    fg1 += 0x7FFFFFFF7FFFFFFF;
    f0 = (fg0 & 0xFFFFFFFF) - (uint64_t)0x7FFFFFFF;
    g0 = (fg0 >> 32) - (uint64_t)0x7FFFFFFF;
    f1 = (fg1 & 0xFFFFFFFF) - (uint64_t)0x7FFFFFFF;
    g1 = (fg1 >> 32) - (uint64_t)0x7FFFFFFF;
    fp_t na = {0}, nb = {0};
    uint64_t nega = lindiv31abs(na, a, b, f0, g0);
    (void)lindiv31abs(nb, a, b, f1, g1);
    ls ^= nega & nb[0];
    fp_copy(a, na);
    fp_copy(b, nb);
  }

  xa = a[0];
  xb = b[0];
  for (int j = 0; j < 35; j++) {
    uint64_t a_odd, swap, t0, t1;
    unsigned char cc;
    a_odd = -(xa & 1);
    cc = inner_fp_sbb(0, xa, xb, &t0);
    (void)inner_fp_sbb(cc, 0, 0, &swap);
    swap &= a_odd;
    ls ^= swap & xa & xb;
    t1 = swap & (xa ^ xb);
    xa ^= t1;
    xb ^= t1;
    xa -= a_odd & xb;
    xa >>= 1;
    ls ^= (xb + 2) >> 1;
  }

  uint32_t r = 1 - ((uint32_t)ls & 2);
  r &= ~fp_is_zero(x);
  return r == 1;
}

void fp_encode_legacy_hash(void *dst, const digit_t *a) {
  memcpy(dst, a, FP_ENCODED_BYTES);
}
