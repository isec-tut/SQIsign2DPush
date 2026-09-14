#include <stddef.h>
#include <stdint.h>
#include <klpt_constants.h>
const short SMALL_PRIMES_1MOD4[11] = {5, 13, 17, 29, 37, 41, 53, 61, 73, 89, 97};
const ibz_t PROD_SMALL_PRIMES_3MOD4 = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0x173b,0x80bd,0xa9d7,0xa185}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x80bd173b,0xa185a9d7}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xa185a9d780bd173b}}}
#endif
;
