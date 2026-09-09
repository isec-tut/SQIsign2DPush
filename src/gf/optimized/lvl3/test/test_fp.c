#include <encoded_sizes.h>
#include "test_extras.h"
#include <stdio.h>
#include <string.h>
#include <bench.h>

// Global constants
extern const digit_t p[NWORDS_FIELD];

#include "../../test/fp_exp_tests.h"

// Benchmark and test parameters  
static int BENCH_LOOPS = 100000;       // Number of iterations per bench
static int TEST_LOOPS  = 100000;       // Number of iterations per test


/* Serialize ordinary integers independently of fp_encode, which takes
 * Montgomery residues. */
static bool fp_decode_case(const fp_t value, bool valid)
{
    uint8_t bytes[FP_ENCODED_BYTES], encoded[FP_ENCODED_BYTES];
    fp_t decoded, expected;

    for (size_t i = 0; i < sizeof(bytes); i++)
        bytes[i] = (uint8_t)(value[i / sizeof(digit_t)] >>
                             (8 * (i % sizeof(digit_t))));

    memset(decoded, 0xa5, sizeof(decoded));
    uint32_t status = fp_decode(&decoded, bytes);
    if (status != (valid ? UINT32_MAX : 0))
        return false;

    if (!valid)
        return fp_is_zero(decoded);

    fp_tomont(expected, value);
    if (!fp_is_equal(decoded, expected))
        return false;

    fp_encode(encoded, &decoded);
    return memcmp(encoded, bytes, sizeof(bytes)) == 0;
}

static bool fp_decode_test(void)
{
    fp_t value;

    fp_set(value, 0);
    if (!fp_decode_case(value, true))
        return false;
    fp_set(value, 1);
    if (!fp_decode_case(value, true))
        return false;

    fp_copy(value, p);
    value[0]--; /* p - 1: p has an all-ones low limb. */
    if (!fp_decode_case(value, true))
        return false;

    fp_copy(value, p);
    if (!fp_decode_case(value, false))
        return false;

    /* p + 1, including carry propagation through all-ones limbs. */
    for (size_t i = 0; i < NWORDS_FIELD; i++) {
        value[i]++;
        if (value[i] != 0)
            break;
    }
    if (!fp_decode_case(value, false))
        return false;

    memset(value, 0xff, sizeof(value));
    if (!fp_decode_case(value, false))
        return false;

    for (int i = 0; i < TEST_LOOPS; i++) {
        fprandom_test(value);
        if (!fp_decode_case(value, true))
            return false;
    }
    return true;
}

bool fp_test()
{ // Tests for the field arithmetic
    bool OK = true;
    int n, passed;
    fp_t a, b, c, d, e, f, ma, mb, mc, md, me, mf;

    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Testing field arithmetic over GF(p): \n\n"); 

    if (!fp_decode_test()) {
        printf("  GF(p) decoding tests... FAILED\n");
        return false;
    }
    printf("  GF(p) decoding tests ............................................ PASSED\n");

    if (!fp_fixed_exp_test(TEST_LOOPS)) {
        printf("  GF(p) fixed exponent tests... FAILED\n");
        return false;
    }
    printf("  GF(p) fixed exponent tests ...................................... PASSED\n");

    // Field addition
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {
        fprandom_test(a); fp_tomont(a, a); fprandom_test(b); fp_tomont(b, b); fprandom_test(c); fp_tomont(c, c); fprandom_test(d); fp_tomont(d, d);

        fp_add(d, a, b); fp_add(e, d, c);                 // e = (a+b)+c
        fp_add(d, b, c); fp_add(f, d, a);                 // f = a+(b+c)
        if (!fp_is_equal(e, f)) { passed=0; break; }

        fp_add(d, a, b);                                  // d = a+b 
        fp_add(e, b, a);                                  // e = b+a
        if (!fp_is_equal(d, e)) { passed=0; break; }

        fp_set(b, 0);
        fp_add(d, a, b);                                  // d = a+0 
        if (!fp_is_equal(a, d)) { passed=0; break; }

        fp_set(b, 0);   
        fp_neg(d, a);                      
        fp_add(e, a, d);                                  // e = a+(-a)
        if (!fp_is_equal(e, b)) { passed=0; break; }
    }
    if (passed==1) printf("  GF(p) addition tests ............................................ PASSED");
    else { printf("  GF(p) addition tests... FAILED"); printf("\n"); return false; }
    printf("\n");

    // Field subtraction
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {
        fprandom_test(a); fp_tomont(a, a); fprandom_test(b); fp_tomont(b, b); fprandom_test(c); fp_tomont(c, c); fprandom_test(d); fp_tomont(d, d);

        fp_sub(d, a, b); fp_sub(e, d, c);                 // e = (a-b)-c
        fp_add(d, b, c); fp_sub(f, a, d);                 // f = a-(b+c)
        if (!fp_is_equal(e, f)) { passed=0; break; }

        fp_sub(d, a, b);                                  // d = a-b 
        fp_sub(e, b, a);
        fp_neg(e, e);                                     // e = -(b-a)
        if (!fp_is_equal(d, e)) { passed=0; break; }

        fp_set(b, 0);
        fp_sub(d, a, b);                                  // d = a-0 
        if (!fp_is_equal(a, d)) { passed=0; break; }
        
        fp_set(b, 0);              
        fp_sub(e, a, a);                                  // e = a+(-a)
        if (!fp_is_equal(e, b)) { passed=0; break; }
    }
    if (passed==1) printf("  GF(p) subtraction tests ......................................... PASSED");
    else { printf("  GF(p) subtraction tests... FAILED"); printf("\n"); return false; }
    printf("\n");

    // Field multiplication
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {    
        fprandom_test(a); fprandom_test(b); fprandom_test(c);
        
        fp_tomont(ma, a);
        fp_frommont(c, ma);
        if (compare_words(a, c, NWORDS_FIELD)!=0) { passed=0; break; }

        fp_tomont(ma, a); fp_tomont(mb, b); fp_tomont(mc, c);
        fp_mul(md, ma, mb); fp_mul(me, md, mc);                          // e = (a*b)*c
        fp_mul(md, mb, mc); fp_mul(mf, md, ma);                          // f = a*(b*c)
        fp_frommont(e, me);
        fp_frommont(f, mf);
        if (compare_words(e, f, NWORDS_FIELD)!=0) { passed=0; break; }

        fp_tomont(ma, a); fp_tomont(mb, b); fp_tomont(mc, c); 
        fp_add(md, mb, mc); fp_mul(me, ma, md);                          // e = a*(b+c)
        fp_mul(md, ma, mb); fp_mul(mf, ma, mc); fp_add(mf, md, mf);      // f = a*b+a*c
        fp_frommont(e, me);
        fp_frommont(f, mf);
        if (compare_words(e, f, NWORDS_FIELD)!=0) { passed=0; break; }
     
        fp_tomont(ma, a); fp_tomont(mb, b);
        fp_mul(md, ma, mb);                                              // d = a*b 
        fp_mul(me, mb, ma);                                              // e = b*a 
        fp_frommont(d, md);
        fp_frommont(e, me);
        if (compare_words(d, e, NWORDS_FIELD)!=0) { passed=0; break; }

        fp_tomont(ma, a);
        fp_set(b, 1); fp_tomont(mb, b);
        fp_mul(md, ma, mb);                                              // d = a*1  
        fp_frommont(a, ma);
        fp_frommont(d, md);                
        if (compare_words(a, d, NWORDS_FIELD)!=0) { passed=0; break; }
       
        fp_set(b, 0);
        fp_tomont(mb, b);
        fp_mul(md, ma, mb);                                              // d = a*0 
        fp_frommont(d, md);                
        if (compare_words(b, d, NWORDS_FIELD)!=0) { passed=0; break; } 
    }
    if (passed==1) printf("  GF(p) multiplication tests ...................................... PASSED");
    else { printf("  GF(p) multiplication tests... FAILED"); printf("\n"); return false; }
    printf("\n");

    // Field squaring
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {
        fprandom_test(a);
        
        fp_tomont(ma, a);
        fp_sqr(mb, ma);                                   // b = a^2
        fp_mul(mc, ma, ma);                               // c = a*a 
        fp_frommont(b, mb);
        fp_frommont(c, mc);
        if (compare_words(b, c, NWORDS_FIELD)!=0) { passed=0; break; }

        fp_set(a, 0); fp_tomont(ma, a);
        fp_sqr(md, ma);                                   // d = 0^2 
        if (!fp_is_equal(ma, md)) { passed=0; break; }
    }
    if (passed==1) printf("  GF(p) squaring tests............................................. PASSED");
    else { printf("  GF(p) squaring tests... FAILED"); printf("\n"); return false; }
    printf("\n");

    // Field inversion
    passed = 1;
    for (n = 0; n < TEST_LOOPS; n++)
    {
        fprandom_test(a);

        fp_tomont(ma, a);
        fp_set(d, 1);
        memcpy(mb, ma, RADIX/8 * NWORDS_FIELD);
        fp_inv(ma);
        fp_mul(mc, ma, mb);                               // c = a*a^-1 
        fp_frommont(c, mc);
        if (compare_words(c, d, NWORDS_FIELD) != 0) { passed = 0; break; }

        fp_set(a, 0);
        fp_set(d, 0);
        fp_inv(a);                                        // c = 0^-1
        if (!fp_is_equal(a, d)) { passed = 0; break; }
    }
    if (passed == 1) printf("  GF(p) inversion tests............................................ PASSED");
    else { printf("  GF(p) inversion tests... FAILED"); printf("\n"); return false; }
    printf("\n");

    // Square root and square detection
    passed = 1;
    for (n = 0; n < TEST_LOOPS; n++)
    {
        fprandom_test(a);

        fp_tomont(ma, a);
        fp_sqr(mc, ma);
        fp_frommont(c, mc);                               // c = a^2
        if (fp_is_square(mc) != 1) { passed = 0; break; }

        fp_sqrt(mc);                                      // c = a = sqrt(c) 
        fp_neg(md, mc);
        fp_frommont(c, mc);
        fp_frommont(d, md);
        if ((compare_words(a, c, NWORDS_FIELD) != 0) && (compare_words(a, d, NWORDS_FIELD) != 0)) { passed = 0; break; }
    }
    if (passed == 1) printf("  Square root, square tests........................................ PASSED");
    else { printf("  Square root, square tests... FAILED"); printf("\n"); return false; }
    printf("\n");
 
    return OK;
}

bool fp_run()
{
    bool OK = true;
    int n;
  unsigned long long cycles, cycles1, cycles2;
  volatile unsigned int square_result = 0;
    fp_t a, b, c;
        
    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Benchmarking field arithmetic: \n\n"); 
        
    fprandom_test(a); fp_tomont(a, a); fprandom_test(b); fp_tomont(b, b); fprandom_test(c); fp_tomont(c, c);

    // GF(p) addition
    cycles = 0;
    for (n=0; n<BENCH_LOOPS; n++)
    {
        cycles1 = cpucycles(); 
        fp_add(c, a, b);
        cycles2 = cpucycles();
        cycles = cycles+(cycles2-cycles1);
    }
    printf("  GF(p) addition runs in .......................................... %7lld cycles", cycles/BENCH_LOOPS);
    printf("\n");

    // GF(p) subtraction
    cycles = 0;
    for (n=0; n<BENCH_LOOPS; n++)
    {
        cycles1 = cpucycles(); 
        fp_sub(c, a, b);
        cycles2 = cpucycles();
        cycles = cycles+(cycles2-cycles1);
    }
    printf("  GF(p) subtraction runs in ....................................... %7lld cycles", cycles/BENCH_LOOPS);
    printf("\n");

    // GF(p) multiplication
    cycles = 0;
    for (n=0; n<BENCH_LOOPS; n++)
    {
        cycles1 = cpucycles(); 
        fp_mul(c, a, b);
        cycles2 = cpucycles();
        cycles = cycles+(cycles2-cycles1);
    }
    printf("  GF(p) multiplication runs in .................................... %7lld cycles", cycles/BENCH_LOOPS);
    printf("\n");

    // GF(p) inversion
    cycles = 0;
    for (n = 0; n < BENCH_LOOPS; n++)
    {
        cycles1 = cpucycles();
        fp_inv(a);
        cycles2 = cpucycles();
        cycles = cycles + (cycles2 - cycles1);
    }
    printf("  GF(p) inversion runs in ......................................... %7lld cycles", cycles/BENCH_LOOPS);
    printf("\n");

    // GF(p) square root
    cycles = 0;
    for (n = 0; n < BENCH_LOOPS; n++)
    {
        cycles1 = cpucycles();
        fp_sqrt(a);
        cycles2 = cpucycles();
        cycles = cycles + (cycles2 - cycles1);
    }
    printf("  GF(p) square root runs in ....................................... %7lld cycles", cycles/BENCH_LOOPS);
    printf("\n");

    // Square checking
    cycles = 0;
    for (n = 0; n < BENCH_LOOPS; n++)
    {
        cycles1 = cpucycles();
    square_result ^= (unsigned int)fp_is_square(a);
        cycles2 = cpucycles();
        cycles = cycles + (cycles2 - cycles1);
    }
    printf("  Square checking runs in ......................................... %7lld cycles", cycles/BENCH_LOOPS);
    printf("\n");

    return OK;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("Please enter an argument: 'test' or 'bench' and <reps>\n");
        exit(1);
    }
    if (!strcmp(argv[1], "test")) {
        TEST_LOOPS = atoi(argv[2]);
        return !fp_test();
    } else if (!strcmp(argv[1], "bench")) {
        BENCH_LOOPS = atoi(argv[2]);
        return !fp_run();
    } else {
        exit(1);
    }
}
