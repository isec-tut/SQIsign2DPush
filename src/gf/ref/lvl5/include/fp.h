#ifndef FP_H
#define FP_H

#include <fp_constants.h>
#include <sqisign_namespace.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <tutil.h>

typedef digit_t fp_t[NWORDS_FIELD]; // Datatype for representing field elements

void fp_set(digit_t *x, const digit_t val);
bool fp_is_equal(const digit_t *a, const digit_t *b);
bool fp_is_zero(const digit_t *a);
void fp_copy(digit_t *out, const digit_t *a);
void fp_select(digit_t *out, const digit_t *a, const digit_t *b, uint32_t c);
digit_t mp_shiftr(digit_t *x, const unsigned int shift,
                  const unsigned int nwords);
void mp_shiftl(digit_t *x, const unsigned int shift, const unsigned int nwords);
void fp_add(digit_t *out, const digit_t *a, const digit_t *b);
void fp_sub(digit_t *out, const digit_t *a, const digit_t *b);
void fp_neg(digit_t *out, const digit_t *a);
void fp_sqr(digit_t *out, const digit_t *a);
void fp_mul(digit_t *out, const digit_t *a, const digit_t *b);
void MUL(digit_t *out, const digit_t a, const digit_t b);
void fp_inv(digit_t *x);
bool fp_is_square(const digit_t *a);
void fp_sqrt(digit_t *a);
void fp_exp3div4(digit_t *out, const digit_t *a);
void fp_div3(digit_t *out, const digit_t *in);
void fp_tomont(digit_t *out, const digit_t *a);
void fp_frommont(digit_t *out, const digit_t *a);
void fp_mont_setone(digit_t *out);

void fp_encode(void *dst, const fp_t *a);
/* Decode a canonical little-endian integer to Montgomery form.
 * Return UINT32_MAX on success; otherwise return 0 and set *d to zero. */
uint32_t fp_decode(fp_t *d, const void *src);
void fp_decode_reduce(fp_t *d, const void *src, size_t len);

/********************** Constant-time unsigned comparisons
 * ***********************/

// The following functions return 1 (TRUE) if condition is true, 0 (FALSE)
// otherwise

static inline unsigned int is_digit_nonzero_ct(digit_t x) { // Is x != 0?
  return (unsigned int)((x | (0 - x)) >> (RADIX - 1));
}

static inline unsigned int is_digit_zero_ct(digit_t x) { // Is x = 0?
  return (unsigned int)(1 ^ is_digit_nonzero_ct(x));
}

static inline unsigned int is_digit_lessthan_ct(digit_t x,
                                                digit_t y) { // Is x < y?
  return (unsigned int)((x ^ ((x ^ y) | ((x - y) ^ y))) >> (RADIX - 1));
}

/********************** Platform-independent macros for digit-size operations
 * **********************/

// Digit addition with carry
#define ADDC(sumOut, carryOut, addend1, addend2, carryIn)                      \
  {                                                                            \
    digit_t tempReg = (addend1) + (digit_t)(carryIn);                          \
    (sumOut) = (addend2) + tempReg;                                            \
    (carryOut) = (is_digit_lessthan_ct(tempReg, (digit_t)(carryIn)) |          \
                  is_digit_lessthan_ct((sumOut), tempReg));                    \
  }

// Digit subtraction with borrow
#define SUBC(differenceOut, borrowOut, minuend, subtrahend, borrowIn)          \
  {                                                                            \
    digit_t tempReg = (minuend) - (subtrahend);                                \
    unsigned int borrowReg = (is_digit_lessthan_ct((minuend), (subtrahend)) |  \
                              ((borrowIn) & is_digit_zero_ct(tempReg)));       \
    (differenceOut) = tempReg - (digit_t)(borrowIn);                           \
    (borrowOut) = borrowReg;                                                   \
  }

// Shift right with flexible datatype
#define SHIFTR(highIn, lowIn, shift, shiftOut, DigitSize)                      \
  (shiftOut) = ((lowIn) >> (shift)) ^ ((highIn) << (DigitSize - (shift)));

// Digit shift left
#define SHIFTL(highIn, lowIn, shift, shiftOut, DigitSize)                      \
  (shiftOut) = ((highIn) << (shift)) ^ ((lowIn) >> (RADIX - (shift)));

#endif