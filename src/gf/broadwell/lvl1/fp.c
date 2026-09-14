#include "include/fp.h"

const uint64_t p[NWORDS_FIELD] = {0xffffffffffffffff, 0xffffffffffffffff,
                                  0xa7ecc14ec3fa83c7, 0x62d7f37f9815e5fc};
const uint64_t R2[NWORDS_FIELD] = {0xb72f360b899325e6, 0xb9b0613be5d0b8bf,
                                   0x3b043bcf4d7492ff, 0x4939383cef249da9};
const uint64_t pp[NWORDS_FIELD] = {0x01, 0x00, 0x00, 0x00};

void fp_set(
    digit_t *x,
    const digit_t val) { // Set field element x = val, where val has wordsize

  x[0] = val;
  for (unsigned int i = 1; i < NWORDS_FIELD; i++) {
    x[i] = 0;
  }
}

void fp_mont_setone(digit_t *out1) {
  out1[0] = 0x2;
  out1[1] = 0x0;
  out1[2] = UINT64_C(0xb0267d62780af870);
  out1[3] = UINT64_C(0x3a501900cfd43406);
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

void fp_neg(digit_t *out,
            const digit_t *a) { // Modular negation, out = -a mod p
                                // Input: a in [0, p-1]
                                // Output: out in [0, p-1]
  unsigned int i, borrow = 0;

  for (i = 0; i < NWORDS_FIELD; i++) {
    SUBC(out[i], borrow, ((digit_t *)p)[i], a[i], borrow);
  }
  fp_sub(out, out, (digit_t *)p);
}

void fp_tomont(digit_t *out,
               const digit_t *a) { // Conversion to Montgomery representation
                                   // out = a*R^2*R^(-1) mod p = a*R mod p,
                                   // where a in [0, p-1].

  fp_mul(out, a, (digit_t *)&R2);
}

void fp_frommont(
    digit_t *out,
    const digit_t
        *a) { // Conversion from Montgomery representation to standard
              // representation out = a*R^(-1) mod p, where a in [0, p-1].
  digit_t one[NWORDS_FIELD] = {0};

  one[0] = 1;
  fp_mul(out, a, one);
}

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

  for (int i = nwords - 1; i > 0; i--) {
    SHIFTL(x[i], x[i - 1], shift, x[i], RADIX);
  }
  x[0] <<= shift;
}

static void fp_exp3div4(digit_t *out, const digit_t *a) {
  fp_t t[16], t2;
  // 1. Precompute odd powers: a^1, a^3, ..., a^31
  fp_copy(t[0], a);
  fp_sqr(t2, t[0]);
  for (int i = 1; i < 16; i++) {
    fp_mul(t[i], t[i - 1], t2);
  }

  // Exponent: e = 3^78 * 2^129 - 1
  // Hex: 0x18B5FCDFE605797F29FB3053B0FEA0F1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
  // (253 bits)
  static const uint8_t exp[32] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF1, 0xA0, 0xFE, 0xB0, 0x53, 0x30,
      0xFB, 0x29, 0x7F, 0x79, 0x05, 0xE6, 0xDF, 0xFC, 0xB5, 0x18};

  fp_t res;
  bool first = true;
  int i = 252; // MSB of e1

  while (i >= 0) {
    uint8_t bit = (exp[i >> 3] >> (i & 7)) & 1;
    if (bit == 0) {
      if (!first)
        fp_sqr(res, res);
      i--;
    } else {
      // Find longest odd window up to 5 bits
      int window_val = 1;
      int best_len = 1;
      for (int len = 2; len <= 5 && (i - len + 1) >= 0; len++) {
        uint8_t b = (exp[(i - len + 1) >> 3] >> ((i - len + 1) & 7)) & 1;
        if (b) {
          best_len = len;
          // Extract value
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

void fp_inv(
    digit_t *a) { // Modular inversion, out = x^-1*R mod p, where R =
                  // 2^(w*nwords), w is the computer wordsize and nwords is the
                  // number of words to represent p Input: a=xR in [0, p-1]
                  // Output: out in [0, p-1]. It outputs 0 if the input does not
                  // have an inverse Requirement: Ceiling(Log(p)) < w*nwords
  fp_t t;

  fp_exp3div4(t, a);
  fp_sqr(t, t);
  fp_sqr(t, t);
  fp_mul(a, t, a); // a^(p-2)
}

bool fp_is_square(const digit_t *a) { // Is field element a square?
                                      // Output: out = 0 (false), 1 (true)
  fp_t t, one;

  fp_exp3div4(t, a);
  fp_sqr(t, t);
  fp_mul(t, t, a); // a^((p-1)/2)
  fp_frommont(t, t);
  fp_set(one, 1);

  return fp_is_equal(t, one);
}

void fp_sqrt(digit_t *a) { // Square root computation, out = a^((p+1)/4) mod p
  fp_t t;

  fp_exp3div4(t, a);
  fp_mul(a, t, a); // a^((p+1)/4)
}