#include "include/fp.h"

// Level 5 Prime p =
// 0x4c54193130bb665f234a5c454b6e0020b8d8a1c1c552969b4c46ef90acec987fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
const uint64_t p[NWORDS_FIELD] = {0xffffffffffffffff, 0xffffffffffffffff,
                                  0xffffffffffffffff, 0xffffffffffffffff,
                                  0x4c46ef90acec987f, 0xb8d8a1c1c552969b,
                                  0x234a5c454b6e0020, 0x4c54193130bb665f};
const uint64_t R2[NWORDS_FIELD] = {0xcc5da41faefc6b2f, 0xbf733adf97cc6f01,
                                   0x87aa1f5dec2fa18b, 0x31884e9f8a5a73b0,
                                   0x717afb04c3247564, 0xd755b50ad457b97e,
                                   0x904dc385989a7072, 0x115ad571d261deaf};

/* fiat-crypto declarations */
typedef unsigned char fiat_p512_uint1;
extern void fiat_p512_msat(uint64_t out1[9]);
extern void fiat_p512_set_one(uint64_t out1[8]);

extern void fiat_p512_mul(uint64_t out1[8], const uint64_t arg1[8],
                          const uint64_t arg2[8]);

void fp_set(
    digit_t *x,
    const digit_t val) { // Set field element x = val, where val has wordsize

  x[0] = val;
  for (unsigned int i = 1; i < NWORDS_FIELD; i++) {
    x[i] = 0;
  }
}

bool fp_is_equal(
    const digit_t *a,
    const digit_t *b) { // Compare two field elements in constant time
                        // Returns 1 (true) if a=b, 0 (false) otherwise
  digit_t r = 0;

  for (unsigned int i = 0; i < NWORDS_FIELD; i++)
    r |= a[i] ^ b[i];

  return (bool)is_digit_zero_ct(r);
}

bool fp_is_zero(
    const digit_t *a) { // Is a field element zero?
                        // Returns 1 (true) if a=0, 0 (false) otherwise
  digit_t r = 0;

  for (unsigned int i = 0; i < NWORDS_FIELD; i++)
    r |= a[i] ^ 0;

  return (bool)is_digit_zero_ct(r);
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

void fp_div3(digit_t *out, const digit_t *in) {
  digit_t t[NWORDS_FIELD];
  fp_set(t, 3);
  fp_tomont(t, t);
  fp_inv(t);
  fp_mul(out, in, t);
}

// void fp_neg(digit_t *out,
//             const digit_t *a) { // Modular negation, out = -a mod p
//                                 // Input: a in [0, p-1]
//                                 // Output: out in [0, p-1]
//   unsigned int i, borrow = 0;

//   for (i = 0; i < NWORDS_FIELD; i++) {
//     SUBC(out[i], borrow, ((digit_t *)p)[i], a[i], borrow);
//   }
//   fp_sub(out, out, (digit_t *)p);
// }

void MUL(digit_t *out, const digit_t a,
         const digit_t b) { // Digit multiplication, digit*digit -> 2-digit
                            // result Inputs: a, b in [0, 2^w-1], where w is the
                            // computer wordsize Output: 0 < out < 2^(2w)-1
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

  for (int i = (int)nwords - 1; i > 0; i--) {
    SHIFTL(x[i], x[i - 1], shift, x[i], RADIX);
  }
  x[0] <<= shift;
}

/* -----------------------------------------------------------------------
 * fp_exp3div4 — out = a^((p-3)/4) mod p   (optimized 2-3 addition chain)
 *
 * p = 2^263 * 3^156 - 1,   e = (p-3)/4 = 2^261 * 3^156 - 1
 *
 * Decomposition:  e = 3^156 * (2^261 - 1) + (3^156 - 1)
 *   => a^e = (a^{3^156})^{2^261 - 1}  *  a^{3^156 - 1}
 * ----------------------------------------------------------------------- */

/* Helper: out = x^{2^k} (square x exactly k times) */
void fp_sqr_repeat(digit_t *out, const digit_t *x, int k) {
  fp_copy(out, x);
  for (int i = 0; i < k; i++)
    fp_sqr(out, out);
}

void fp_exp3div4(digit_t *out, const digit_t *a) {
  fp_t t[16], t2;
  // 1. Precompute odd powers: a^1, a^3, ..., a^31
  fp_copy(t[0], a);
  fp_sqr(t2, t[0]);
  for (int i = 1; i < 16; i++) {
    fp_mul(t[i], t[i - 1], t2);
  }

  // Exponent: e = (p-3)/4 = 3^{156} * 2^{261} - 1
  // Correct bytes from golden hex (64 bytes, Little Endian)
  static const uint8_t exp[64] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F,
      0x26, 0x3B, 0x2B, 0xE4, 0xBB, 0x11, 0xD3, 0xA6, 0xA5, 0x54, 0x71,
      0x70, 0x28, 0x36, 0x2E, 0x08, 0x80, 0xDB, 0x52, 0x11, 0x97, 0xD2,
      0xC8, 0x97, 0xD9, 0x2E, 0x4C, 0x4C, 0x06, 0x15, 0x13};

  fp_t res;
  bool first = true;
  int i = 508; // Approx bits for e5

  while (i >= 0) {
    uint8_t bit = (exp[i >> 3] >> (i & 7)) & 1;
    if (bit == 0) {
      if (!first)
        fp_sqr(res, res);
      i--;
    } else {
      int window_val = 1;
      int best_len = 1;
      for (int len = 2; len <= 5 && (i - len + 1) >= 0; len++) {
        uint8_t b = (exp[(i - len + 1) >> 3] >> ((i - len + 1) & 7)) & 1;
        if (b) {
          best_len = len;
          int v = 0;
          for (int k = 0; k < len; k++) {
            v |= ((exp[(i - k) >> 3] >> ((i - k) & 7)) & 1) << (len - 1 - k);
          }
          window_val = v;
        }
      }

      if (first) {
        fp_copy(res, t[window_val >> 1]);
        first = false;
      } else {
        for (int k = 0; k < best_len; k++)
          fp_sqr(res, res);
        fp_mul(res, res, t[window_val >> 1]);
      }
      i -= best_len;
    }
  }
  fp_copy(out, res);
}

void fp_inv(digit_t *a) {
  fp_t t;
  fp_exp3div4(t, a); // t = a^((p-3)/4)
  fp_sqr(t, t);      // t = a^((p-3)/2)
  fp_sqr(t, t);      // t = a^(p-3)
  fp_mul(a, t, a);   // a = a^(p-2)
}

// helper macros and functions for binary GCD Legendre symbol calculation
static inline uint64_t sgnw(uint64_t x) {
  return (uint64_t)(*(int64_t *)&x >> 63);
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

static inline unsigned char inner_fp_sbb(unsigned char cc, uint64_t a,
                                         uint64_t b, uint64_t *d) {
  uint64_t tempReg = a - b;
  unsigned int borrowReg = (a < b) | ((cc != 0) & (tempReg == 0));
  *d = tempReg - (uint64_t)(cc != 0);
  return (unsigned char)borrowReg;
}

static inline unsigned char inner_fp_adc(unsigned char cc, uint64_t a,
                                         uint64_t b, uint64_t *d) {
  uint64_t tempReg = a + (uint64_t)(cc != 0);
  *d = b + tempReg;
  return (unsigned char)((tempReg < (uint64_t)(cc != 0)) | (*d < tempReg));
}

static inline void inner_fp_normalize(digit_t *d, const digit_t *a) {
  uint64_t d0, d1, d2, d3, d4, d5, d6, d7, m;
  unsigned char cc;

  cc = inner_fp_sbb(0, a[0], p[0], &d0);
  cc = inner_fp_sbb(cc, a[1], p[1], &d1);
  cc = inner_fp_sbb(cc, a[2], p[2], &d2);
  cc = inner_fp_sbb(cc, a[3], p[3], &d3);
  cc = inner_fp_sbb(cc, a[4], p[4], &d4);
  cc = inner_fp_sbb(cc, a[5], p[5], &d5);
  cc = inner_fp_sbb(cc, a[6], p[6], &d6);
  cc = inner_fp_sbb(cc, a[7], p[7], &d7);

  (void)inner_fp_sbb(cc, 0, 0, &m);
  cc = inner_fp_adc(0, d0, m & p[0], &d0);
  cc = inner_fp_adc(cc, d1, m & p[1], &d1);
  cc = inner_fp_adc(cc, d2, m & p[2], &d2);
  cc = inner_fp_adc(cc, d3, m & p[3], &d3);
  cc = inner_fp_adc(cc, d4, m & p[4], &d4);
  cc = inner_fp_adc(cc, d5, m & p[5], &d5);
  cc = inner_fp_adc(cc, d6, m & p[6], &d6);
  (void)inner_fp_adc(cc, d7, m & p[7], &d7);

  d[0] = d0;
  d[1] = d1;
  d[2] = d2;
  d[3] = d3;
  d[4] = d4;
  d[5] = d5;
  d[6] = d6;
  d[7] = d7;
}

#define inner_fp_umul(lo, hi, x, y)                                            \
  do {                                                                         \
    unsigned __int128 umul_tmp;                                                \
    umul_tmp = (unsigned __int128)(x) * (unsigned __int128)(y);                \
    (lo) = (uint64_t)umul_tmp;                                                 \
    (hi) = (uint64_t)(umul_tmp >> 64);                                         \
  } while (0)

#define inner_fp_umul_x2(lo, hi, x1, y1, x2, y2)                               \
  do {                                                                         \
    unsigned __int128 umul_tmp;                                                \
    umul_tmp = (unsigned __int128)(x1) * (unsigned __int128)(y1) +             \
               (unsigned __int128)(x2) * (unsigned __int128)(y2);              \
    (lo) = (uint64_t)umul_tmp;                                                 \
    (hi) = (uint64_t)(umul_tmp >> 64);                                         \
  } while (0)

#define inner_fp_umul_x2_add(lo, hi, x1, y1, x2, y2, z)                        \
  do {                                                                         \
    unsigned __int128 umul_tmp;                                                \
    umul_tmp = (unsigned __int128)(x1) * (unsigned __int128)(y1) +             \
               (unsigned __int128)(x2) * (unsigned __int128)(y2) +             \
               (unsigned __int128)(uint64_t)(z);                               \
    (lo) = (uint64_t)umul_tmp;                                                 \
    (hi) = (uint64_t)(umul_tmp >> 64);                                         \
  } while (0)

static uint64_t lindiv31abs(digit_t *d, const digit_t *a, const digit_t *b,
                            uint64_t f, uint64_t g) {
  uint64_t sf = sgnw(f);
  f = (f ^ sf) - sf;
  uint64_t sg = sgnw(g);
  g = (g ^ sg) - sg;

  uint64_t a0, a1, a2, a3, a4, a5, a6, a7, a8;
  uint64_t b0, b1, b2, b3, b4, b5, b6, b7, b8;
  unsigned char cc;

  cc = inner_fp_sbb(0, a[0] ^ sf, sf, &a0);
  cc = inner_fp_sbb(cc, a[1] ^ sf, sf, &a1);
  cc = inner_fp_sbb(cc, a[2] ^ sf, sf, &a2);
  cc = inner_fp_sbb(cc, a[3] ^ sf, sf, &a3);
  cc = inner_fp_sbb(cc, a[4] ^ sf, sf, &a4);
  cc = inner_fp_sbb(cc, a[5] ^ sf, sf, &a5);
  cc = inner_fp_sbb(cc, a[6] ^ sf, sf, &a6);
  cc = inner_fp_sbb(cc, a[7] ^ sf, sf, &a7);
  (void)inner_fp_sbb(cc, 0, 0, &a8);

  cc = inner_fp_sbb(0, b[0] ^ sg, sg, &b0);
  cc = inner_fp_sbb(cc, b[1] ^ sg, sg, &b1);
  cc = inner_fp_sbb(cc, b[2] ^ sg, sg, &b2);
  cc = inner_fp_sbb(cc, b[3] ^ sg, sg, &b3);
  cc = inner_fp_sbb(cc, b[4] ^ sg, sg, &b4);
  cc = inner_fp_sbb(cc, b[5] ^ sg, sg, &b5);
  cc = inner_fp_sbb(cc, b[6] ^ sg, sg, &b6);
  cc = inner_fp_sbb(cc, b[7] ^ sg, sg, &b7);
  (void)inner_fp_sbb(cc, 0, 0, &b8);

  uint64_t d0, d1, d2, d3, d4, d5, d6, d7, d8, t;
  inner_fp_umul_x2(d0, t, a0, f, b0, g);
  inner_fp_umul_x2_add(d1, t, a1, f, b1, g, t);
  inner_fp_umul_x2_add(d2, t, a2, f, b2, g, t);
  inner_fp_umul_x2_add(d3, t, a3, f, b3, g, t);
  inner_fp_umul_x2_add(d4, t, a4, f, b4, g, t);
  inner_fp_umul_x2_add(d5, t, a5, f, b5, g, t);
  inner_fp_umul_x2_add(d6, t, a6, f, b6, g, t);
  inner_fp_umul_x2_add(d7, t, a7, f, b7, g, t);
  d8 = t - (a8 & f) - (b8 & g);

  d0 = (d0 >> 31) | (d1 << 33);
  d1 = (d1 >> 31) | (d2 << 33);
  d2 = (d2 >> 31) | (d3 << 33);
  d3 = (d3 >> 31) | (d4 << 33);
  d4 = (d4 >> 31) | (d5 << 33);
  d5 = (d5 >> 31) | (d6 << 33);
  d6 = (d6 >> 31) | (d7 << 33);
  d7 = (d7 >> 31) | (d8 << 33);

  t = sgnw(d8);
  cc = inner_fp_sbb(0, d0 ^ t, t, &d0);
  cc = inner_fp_sbb(cc, d1 ^ t, t, &d1);
  cc = inner_fp_sbb(cc, d2 ^ t, t, &d2);
  cc = inner_fp_sbb(cc, d3 ^ t, t, &d3);
  cc = inner_fp_sbb(cc, d4 ^ t, t, &d4);
  cc = inner_fp_sbb(cc, d5 ^ t, t, &d5);
  cc = inner_fp_sbb(cc, d6 ^ t, t, &d6);
  (void)inner_fp_sbb(cc, d7 ^ t, t, &d7);

  d[0] = d0;
  d[1] = d1;
  d[2] = d2;
  d[3] = d3;
  d[4] = d4;
  d[5] = d5;
  d[6] = d6;
  d[7] = d7;
  return t;
}

bool fp_is_square(const digit_t *x) {
  fp_t a, b;
  uint64_t xa, xb, f0, g0, f1, g1, ls;

  inner_fp_normalize(a, x);

  b[0] = p[0];
  b[1] = p[1];
  b[2] = p[2];
  b[3] = p[3];
  b[4] = p[4];
  b[5] = p[5];
  b[6] = p[6];
  b[7] = p[7];

  ls = 0;

  for (int i = 0; i < 31; i++) {
    uint64_t m7 = a[7] | b[7];
    uint64_t m6 = a[6] | b[6];
    uint64_t m5 = a[5] | b[5];
    uint64_t m4 = a[4] | b[4];
    uint64_t m3 = a[3] | b[3];
    uint64_t m2 = a[2] | b[2];
    uint64_t m1 = a[1] | b[1];
    uint64_t tnz7 = sgnw(m7 | -m7);
    uint64_t tnz6 = sgnw(m6 | -m6) & ~tnz7;
    uint64_t tnz5 = sgnw(m5 | -m5) & ~tnz7 & ~tnz6;
    uint64_t tnz4 = sgnw(m4 | -m4) & ~tnz7 & ~tnz6 & ~tnz5;
    uint64_t tnz3 = sgnw(m3 | -m3) & ~tnz7 & ~tnz6 & ~tnz5 & ~tnz4;
    uint64_t tnz2 = sgnw(m2 | -m2) & ~tnz7 & ~tnz6 & ~tnz5 & ~tnz4 & ~tnz3;
    uint64_t tnz1 =
        sgnw(m1 | -m1) & ~tnz7 & ~tnz6 & ~tnz5 & ~tnz4 & ~tnz3 & ~tnz2;
    uint64_t tnzm = (m7 & tnz7) | (m6 & tnz6) | (m5 & tnz5) | (m4 & tnz4) |
                    (m3 & tnz3) | (m2 & tnz2) | (m1 & tnz1);
    uint64_t tnza = (a[7] & tnz7) | (a[6] & tnz6) | (a[5] & tnz5) |
                    (a[4] & tnz4) | (a[3] & tnz3) | (a[2] & tnz2) |
                    (a[1] & tnz1);
    uint64_t tnzb = (b[7] & tnz7) | (b[6] & tnz6) | (b[5] & tnz5) |
                    (b[4] & tnz4) | (b[3] & tnz3) | (b[2] & tnz2) |
                    (b[1] & tnz1);
    uint64_t snza = (a[6] & tnz7) | (a[5] & tnz6) | (a[4] & tnz5) |
                    (a[3] & tnz4) | (a[2] & tnz3) | (a[1] & tnz2) |
                    (a[0] & tnz1);
    uint64_t snzb = (b[6] & tnz7) | (b[5] & tnz6) | (b[4] & tnz5) |
                    (b[3] & tnz4) | (b[2] & tnz3) | (b[1] & tnz2) |
                    (b[0] & tnz1);

    int64_t s = lzcnt(tnzm);
    uint64_t sm = (uint64_t)((31 - s) >> 63);
    tnza ^= sm & (tnza ^ ((tnza << 32) | (snza >> 32)));
    tnzb ^= sm & (tnzb ^ ((tnzb << 32) | (snzb >> 32)));
    s -= 32 & sm;
    tnza <<= s;
    tnzb <<= s;

    uint64_t tzx = ~(tnz1 | tnz2 | tnz3 | tnz4 | tnz5 | tnz6 | tnz7);
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
    fp_t na, nb;
    uint64_t nega = lindiv31abs(na, a, b, f0, g0);
    (void)lindiv31abs(nb, a, b, f1, g1);
    ls ^= nega & nb[0];
    fp_copy(a, na);
    fp_copy(b, nb);
  }

  xa = a[0];
  xb = b[0];
  for (int j = 0; j < 47; j++) {
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

void fp_sqrt(digit_t *a) { // Square root computation, out = a^((p+1)/4) mod p
  fp_t t, tmp;

  fp_copy(t, a);
  for (int i = 0; i < 156; i++) {
    fp_sqr(tmp, t);
    fp_mul(t, tmp, t); // t = t^3
  }
  for (int i = 0; i < 261; i++) {
    fp_sqr(t, t);
  }
  fp_copy(a, t);
}
