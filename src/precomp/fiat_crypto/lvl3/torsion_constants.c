#include <stddef.h>
#include <stdint.h>
#include <torsion_constants.h>
const uint64_t TORSION_PLUS_EVEN_POWER = 191;
const uint64_t TORSION_ODD_PRIMES[2] = {3, 0x3fd};
const uint64_t TORSION_ODD_POWERS[2] = {117, 1};
const uint64_t TORSION_PLUS_ODD_PRIMES[1] = {3};
const size_t TORSION_PLUS_ODD_POWERS[1] = {117};
const uint64_t TORSION_MINUS_ODD_PRIMES[1] = {0x3fd};
const size_t TORSION_MINUS_ODD_POWERS[1] = {1};
const size_t DEGREE_COMMITMENT_POWERS[2] = {0, 1};
const ibz_t CHARACTERISTIC = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 24, ._mp_d = (mp_limb_t[]) {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0x7fff,0x5699,0xbc2a,0xd546,0xb46,0x7cf5,0x88ce,0xcc69,0xa879,0x2196,0xc54,0x702e,0x15b}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 12, ._mp_d = (mp_limb_t[]) {0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0x7fffffff,0xbc2a5699,0xb46d546,0x88ce7cf5,0xa879cc69,0xc542196,0x15b702e}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 6, ._mp_d = (mp_limb_t[]) {0xffffffffffffffff,0xffffffffffffffff,0x7fffffffffffffff,0xb46d546bc2a5699,0xa879cc6988ce7cf5,0x15b702e0c542196}}}
#endif
;
const ibz_t TORSION_ODD = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 13, ._mp_d = (mp_limb_t[]) {0xc467,0xe9b6,0x3638,0xf301,0xba9b,0x3f10,0x81cd,0xdb88,0xebbb,0x5713,0xcf4e,0xd35c,0xa}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 7, ._mp_d = (mp_limb_t[]) {0xe9b6c467,0xf3013638,0x3f10ba9b,0xdb8881cd,0x5713ebbb,0xd35ccf4e,0xa}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0xf3013638e9b6c467,0xdb8881cd3f10ba9b,0xd35ccf4e5713ebbb,0xa}}}
#endif
;
const ibz_t TORSION_ODD_PRIMEPOWERS[2] = {
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 12, ._mp_d = (mp_limb_t[]) {0xad33,0x7854,0xaa8d,0x168d,0xf9ea,0x119c,0x98d3,0x50f3,0x432d,0x18a8,0xe05c,0x2b6}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 6, ._mp_d = (mp_limb_t[]) {0x7854ad33,0x168daa8d,0x119cf9ea,0x50f398d3,0x18a8432d,0x2b6e05c}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x168daa8d7854ad33,0x50f398d3119cf9ea,0x2b6e05c18a8432d}}}
#endif
, 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#endif
};
const ibz_t TORSION_ODD_PLUS = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 12, ._mp_d = (mp_limb_t[]) {0xad33,0x7854,0xaa8d,0x168d,0xf9ea,0x119c,0x98d3,0x50f3,0x432d,0x18a8,0xe05c,0x2b6}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 6, ._mp_d = (mp_limb_t[]) {0x7854ad33,0x168daa8d,0x119cf9ea,0x50f398d3,0x18a8432d,0x2b6e05c}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x168daa8d7854ad33,0x50f398d3119cf9ea,0x2b6e05c18a8432d}}}
#endif
;
const ibz_t TORSION_ODD_MINUS = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#endif
;
const ibz_t TORSION_PLUS_2POWER = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 12, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x8000}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 6, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x80000000}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x8000000000000000}}}
#endif
;
const ibz_t TORSION_PLUS_3POWER = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 12, ._mp_d = (mp_limb_t[]) {0xad33,0x7854,0xaa8d,0x168d,0xf9ea,0x119c,0x98d3,0x50f3,0x432d,0x18a8,0xe05c,0x2b6}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 6, ._mp_d = (mp_limb_t[]) {0x7854ad33,0x168daa8d,0x119cf9ea,0x50f398d3,0x18a8432d,0x2b6e05c}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x168daa8d7854ad33,0x50f398d3119cf9ea,0x2b6e05c18a8432d}}}
#endif
;
const ibz_t TORSION_PLUS_23POWER = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 24, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x8000,0x5699,0xbc2a,0xd546,0xb46,0x7cf5,0x88ce,0xcc69,0xa879,0x2196,0xc54,0x702e,0x15b}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 12, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x80000000,0xbc2a5699,0xb46d546,0x88ce7cf5,0xa879cc69,0xc542196,0x15b702e}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 6, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x8000000000000000,0xb46d546bc2a5699,0xa879cc6988ce7cf5,0x15b702e0c542196}}}
#endif
;
const ibz_t DEGREE_COMMITMENT = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#endif
;
const ibz_t DEGREE_COMMITMENT_PLUS = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1}}}
#endif
;
const ibz_t DEGREE_COMMITMENT_MINUS = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3fd}}}
#endif
;
const ibz_t DEGREE_CHALLENGE = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 24, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x8000,0x5699,0xbc2a,0xd546,0xb46,0x7cf5,0x88ce,0xcc69,0xa879,0x2196,0xc54,0x702e,0x15b}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 12, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x80000000,0xbc2a5699,0xb46d546,0x88ce7cf5,0xa879cc69,0xc542196,0x15b702e}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 6, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x8000000000000000,0xb46d546bc2a5699,0xa879cc6988ce7cf5,0x15b702e0c542196}}}
#endif
;
