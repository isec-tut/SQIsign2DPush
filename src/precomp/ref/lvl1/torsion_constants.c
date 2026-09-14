#include <stddef.h>
#include <stdint.h>
#include <torsion_constants.h>
#if 0
#elif 8*DIGIT_LEN == 16
const uint64_t TORSION_PLUS_EVEN_POWER = 0x83;
const uint64_t TORSION_ODD_PRIMES[3] = {0x3, 0x5, 0x35};
const uint64_t TORSION_ODD_POWERS[3] = {0x4e, 0x1, 0x1};
const uint64_t TORSION_PLUS_ODD_PRIMES[1] = {0x3};
const size_t TORSION_PLUS_ODD_POWERS[1] = {0x4e};
const uint64_t TORSION_MINUS_ODD_PRIMES[2] = {0x5, 0x35};
const size_t TORSION_MINUS_ODD_POWERS[2] = {0x1, 0x1};
const size_t DEGREE_COMMITMENT_POWERS[3] = {0x0, 0x1, 0x1};
const ibz_t CHARACTERISTIC = {{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0x83c7,0xc3fa,0xc14e,0xa7ec,0xe5fc,0x9815,0xf37f,0x62d7}}};
const ibz_t TORSION_ODD = {{._mp_alloc = 0, ._mp_size = 9, ._mp_d = (mp_limb_t[]) {0x4d41,0x1bca,0x8351,0x3a82,0x6251,0x8dd5,0x61e2,0xca31,0xc}}};
const ibz_t TORSION_ODD_PRIMEPOWERS[3] = {{{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x5079,0xd87f,0x9829,0x94fd,0xbcbf,0xf302,0xfe6f,0xc5a}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x5}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x35}}}};
const ibz_t TORSION_ODD_PLUS = {{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x5079,0xd87f,0x9829,0x94fd,0xbcbf,0xf302,0xfe6f,0xc5a}}};
const ibz_t TORSION_ODD_MINUS = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x109}}};
const ibz_t TORSION_PLUS_2POWER = {{._mp_alloc = 0, ._mp_size = 9, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x8}}};
const ibz_t TORSION_PLUS_3POWER = {{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x5079,0xd87f,0x9829,0x94fd,0xbcbf,0xf302,0xfe6f,0xc5a}}};
const ibz_t TORSION_PLUS_23POWER = {{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x83c8,0xc3fa,0xc14e,0xa7ec,0xe5fc,0x9815,0xf37f,0x62d7}}};
const ibz_t DEGREE_COMMITMENT = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x109}}};
const ibz_t DEGREE_COMMITMENT_PLUS = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1}}};
const ibz_t DEGREE_COMMITMENT_MINUS = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x109}}};
const ibz_t DEGREE_CHALLENGE = {{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x83c8,0xc3fa,0xc14e,0xa7ec,0xe5fc,0x9815,0xf37f,0x62d7}}};
#elif 8*DIGIT_LEN == 32
const uint64_t TORSION_PLUS_EVEN_POWER = 0x83;
const uint64_t TORSION_ODD_PRIMES[3] = {0x3, 0x5, 0x35};
const uint64_t TORSION_ODD_POWERS[3] = {0x4e, 0x1, 0x1};
const uint64_t TORSION_PLUS_ODD_PRIMES[1] = {0x3};
const size_t TORSION_PLUS_ODD_POWERS[1] = {0x4e};
const uint64_t TORSION_MINUS_ODD_PRIMES[2] = {0x5, 0x35};
const size_t TORSION_MINUS_ODD_POWERS[2] = {0x1, 0x1};
const size_t DEGREE_COMMITMENT_POWERS[3] = {0x0, 0x1, 0x1};
const ibz_t CHARACTERISTIC = {{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xc3fa83c7,0xa7ecc14e,0x9815e5fc,0x62d7f37f}}};
const ibz_t TORSION_ODD = {{._mp_alloc = 0, ._mp_size = 5, ._mp_d = (mp_limb_t[]) {0x1bca4d41,0x3a828351,0x8dd56251,0xca3161e2,0xc}}};
const ibz_t TORSION_ODD_PRIMEPOWERS[3] = {{{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0xd87f5079,0x94fd9829,0xf302bcbf,0xc5afe6f}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x5}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x35}}}};
const ibz_t TORSION_ODD_PLUS = {{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0xd87f5079,0x94fd9829,0xf302bcbf,0xc5afe6f}}};
const ibz_t TORSION_ODD_MINUS = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x109}}};
const ibz_t TORSION_PLUS_2POWER = {{._mp_alloc = 0, ._mp_size = 5, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x8}}};
const ibz_t TORSION_PLUS_3POWER = {{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0xd87f5079,0x94fd9829,0xf302bcbf,0xc5afe6f}}};
const ibz_t TORSION_PLUS_23POWER = {{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0xc3fa83c8,0xa7ecc14e,0x9815e5fc,0x62d7f37f}}};
const ibz_t DEGREE_COMMITMENT = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x109}}};
const ibz_t DEGREE_COMMITMENT_PLUS = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1}}};
const ibz_t DEGREE_COMMITMENT_MINUS = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x109}}};
const ibz_t DEGREE_CHALLENGE = {{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0xc3fa83c8,0xa7ecc14e,0x9815e5fc,0x62d7f37f}}};
#elif 8*DIGIT_LEN == 64
const uint64_t TORSION_PLUS_EVEN_POWER = 0x83;
const uint64_t TORSION_ODD_PRIMES[3] = {0x3, 0x5, 0x35};
const uint64_t TORSION_ODD_POWERS[3] = {0x4e, 0x1, 0x1};
const uint64_t TORSION_PLUS_ODD_PRIMES[1] = {0x3};
const size_t TORSION_PLUS_ODD_POWERS[1] = {0x4e};
const uint64_t TORSION_MINUS_ODD_PRIMES[2] = {0x5, 0x35};
const size_t TORSION_MINUS_ODD_POWERS[2] = {0x1, 0x1};
const size_t DEGREE_COMMITMENT_POWERS[3] = {0x0, 0x1, 0x1};
const ibz_t CHARACTERISTIC = {{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0xffffffffffffffff,0xffffffffffffffff,0xa7ecc14ec3fa83c7,0x62d7f37f9815e5fc}}};
const ibz_t TORSION_ODD = {{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x3a8283511bca4d41,0xca3161e28dd56251,0xc}}};
const ibz_t TORSION_ODD_PRIMEPOWERS[3] = {{{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x94fd9829d87f5079,0xc5afe6ff302bcbf}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x5}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x35}}}};
const ibz_t TORSION_ODD_PLUS = {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x94fd9829d87f5079,0xc5afe6ff302bcbf}}};
const ibz_t TORSION_ODD_MINUS = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x109}}};
const ibz_t TORSION_PLUS_2POWER = {{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x8}}};
const ibz_t TORSION_PLUS_3POWER = {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x94fd9829d87f5079,0xc5afe6ff302bcbf}}};
const ibz_t TORSION_PLUS_23POWER = {{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0x0,0x0,0xa7ecc14ec3fa83c8,0x62d7f37f9815e5fc}}};
const ibz_t DEGREE_COMMITMENT = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x109}}};
const ibz_t DEGREE_COMMITMENT_PLUS = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1}}};
const ibz_t DEGREE_COMMITMENT_MINUS = {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x109}}};
const ibz_t DEGREE_CHALLENGE = {{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0x0,0x0,0xa7ecc14ec3fa83c8,0x62d7f37f9815e5fc}}};
#endif
