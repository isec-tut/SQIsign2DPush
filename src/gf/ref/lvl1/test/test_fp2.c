#include "test_extras.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <bench.h>

// Global constants
extern const digit_t p[NWORDS_FIELD];

// Benchmark and test parameters  
static int BENCH_LOOPS = 100000;       // Number of iterations per bench
static int TEST_LOOPS  = 100000;       // Number of iterations per test

/*
 * RDTSC is not serializing.  Bracket a whole dependency chain with the
 * recommended fences, rather than timing one call at a time: the latter is
 * dominated by the two timestamp reads and lets an LTO build remove a call
 * whose result is never used.
 */
#if defined(TARGET_AMD64) || defined(TARGET_X86)
static inline uint64_t bench_cycles_start(void)
{
    unsigned int hi, lo;
    asm volatile("lfence\n\trdtsc" : "=a"(lo), "=d"(hi) : : "memory");
    return ((uint64_t)hi << 32) | lo;
}

static inline uint64_t bench_cycles_end(void)
{
    unsigned int hi, lo;
    asm volatile("rdtscp\n\tlfence" : "=a"(lo), "=d"(hi) : : "rcx", "memory");
    return ((uint64_t)hi << 32) | lo;
}
#else
static inline uint64_t bench_cycles_start(void) { return (uint64_t)cpucycles(); }
static inline uint64_t bench_cycles_end(void) { return (uint64_t)cpucycles(); }
#endif

/* A volatile sink makes the final value observable even with -O3 -flto. */
static volatile digit_t fp2_bench_sink;
static bool (*volatile fp2_is_square_bench)(const fp2_t *) = fp2_is_square;

static void bench_consume_fp2(const fp2_t *x)
{
    fp2_bench_sink ^= x->re[0] ^ x->im[0];
}

#define BENCH_TIME_SAMPLES 31

static uint64_t bench_wall_ns(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * UINT64_C(1000000000) + now.tv_nsec;
}

static int bench_compare_u64(const void *a, const void *b)
{
    const uint64_t ua = *(const uint64_t *)a;
    const uint64_t ub = *(const uint64_t *)b;
    return (ua > ub) - (ua < ub);
}

static void bench_report(const char *name, uint64_t total_cycles,
                         int operations, uint64_t *ns_samples, int sample_count)
{
    qsort(ns_samples, sample_count, sizeof(*ns_samples), bench_compare_u64);
    printf("  %-22s %7llu cycles/op\n", name,
           (unsigned long long)(total_cycles / operations));
    printf("    time [ms, %d operations]: median %.3f, min %.3f, max %.3f\n",
           operations,
           ns_samples[sample_count / 2] * (double)operations / 1e6,
           ns_samples[0] * (double)operations / 1e6,
           ns_samples[sample_count - 1] * (double)operations / 1e6);
    printf("    time [us/op]: median %.3f, min %.3f, max %.3f\n",
           ns_samples[sample_count / 2] / 1e3,
           ns_samples[0] / 1e3,
           ns_samples[sample_count - 1] / 1e3);
}

/* Each sample uses a dependency chain. This prevents LTO from hoisting or
 * deleting the operation and makes the reported cycles a latency measurement. */
#define BENCH_FP2_CHAIN(name, operation) do { \
    uint64_t ns_samples[BENCH_TIME_SAMPLES]; \
    uint64_t total_cycles = 0; \
    int sample_count = BENCH_LOOPS < BENCH_TIME_SAMPLES ? BENCH_LOOPS : BENCH_TIME_SAMPLES; \
    int remaining = BENCH_LOOPS; \
    int sample; \
    for (sample = 0; sample < sample_count; sample++) { \
        int count = remaining / (sample_count - sample); \
        int op; \
        uint64_t wall1, wall2, cycles1, cycles2; \
        remaining -= count; \
        fp2_copy(&chain, &a); \
        wall1 = bench_wall_ns(); \
        cycles1 = bench_cycles_start(); \
        for (op = 0; op < count; op++) { operation; } \
        cycles2 = bench_cycles_end(); \
        wall2 = bench_wall_ns(); \
        bench_consume_fp2(&chain); \
        total_cycles += cycles2 - cycles1; \
        ns_samples[sample] = (wall2 - wall1) / (uint64_t)count; \
    } \
    bench_report(name, total_cycles, BENCH_LOOPS, ns_samples, sample_count); \
} while (0)


/* Check the sign in ordinary representation, independently of fp_encode. */
static bool fp2_sqrt_has_canonical_sign(const fp2_t *root)
{
    fp2_t ordinary;
    fp2_frommont(&ordinary, root);
    return !(ordinary.re[0] & 1) &&
           (!fp_is_zero(root->re) || !(ordinary.im[0] & 1));
}

static bool fp2_sqrt_sign_test(void)
{
    fp2_t input, root, square;
    for (int i = 0; i < 3; i++) {
        fp2_set_zero(&input);
        if (i == 1)
            fp_set(input.re, 1);
        if (i == 2) {
            fp_copy(input.re, p);
            input.re[0]--; /* -1: the root has zero real part. */
        }
        fp2_tomont(&input, &input);
        fp2_copy(&root, &input);
        fp2_sqrt(&root);
        fp2_sqr(&square, &root);
        if (!fp2_is_equal(&square, &input) ||
            !fp2_sqrt_has_canonical_sign(&root))
            return false;
    }
    return true;
}

bool fp2_test()
{ // Tests for the GF(p^2) arithmetic
    bool OK = true;
    int n, passed;
    fp2_t a, b, c, d, e, f, ma, mb, mc, md, me, mf;

    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Testing arithmetic over GF(p^2): \n\n"); 

    if (!fp2_sqrt_sign_test()) {
        printf("  GF(p^2) square root sign tests... FAILED\n");
        return false;
    }
    printf("  GF(p^2) square root sign tests ................................... PASSED\n");

    // Addition in GF(p^2)
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {
        fp2random_test(&a); fp2_tomont(&a, &a); fp2random_test(&b); fp2_tomont(&b, &b); fp2random_test(&c); fp2_tomont(&c, &c); fp2random_test(&d); fp2_tomont(&d, &d);

        fp2_add(&d, &a, &b); fp2_add(&e, &d, &c);                 // e = (a+b)+c
        fp2_add(&d, &b, &c); fp2_add(&f, &d, &a);                 // f = a+(b+c)
        if (!fp2_is_equal(&e, &f)) { passed=0; break; }

        fp2_add(&d, &a, &b);                                      // d = a+b 
        fp2_add(&e, &b, &a);                                      // e = b+a
        if (!fp2_is_equal(&d, &e)) { passed=0; break; }

        fp2_set(&b, 0);
        fp2_add(&d, &a, &b);                                      // d = a+0 
        if (!fp2_is_equal(&a, &d)) { passed=0; break; }

        fp2_set(&b, 0);   
        fp2_neg(&d, &a);                      
        fp2_add(&e, &a, &d);                                      // e = a+(-a)
        if (!fp2_is_equal(&e, &b)) { passed=0; break; }
    }
    if (passed==1) printf("  GF(p^2) addition tests ............................................ PASSED");
    else { printf("  GF(p^2) addition tests... FAILED"); printf("\n"); return false; }
    printf("\n");
    
    // Subtraction in GF(p^2)
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {
        fp2random_test(&a); fp2_tomont(&a, &a); fp2random_test(&b); fp2_tomont(&b, &b); fp2random_test(&c); fp2_tomont(&c, &c); fp2random_test(&d); fp2_tomont(&d, &d);

        fp2_sub(&d, &a, &b); fp2_sub(&e, &d, &c);                 // e = (a-b)-c
        fp2_add(&d, &b, &c); fp2_sub(&f, &a, &d);                 // f = a-(b+c)
        if (!fp2_is_equal(&e, &f)) { passed=0; break; }

        fp2_sub(&d, &a, &b);                                      // d = a-b 
        fp2_sub(&e, &b, &a);
        fp2_neg(&e, &e);                                          // e = -(b-a)
        if (!fp2_is_equal(&d, &e)) { passed=0; break; }

        fp2_set(&b, 0);
        fp2_sub(&d, &a, &b);                                      // d = a-0 
        if (!fp2_is_equal(&a, &d)) { passed=0; break; }
        
        fp2_set(&b, 0);              
        fp2_sub(&e, &a, &a);                                      // e = a+(-a)
        if (!fp2_is_equal(&e, &b)) { passed=0; break; }
    }
    if (passed==1) printf("  GF(p^2) subtraction tests ......................................... PASSED");
    else { printf("  GF(p^2) subtraction tests... FAILED"); printf("\n"); return false; }
    printf("\n");

    // Multiplication in GF(p^2)
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {    
        fp2random_test(&a); fp2random_test(&b); fp2random_test(&c);
        
        fp2_tomont(&ma, &a);
        fp2_frommont(&c, &ma);
        if (compare_words((digit_t*)&a, (digit_t*)&c, 2*NWORDS_FIELD)!=0) { passed=0; break; }

        fp2_tomont(&ma, &a); fp2_tomont(&mb, &b); fp2_tomont(&mc, &c);
        fp2_mul(&md, &ma, &mb); fp2_mul(&me, &md, &mc);                          // e = (a*b)*c
        fp2_mul(&md, &mb, &mc); fp2_mul(&mf, &md, &ma);                          // f = a*(b*c)
        fp2_frommont(&e, &me);
        fp2_frommont(&f, &mf);
        if (compare_words((digit_t*)&e, (digit_t*)&f, 2*NWORDS_FIELD)!=0) { passed=0; break; }

        fp2_tomont(&ma, &a); fp2_tomont(&mb, &b); fp2_tomont(&mc, &c); 
        fp2_add(&md, &mb, &mc); fp2_mul(&me, &ma, &md);                          // e = a*(b+c)
        fp2_mul(&md, &ma, &mb); fp2_mul(&mf, &ma, &mc); fp2_add(&mf, &md, &mf);  // f = a*b+a*c
        fp2_frommont(&e, &me);
        fp2_frommont(&f, &mf);
        if (compare_words((digit_t*)&e, (digit_t*)&f, 2*NWORDS_FIELD)!=0) { passed=0; break; }
     
        fp2_tomont(&ma, &a); fp2_tomont(&mb, &b);
        fp2_mul(&md, &ma, &mb);                                                  // d = a*b 
        fp2_mul(&me, &mb, &ma);                                                  // e = b*a 
        fp2_frommont(&d, &md);
        fp2_frommont(&e, &me);
        if (compare_words((digit_t*)&d, (digit_t*)&e, 2*NWORDS_FIELD)!=0) { passed=0; break; }

        fp2_tomont(&ma, &a);
        fp2_set(&b, 1); fp2_tomont(&mb, &b);
        fp2_mul(&md, &ma, &mb);                                                  // d = a*1  
        fp2_frommont(&a, &ma);
        fp2_frommont(&d, &md);                
        if (compare_words((digit_t*)&a, (digit_t*)&d, 2*NWORDS_FIELD)!=0) { passed=0; break; }
       
        fp2_set(&b, 0);
        fp2_tomont(&mb, &b);
        fp2_mul(&md, &ma, &mb);                                                  // d = a*0 
        fp2_frommont(&d, &md);                
        if (compare_words((digit_t*)&b, (digit_t*)&d, 2*NWORDS_FIELD)!=0) { passed=0; break; }
    }
    if (passed==1) printf("  GF(p^2) multiplication tests ...................................... PASSED");
    else { printf("  GF(p^2) multiplication tests... FAILED"); printf("\n"); return false; }
    printf("\n");

    // Squaring in GF(p^2)
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {
        fp2random_test(&a);
        
        fp2_tomont(&ma, &a);
        fp2_sqr(&mb, &ma);                                          // b = a^2
        fp2_mul(&mc, &ma, &ma);                                     // c = a*a 
        fp2_frommont(&b, &mb);
        fp2_frommont(&c, &mc);
        if (compare_words((digit_t*)&b, (digit_t*)&c, 2*NWORDS_FIELD)!=0) { passed=0; break; }

        fp2_set(&a, 0); fp2_tomont(&ma, &a);
        fp2_sqr(&md, &ma);                                          // d = 0^2 
        if (!fp2_is_equal(&ma, &md)) { passed=0; break; }
    }
    if (passed==1) printf("  GF(p^2) squaring tests............................................. PASSED");
    else { printf("  GF(p^2) squaring tests... FAILED"); printf("\n"); return false; }
    printf("\n");
    
    // Inversion in GF(p^2)
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {
        fp2random_test(&a);

        fp2_tomont(&ma, &a);
        fp2_set(&d, 1);
        memcpy(&mb, &ma, RADIX/8 * 2*NWORDS_FIELD);
        fp2_inv(&ma);
        fp2_mul(&mc, &ma, &mb);                                     // c = a*a^-1 
        fp2_frommont(&c, &mc);
        if (compare_words((digit_t*)&c, (digit_t*)&d, 2*NWORDS_FIELD) != 0) { passed = 0; break; }

        fp2_set(&a, 0);
        fp2_set(&d, 0);
        fp2_inv(&a);                                                // c = 0^-1
        if (compare_words((digit_t*)&a, (digit_t*)&d, 2*NWORDS_FIELD) != 0) { passed = 0; break; }
    }
    if (passed == 1) printf("  GF(p^2) inversion tests............................................ PASSED");
    else { printf("  GF(p^2) inversion tests... FAILED"); printf("\n"); return false; }
    printf("\n");
    
    // Square root and square detection in GF(p^2)
    passed = 1;
    for (n=0; n<TEST_LOOPS; n++)
    {
        fp2random_test(&a);

        fp2_tomont(&ma, &a);
        fp2_sqr(&mc, &ma);
        fp2_frommont(&c, &mc);                                      // c = a^2
        if (fp2_is_square(&mc) != 1) { passed = 0; break; }        

        fp2_sqrt(&mc);                                              // c = a = sqrt(c) 
        if (!fp2_sqrt_has_canonical_sign(&mc)) { passed = 0; break; }
        fp2_neg(&md, &mc);
        fp2_frommont(&c, &mc);
        fp2_frommont(&d, &md);
        if ((compare_words((digit_t*)&a, (digit_t*)&c, 2*NWORDS_FIELD) != 0) & (compare_words((digit_t*)&a, (digit_t*)&d, 2*NWORDS_FIELD) != 0)) { passed = 0; break; }
    }
    if (passed == 1) printf("  Square root, square tests.......................................... PASSED");
    else { printf("  Square root, square tests... FAILED"); printf("\n"); return false; }
    printf("\n");

    return OK;
}

bool fp2_run()
{
    bool OK = true;
    unsigned int square_result = 0;
    fp2_t a, b, chain;
        
    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Benchmarking arithmetic over GF(p^2): \n\n"); 
        
    fp2random_test(&a); fp2_tomont(&a, &a); fp2random_test(&b); fp2_tomont(&b, &b);

    BENCH_FP2_CHAIN("GF(p^2) addition", fp2_add(&chain, &chain, &b));
    BENCH_FP2_CHAIN("GF(p^2) subtraction", fp2_sub(&chain, &chain, &b));
    BENCH_FP2_CHAIN("GF(p^2) squaring", fp2_sqr(&chain, &chain));
    BENCH_FP2_CHAIN("GF(p^2) multiplication", fp2_mul(&chain, &chain, &b));
    BENCH_FP2_CHAIN("GF(p^2) inversion", fp2_inv(&chain));
    BENCH_FP2_CHAIN("GF(p^2) square root", fp2_sqrt(&chain));
    BENCH_FP2_CHAIN("Square checking",
                    square_result ^= (unsigned int)fp2_is_square_bench(&chain));
    fp2_bench_sink ^= square_result;

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
        return !fp2_test();
    } else if (!strcmp(argv[1], "bench")) {
        BENCH_LOOPS = atoi(argv[2]);
        if (BENCH_LOOPS <= 0) {
            printf("The benchmark repetition count must be positive.\n");
            return 1;
        }
        return !fp2_run();
    } else {
        exit(1);
    }
}
