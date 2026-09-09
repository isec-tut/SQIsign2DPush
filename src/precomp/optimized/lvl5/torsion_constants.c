#include <stddef.h>
#include <stdint.h>
#include <torsion_constants.h>
const uint64_t TORSION_PLUS_EVEN_POWER = 0x107;
const uint64_t TORSION_ODD_PRIMES[6] = {3, 11, 17, 59, 0x3b3, 0x3be1};
const uint64_t TORSION_ODD_POWERS[6] = {156, 1, 1, 1, 1, 1};
const uint64_t TORSION_PLUS_ODD_PRIMES[1] = {3};
const size_t TORSION_PLUS_ODD_POWERS[1] = {156};
const uint64_t TORSION_MINUS_ODD_PRIMES[5] = {11, 17, 59, 0x3b3, 0x3be1};
const size_t TORSION_MINUS_ODD_POWERS[5] = {1, 1, 1, 1, 1};
const size_t DEGREE_COMMITMENT_POWERS[6] = {0, 1, 1, 1, 1, 1};
const ibz_t CHARACTERISTIC = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 32, ._mp_d = (mp_limb_t[]) {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0x987f,0xacec,0xef90,0x4c46,0x969b,0xc552,0xa1c1,0xb8d8,0x20,0x4b6e,0x5c45,0x234a,0x665f,0x30bb,0x1931,0x4c54}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xacec987f,0x4c46ef90,0xc552969b,0xb8d8a1c1,0x4b6e0020,0x234a5c45,0x30bb665f,0x4c541931}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff,0x4c46ef90acec987f,0xb8d8a1c1c552969b,0x234a5c454b6e0020,0x4c54193130bb665f}}}
#endif
;
const ibz_t TORSION_ODD = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 18, ._mp_d = (mp_limb_t[]) {0xda2b,0x4791,0x4e01,0xf512,0xcb,0xf98d,0x6529,0x96e2,0xd751,0x87d1,0xf2b,0xfb01,0x1695,0x6937,0x679e,0xa43,0xa5e9,0x163c}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 9, ._mp_d = (mp_limb_t[]) {0x4791da2b,0xf5124e01,0xf98d00cb,0x96e26529,0x87d1d751,0xfb010f2b,0x69371695,0xa43679e,0x163ca5e9}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 5, ._mp_d = (mp_limb_t[]) {0xf5124e014791da2b,0x96e26529f98d00cb,0xfb010f2b87d1d751,0xa43679e69371695,0x163ca5e9}}}
#endif
;
const ibz_t TORSION_ODD_PRIMEPOWERS[6] = {
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0xd931,0x2159,0x8ddf,0x3698,0xa52d,0x838a,0xb143,0x4171,0xdc00,0x8a96,0x94b8,0xbe46,0x76cc,0x6261,0xa832,0x98}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x2159d931,0x36988ddf,0x838aa52d,0x4171b143,0x8a96dc00,0xbe4694b8,0x626176cc,0x98a832}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0x36988ddf2159d931,0x4171b143838aa52d,0xbe4694b88a96dc00,0x98a832626176cc}}}
#endif
, 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xb}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xb}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xb}}}
#endif
, 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x11}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x11}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x11}}}
#endif
, 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3b}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3b}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3b}}}
#endif
, 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3b3}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3b3}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3b3}}}
#endif
, 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3be1}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3be1}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3be1}}}
#endif
};
const ibz_t TORSION_ODD_PLUS = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0xd931,0x2159,0x8ddf,0x3698,0xa52d,0x838a,0xb143,0x4171,0xdc00,0x8a96,0x94b8,0xbe46,0x76cc,0x6261,0xa832,0x98}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x2159d931,0x36988ddf,0x838aa52d,0x4171b143,0x8a96dc00,0xbe4694b8,0x626176cc,0x98a832}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0x36988ddf2159d931,0x4171b143838aa52d,0xbe4694b88a96dc00,0x98a832626176cc}}}
#endif
;
const ibz_t TORSION_ODD_MINUS = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x921b,0x4a5a,0x25}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x4a5a921b,0x25}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x254a5a921b}}}
#endif
;
const ibz_t TORSION_PLUS_2POWER = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 17, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 9, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 5, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x80}}}
#endif
;
const ibz_t TORSION_PLUS_3POWER = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0xd931,0x2159,0x8ddf,0x3698,0xa52d,0x838a,0xb143,0x4171,0xdc00,0x8a96,0x94b8,0xbe46,0x76cc,0x6261,0xa832,0x98}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x2159d931,0x36988ddf,0x838aa52d,0x4171b143,0x8a96dc00,0xbe4694b8,0x626176cc,0x98a832}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0x36988ddf2159d931,0x4171b143838aa52d,0xbe4694b88a96dc00,0x98a832626176cc}}}
#endif
;
const ibz_t TORSION_PLUS_23POWER = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 32, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x9880,0xacec,0xef90,0x4c46,0x969b,0xc552,0xa1c1,0xb8d8,0x20,0x4b6e,0x5c45,0x234a,0x665f,0x30bb,0x1931,0x4c54}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xacec9880,0x4c46ef90,0xc552969b,0xb8d8a1c1,0x4b6e0020,0x234a5c45,0x30bb665f,0x4c541931}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x4c46ef90acec9880,0xb8d8a1c1c552969b,0x234a5c454b6e0020,0x4c54193130bb665f}}}
#endif
;
const ibz_t DEGREE_COMMITMENT = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x921b,0x4a5a,0x25}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x4a5a921b,0x25}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x254a5a921b}}}
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
{{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x921b,0x4a5a,0x25}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x4a5a921b,0x25}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x254a5a921b}}}
#endif
;
const ibz_t DEGREE_CHALLENGE = 
#if 0
#elif GMP_LIMB_BITS == 16
{{._mp_alloc = 0, ._mp_size = 32, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x9880,0xacec,0xef90,0x4c46,0x969b,0xc552,0xa1c1,0xb8d8,0x20,0x4b6e,0x5c45,0x234a,0x665f,0x30bb,0x1931,0x4c54}}}
#elif GMP_LIMB_BITS == 32
{{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xacec9880,0x4c46ef90,0xc552969b,0xb8d8a1c1,0x4b6e0020,0x234a5c45,0x30bb665f,0x4c541931}}}
#elif GMP_LIMB_BITS == 64
{{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x4c46ef90acec9880,0xb8d8a1c1c552969b,0x234a5c454b6e0020,0x4c54193130bb665f}}}
#endif
;
