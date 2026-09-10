#ifndef FP_EXP_TESTS_H
#define FP_EXP_TESTS_H

/* Test-only binary exponentiation, independent of the generated window steps.
 * Include after test_extras.h and the declaration of p. */
static void fp_exp_binary_reference(fp_t out, const fp_t input,
                                    const fp_t exponent)
{
    fp_mont_setone(out);
    for (int bit = NWORDS_FIELD * RADIX - 1; bit >= 0; bit--) {
        fp_sqr(out, out);
        if ((exponent[bit / RADIX] >> (bit % RADIX)) & 1)
            fp_mul(out, out, input);
    }
}

static bool fp_fixed_exp_case(const fp_t value)
{
    fp_t input, exponent, expected, actual, inplace;
    fp_tomont(input, value);
    fp_copy(exponent, p);
    /* All supported primes have an all-ones low limb. */
    exponent[0] -= 3;
    mp_shiftr(exponent, 2, NWORDS_FIELD);

    fp_exp_binary_reference(expected, input, exponent);
    fp_exp3div4(actual, input);
    fp_copy(inplace, input);
    fp_exp3div4(inplace, inplace);
    if (!fp_is_equal(actual, expected) || !fp_is_equal(inplace, expected))
        return false;

    /* (p+1)/4 = (p-3)/4 + 1, with full carry propagation. */
    for (size_t i = 0; i < NWORDS_FIELD; i++) {
        exponent[i]++;
        if (exponent[i] != 0)
            break;
    }
    fp_exp_binary_reference(expected, input, exponent);
    fp_copy(actual, input);
    fp_sqrt(actual);
    return fp_is_equal(actual, expected);
}

static bool fp_fixed_exp_test(int repetitions)
{
    fp_t value;
    fp_set(value, 0);
    if (!fp_fixed_exp_case(value))
        return false;
    fp_set(value, 1);
    if (!fp_fixed_exp_case(value))
        return false;
    fp_copy(value, p);
    value[0]--;
    if (!fp_fixed_exp_case(value))
        return false;
    for (int i = 0; i < repetitions; i++) {
        fprandom_test(value);
        if (!fp_fixed_exp_case(value))
            return false;
    }
    return true;
}

#endif
