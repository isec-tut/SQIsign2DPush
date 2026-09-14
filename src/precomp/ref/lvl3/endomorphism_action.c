#include <endomorphism_action.h>
#include <stddef.h>
#include <stdint.h>
const ec_basis_t BASIS_EVEN = {
    {{{0x79f064aa8262e0, 0x6e339251faa5f8, 0xfeea99078a700, 0x3726ceafbb705c, 0x5f391ef6fdc438, 0xa81a33909d73d, 0x452cfcbf4d0},
      {0x143c162368b21f, 0x7068cd84b7d05a, 0x15e71660498807, 0x40c418dc2815ae, 0x6abd7726074382, 0x266ce9c2859fc5, 0x40443aa2fe2a}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x42af0a1c902c87, 0x3e72eadcdb1077, 0x721cb91e1839de, 0x62e1d8abaafd03, 0x7fec3f1450fa2b, 0x31eace503a2233, 0x15d1ed1a6f86},
      {0x361ce365d6bef, 0x2dfa3c4f4f4632, 0x13d23fcad64ce8, 0x73d86f8133ec8c, 0x1da3d287cd927d, 0x669a2005c1358e, 0x98021d7414c}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0xa52dfa8747f31, 0x380f6910b3addf, 0x2d2729056009ff, 0x67b9801e4f4f54, 0x9869c7dfb545, 0x7a14c0f84ae3d2, 0x366d8937957d},
      {0x38b7960f2b0090, 0xd0c1d53c030f9, 0x7bba0abd403d63, 0x36e251ede0673d, 0x33250ca197f2b1, 0x1fb99abc21afcb, 0x3c3744a51b58}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}}};
const ec_basis_t BASIS_THREE = {
    {{{0x6adcc098e33724, 0x138c9ae0e6be56, 0x2bc4f6f098f460, 0x1e66d7f5fa51b2, 0x594d687976fb60, 0x7a1fad54fd9cb3, 0x1a39c2bd8a90},
      {0x6db28637b1b5ef, 0x73cb24db71e972, 0x71344c372d0d32, 0x51ca233812952c, 0x55a1234658663, 0xe701c3a909bea, 0x2cf365f7c817}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x18233978a1704f, 0x69ba4d52797ab9, 0x387bbfdf1ecc07, 0x303013f930ba8, 0x61b4737c66e090, 0x3af8ca0798b573, 0x4c295426031b},
      {0x6ba9c24c72dfda, 0x89e0faada51f8, 0x3d6170d114eb62, 0x72460713bc016f, 0x47e3452a819e1, 0x3959991474d5a1, 0x4b4d67a80223}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x4caeb7ce25548a, 0x2f7a3b12aa6e54, 0x130b6446f6bfd0, 0x16d61ca7a3df3c, 0x28d38c64f430af, 0x7d4e6c2c80ffbf, 0x4c013c1b68a8},
      {0x58226b5bd04f23, 0x1659035267defc, 0x38166b8c806613, 0x7a2e8b78a14e9f, 0x219dfa7689ddab, 0x14e4e8148d491f, 0x2685dba4a627}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}}};
const ec_basis_t BASIS_ODD_PLUS = {
    {{{0x6adcc098e33724, 0x138c9ae0e6be56, 0x2bc4f6f098f460, 0x1e66d7f5fa51b2, 0x594d687976fb60, 0x7a1fad54fd9cb3, 0x1a39c2bd8a90},
      {0x6db28637b1b5ef, 0x73cb24db71e972, 0x71344c372d0d32, 0x51ca233812952c, 0x55a1234658663, 0xe701c3a909bea, 0x2cf365f7c817}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x18233978a1704f, 0x69ba4d52797ab9, 0x387bbfdf1ecc07, 0x303013f930ba8, 0x61b4737c66e090, 0x3af8ca0798b573, 0x4c295426031b},
      {0x6ba9c24c72dfda, 0x89e0faada51f8, 0x3d6170d114eb62, 0x72460713bc016f, 0x47e3452a819e1, 0x3959991474d5a1, 0x4b4d67a80223}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x4caeb7ce25548a, 0x2f7a3b12aa6e54, 0x130b6446f6bfd0, 0x16d61ca7a3df3c, 0x28d38c64f430af, 0x7d4e6c2c80ffbf, 0x4c013c1b68a8},
      {0x58226b5bd04f23, 0x1659035267defc, 0x38166b8c806613, 0x7a2e8b78a14e9f, 0x219dfa7689ddab, 0x14e4e8148d491f, 0x2685dba4a627}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}}};
const ec_basis_t BASIS_ODD_MINUS = {
    {{{0x6c6d4b783cab42, 0x140ad300392e58, 0x6468ebb0ef443d, 0x114a447677470e, 0x3cf418dcf16519, 0x5a340cafaf9c73, 0x23fdb57ff369},
      {0xb9fbe203d95bb, 0x73b4bc96394a2e, 0x51bf8ce8125867, 0x17cef0c0918e46, 0x39623a30f2c070, 0x6c8bdd81d0c234, 0x1c45c92a5e8}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x193e87c1ff03cd, 0x4e285b0d342b4a, 0x45bc04d86e7f2a, 0x573121048510e7, 0x25af517c93551c, 0x5e07966cc159f3, 0x24bb3293e1fa},
      {0x56280e4c6091c, 0x28f9a87d9273b6, 0x1cc86d8907e121, 0x4c103072a5f210, 0x340a81583ef9e, 0x5e4f13c94406a3, 0xcffabc873fd}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x7f11dcf9ab1764, 0x5524699deef090, 0x66e94fce0bae5d, 0x56ed6abb6138ed, 0x4119ce124298dc, 0x7541c8ca73936f, 0x20fb75cc5e76},
      {0x53f8002251d129, 0x5968dda66fa565, 0x10709440e1cecc, 0x10c8cca1a58326, 0x7c2f00d4b1261d, 0x389a540e694bde, 0x28126e3a8bfa}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}}};
const ec_basis_t BASIS_COMMITMENT_PLUS = {
    {{{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}},
     {{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}},
     {{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}},
     {{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}}};
const ec_basis_t BASIS_COMMITMENT_MINUS = {
    {{{0x6c6d4b783cab42, 0x140ad300392e58, 0x6468ebb0ef443d, 0x114a447677470e, 0x3cf418dcf16519, 0x5a340cafaf9c73, 0x23fdb57ff369},
      {0xb9fbe203d95bb, 0x73b4bc96394a2e, 0x51bf8ce8125867, 0x17cef0c0918e46, 0x39623a30f2c070, 0x6c8bdd81d0c234, 0x1c45c92a5e8}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x193e87c1ff03cd, 0x4e285b0d342b4a, 0x45bc04d86e7f2a, 0x573121048510e7, 0x25af517c93551c, 0x5e07966cc159f3, 0x24bb3293e1fa},
      {0x56280e4c6091c, 0x28f9a87d9273b6, 0x1cc86d8907e121, 0x4c103072a5f210, 0x340a81583ef9e, 0x5e4f13c94406a3, 0xcffabc873fd}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x7f11dcf9ab1764, 0x5524699deef090, 0x66e94fce0bae5d, 0x56ed6abb6138ed, 0x4119ce124298dc, 0x7541c8ca73936f, 0x20fb75cc5e76},
      {0x53f8002251d129, 0x5968dda66fa565, 0x10709440e1cecc, 0x10c8cca1a58326, 0x7c2f00d4b1261d, 0x389a540e694bde, 0x28126e3a8bfa}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}}};
const ec_basis_t BASIS_CHALLENGE = {
    {{{0x4445847dec4bf9, 0x79ea3e0148c139, 0x5c5144400d8599, 0x70e88ce8c2b375, 0xf8a479cf4ee08, 0x5aa7ca8337bc9b, 0x4241258ae77},
      {0x486045239a04ec, 0x4c5876497f8eb, 0x5d7c798b58f6a7, 0x6cac91e41f35f7, 0x7b3a0bd32ed4ee, 0x4414a5bbc8c07c, 0x1284e90cd4f6}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x29baec227901e2, 0x740a3b111f2e54, 0x52f01362deaafd, 0xd268fbf998558, 0x2f92652d7f4a6b, 0x4adddd89da1b5b, 0x5590ac45a91e},
      {0x439f1134dd9e67, 0x1290a96dc327e8, 0x12013f7cfb4ab7, 0x4c27912ac3a7af, 0x61d9c1c210b493, 0x1c5b6576982980, 0x232353d66d4b}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}},
    {{{0x39f682ada8ae76, 0x34afe17bedabd3, 0x74d5c5d41e3770, 0x584ccfc8e9b9e9, 0x43fbe5daf8d5b3, 0x1e40f9465650e5, 0x14f7c34a2aff},
      {0x229291b88d52a9, 0x72bb810bdc806d, 0x5407603ccaad74, 0x67e9714f527ed, 0xf3ce0520fa8ee, 0x51588b3a1a5862, 0x13628b90250c}},
     {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}}};
const ec_curve_t CURVE_E0 = {{{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}, {{0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}, {{{0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}, {{0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}}};
const ibz_mat_2x2_t ACTION_I = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x3efb, 0x9f20, 0x431e, 0x2149, 0x5f9b,
                                 0x2e9a, 0xe45e, 0x58af, 0x7e8e, 0x2a72,
                                 0xed61, 0x8989, 0x3e3a, 0x7167, 0xb438,
                                 0x6e58, 0x29a4, 0xf48f, 0x4047, 0xff4b,
                                 0xa9a6, 0x3452, 0xcbd9, 0x771b, 0x3}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x9f203efb, 0x2149431e, 0x2e9a5f9b, 0x58afe45e,
                                 0x2a727e8e, 0x8989ed61, 0x71673e3a, 0x6e58b438,
                                 0xf48f29a4, 0xff4b4047, 0x3452a9a6, 0x771bcbd9,
                                 0x3}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x2149431e9f203efb, 0x58afe45e2e9a5f9b,
                                 0x8989ed612a727e8e, 0x6e58b43871673e3a,
                                 0xff4b4047f48f29a4, 0x771bcbd93452a9a6, 0x3}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0xd76f, 0x5c8,  0x513e, 0x4b3d, 0x8d1,
                                 0x4151, 0xccef, 0xa91,  0x23cb, 0x8865,
                                 0x10ce, 0xb438, 0xb6cc, 0x9784, 0xa1df,
                                 0xa096, 0xdd0e, 0x780b, 0x44ec, 0x65e4,
                                 0xfa09, 0x3bb5, 0x3b6,  0xabbc, 0x4}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x5c8d76f, 0x4b3d513e, 0x415108d1, 0xa91ccef,
                                 0x886523cb, 0xb43810ce, 0x9784b6cc, 0xa096a1df,
                                 0x780bdd0e, 0x65e444ec, 0x3bb5fa09, 0xabbc03b6,
                                 0x4}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x4b3d513e05c8d76f, 0xa91ccef415108d1,
                                 0xb43810ce886523cb, 0xa096a1df9784b6cc,
                                 0x65e444ec780bdd0e, 0xabbc03b63bb5fa09, 0x4}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x657a, 0x81b4, 0x776d, 0x646a, 0x193b,
                                 0x586c, 0xc57,  0x38be, 0x746b, 0xacb9,
                                 0xb434, 0xf17,  0xb186, 0x9042, 0x6b77,
                                 0x34e7, 0x272e, 0x233e, 0xc8cf, 0xa09b,
                                 0x4ce1, 0x20f,  0x7c94, 0x5c5c, 0x1}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x81b4657a, 0x646a776d, 0x586c193b, 0x38be0c57,
                                 0xacb9746b, 0xf17b434, 0x9042b186, 0x34e76b77,
                                 0x233e272e, 0xa09bc8cf, 0x20f4ce1, 0x5c5c7c94,
                                 0x1}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x646a776d81b4657a, 0x38be0c57586c193b,
                                 0xf17b434acb9746b, 0x34e76b779042b186,
                                 0xa09bc8cf233e272e, 0x5c5c7c94020f4ce1, 0x1}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0xc105, 0x60df, 0xbce1, 0xdeb6, 0xa064,
                                 0xd165, 0x1ba1, 0xa750, 0x8171, 0xd58d,
                                 0x129e, 0xf676, 0x23f8, 0x374,  0xe6e4,
                                 0x8b27, 0x33a9, 0xaaf9, 0x9e,   0xee79,
                                 0x4c36, 0xf737, 0x9bcd, 0xf292, 0x1}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x60dfc105, 0xdeb6bce1, 0xd165a064, 0xa7501ba1,
                                 0xd58d8171, 0xf676129e, 0x37423f8, 0x8b27e6e4,
                                 0xaaf933a9, 0xee79009e, 0xf7374c36, 0xf2929bcd,
                                 0x1}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0xdeb6bce160dfc105, 0xa7501ba1d165a064,
                                 0xf676129ed58d8171, 0x8b27e6e4037423f8,
                                 0xee79009eaaf933a9, 0xf2929bcdf7374c36, 0x1}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_J = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0xa677, 0x3980, 0xb693, 0xbc0c, 0xc7c4,
                                 0xd1aa, 0x9d15, 0xdc18, 0xa058, 0x83a1,
                                 0x136d, 0x3d99, 0xecef, 0xbf8b, 0x7e4b,
                                 0xc82e, 0x732b, 0x9ccf, 0x7f18, 0x92f4,
                                 0xfef3, 0x1e6c, 0x2013, 0x534,  0x2}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x3980a677, 0xbc0cb693, 0xd1aac7c4, 0xdc189d15,
                                 0x83a1a058, 0x3d99136d, 0xbf8becef, 0xc82e7e4b,
                                 0x9ccf732b, 0x92f47f18, 0x1e6cfef3, 0x5342013,
                                 0x2}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0xbc0cb6933980a677, 0xdc189d15d1aac7c4,
                                 0x3d99136d83a1a058, 0xc82e7e4bbf8becef,
                                 0x92f47f189ccf732b, 0x53420131e6cfef3, 0x2}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0xb157, 0x6bc0, 0xd024, 0x5c77, 0x5838,
                                 0x95c3, 0x4a7d, 0x2831, 0xed16, 0xdac7,
                                 0x6d4,  0x7d3a, 0x668e, 0x98d9, 0xb5be,
                                 0x829a, 0xb391, 0x4ebb, 0x15e,  0x36b0,
                                 0x4614, 0x609f, 0xfd0b, 0xb83c, 0x2}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x6bc0b157, 0x5c77d024, 0x95c35838, 0x28314a7d,
                                 0xdac7ed16, 0x7d3a06d4, 0x98d9668e, 0x829ab5be,
                                 0x4ebbb391, 0x36b0015e, 0x609f4614, 0xb83cfd0b,
                                 0x2}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x5c77d0246bc0b157, 0x28314a7d95c35838,
                                 0x7d3a06d4dac7ed16, 0x829ab5be98d9668e,
                                 0x36b0015e4ebbb391, 0xb83cfd0b609f4614, 0x2}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0xd2d0, 0xfa08, 0x3ed2, 0xcb9c, 0x4279,
                                 0xe03f, 0xba0e, 0x6732, 0x954c, 0x3ac5,
                                 0x9830, 0x2dc,  0xb3e6, 0xfa09, 0xe4f0,
                                 0xeb28, 0xb6c8, 0x3947, 0x20de, 0x901c,
                                 0x6c04, 0x8160, 0xb3a1, 0x7486, 0x2}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0xfa08d2d0, 0xcb9c3ed2, 0xe03f4279, 0x6732ba0e,
                                 0x3ac5954c, 0x2dc9830, 0xfa09b3e6, 0xeb28e4f0,
                                 0x3947b6c8, 0x901c20de, 0x81606c04, 0x7486b3a1,
                                 0x2}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0xcb9c3ed2fa08d2d0, 0x6732ba0ee03f4279,
                                 0x2dc98303ac5954c, 0xeb28e4f0fa09b3e6,
                                 0x901c20de3947b6c8, 0x7486b3a181606c04, 0x2}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x5989, 0xc67f, 0x496c, 0x43f3, 0x383b,
                                 0x2e55, 0x62ea, 0x23e7, 0x5fa7, 0x7c5e,
                                 0xec92, 0x4266, 0x7544, 0xb54f, 0x1cd0,
                                 0x3152, 0xea22, 0x2b8,  0xc1ce, 0x5acf,
                                 0xf6ea, 0xd1c,  0x4794, 0x647a, 0x3}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d =
              (mp_limb_t[]){0xc67f5989, 0x43f3496c, 0x2e55383b, 0x23e762ea,
                            0x7c5e5fa7, 0x4266ec92, 0xb54f7544, 0x31521cd0,
                            0x2b8ea22, 0x5acfc1ce, 0xd1cf6ea, 0x647a4794, 0x3}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x43f3496cc67f5989, 0x23e762ea2e55383b,
                                 0x4266ec927c5e5fa7, 0x31521cd0b54f7544,
                                 0x5acfc1ce02b8ea22, 0x647a47940d1cf6ea, 0x3}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_K = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x20dd, 0xd6b4, 0x378b, 0x28ff, 0xbfe8,
                                 0xb35e, 0xf1f,  0x300b, 0x317d, 0xa732,
                                 0x3466, 0x1b0c, 0x8012, 0xd243, 0xcba5,
                                 0x6bf5, 0x686f, 0x72a4, 0x32c4, 0x2424,
                                 0x7539, 0xefdc, 0x59fa, 0x4c76, 0x4}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0xd6b420dd, 0x28ff378b, 0xb35ebfe8, 0x300b0f1f,
                                 0xa732317d, 0x1b0c3466, 0xd2438012, 0x6bf5cba5,
                                 0x72a4686f, 0x242432c4, 0xefdc7539, 0x4c7659fa,
                                 0x4}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x28ff378bd6b420dd, 0x300b0f1fb35ebfe8,
                                 0x1b0c3466a732317d, 0x6bf5cba5d2438012,
                                 0x242432c472a4686f, 0x4c7659faefdc7539, 0x4}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0xd3b4, 0x2a32, 0x6286, 0x45fd, 0x72b3,
                                 0x2e1f, 0x7e95, 0xc292, 0x3fad, 0xb52d,
                                 0x8744, 0xdd96, 0x2759, 0xd8c0, 0x10ed,
                                 0x2ba1, 0xe8db, 0x7721, 0x951e, 0x931,
                                 0xa430, 0x2bc5, 0x9438, 0x13ec, 0x3}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x2a32d3b4, 0x45fd6286, 0x2e1f72b3, 0xc2927e95,
                                 0xb52d3fad, 0xdd968744, 0xd8c02759, 0x2ba110ed,
                                 0x7721e8db, 0x931951e, 0x2bc5a430, 0x13ec9438,
                                 0x3}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x45fd62862a32d3b4, 0xc2927e952e1f72b3,
                                 0xdd968744b52d3fad, 0x2ba110edd8c02759,
                                 0x931951e7721e8db, 0x13ec94382bc5a430, 0x3}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 24,
          ._mp_d =
              (mp_limb_t[]){0x35c6, 0x57f3, 0xfa08, 0xfd6a, 0xa8f3, 0x1173,
                            0x2cb7, 0xf002, 0x13e1, 0xf3a0, 0xafa4, 0x4ca,
                            0x24f8, 0x2c02, 0xc293, 0x427d, 0x6b19, 0x1dbd,
                            0xffe4, 0x85f7, 0x9978, 0x8cdc, 0x9cae, 0xf951}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 12,
          ._mp_d =
              (mp_limb_t[]){0x57f335c6, 0xfd6afa08, 0x1173a8f3, 0xf0022cb7,
                            0xf3a013e1, 0x4caafa4, 0x2c0224f8, 0x427dc293,
                            0x1dbd6b19, 0x85f7ffe4, 0x8cdc9978, 0xf9519cae}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 6,
          ._mp_d = (mp_limb_t[]){0xfd6afa0857f335c6, 0xf0022cb71173a8f3,
                                 0x4caafa4f3a013e1, 0x427dc2932c0224f8,
                                 0x85f7ffe41dbd6b19, 0xf9519cae8cdc9978}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0xdf23, 0x294b, 0xc874, 0xd700, 0x4017,
                                 0x4ca1, 0xf0e0, 0xcff4, 0xce82, 0x58cd,
                                 0xcb99, 0x64f3, 0xe221, 0xa297, 0xcf76,
                                 0x8d8a, 0xf4de, 0x2ce3, 0xe22,  0xc9a0,
                                 0x80a4, 0x3bad, 0xdac,  0x1d38, 0x1}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x294bdf23, 0xd700c874, 0x4ca14017, 0xcff4f0e0,
                                 0x58cdce82, 0x64f3cb99, 0xa297e221, 0x8d8acf76,
                                 0x2ce3f4de, 0xc9a00e22, 0x3bad80a4, 0x1d380dac,
                                 0x1}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0xd700c874294bdf23, 0xcff4f0e04ca14017,
                                 0x64f3cb9958cdce82, 0x8d8acf76a297e221,
                                 0xc9a00e222ce3f4de, 0x1d380dac3bad80a4, 0x1}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_GEN2 = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x3efb, 0x9f20, 0x431e, 0x2149, 0x5f9b,
                                 0x2e9a, 0xe45e, 0x58af, 0x7e8e, 0x2a72,
                                 0xed61, 0x8989, 0x3e3a, 0x7167, 0xb438,
                                 0x6e58, 0x29a4, 0xf48f, 0x4047, 0xff4b,
                                 0xa9a6, 0x3452, 0xcbd9, 0x771b, 0x3}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x9f203efb, 0x2149431e, 0x2e9a5f9b, 0x58afe45e,
                                 0x2a727e8e, 0x8989ed61, 0x71673e3a, 0x6e58b438,
                                 0xf48f29a4, 0xff4b4047, 0x3452a9a6, 0x771bcbd9,
                                 0x3}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x2149431e9f203efb, 0x58afe45e2e9a5f9b,
                                 0x8989ed612a727e8e, 0x6e58b43871673e3a,
                                 0xff4b4047f48f29a4, 0x771bcbd93452a9a6, 0x3}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0xd76f, 0x5c8,  0x513e, 0x4b3d, 0x8d1,
                                 0x4151, 0xccef, 0xa91,  0x23cb, 0x8865,
                                 0x10ce, 0xb438, 0xb6cc, 0x9784, 0xa1df,
                                 0xa096, 0xdd0e, 0x780b, 0x44ec, 0x65e4,
                                 0xfa09, 0x3bb5, 0x3b6,  0xabbc, 0x4}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x5c8d76f, 0x4b3d513e, 0x415108d1, 0xa91ccef,
                                 0x886523cb, 0xb43810ce, 0x9784b6cc, 0xa096a1df,
                                 0x780bdd0e, 0x65e444ec, 0x3bb5fa09, 0xabbc03b6,
                                 0x4}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x4b3d513e05c8d76f, 0xa91ccef415108d1,
                                 0xb43810ce886523cb, 0xa096a1df9784b6cc,
                                 0x65e444ec780bdd0e, 0xabbc03b63bb5fa09, 0x4}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x657a, 0x81b4, 0x776d, 0x646a, 0x193b,
                                 0x586c, 0xc57,  0x38be, 0x746b, 0xacb9,
                                 0xb434, 0xf17,  0xb186, 0x9042, 0x6b77,
                                 0x34e7, 0x272e, 0x233e, 0xc8cf, 0xa09b,
                                 0x4ce1, 0x20f,  0x7c94, 0x5c5c, 0x1}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x81b4657a, 0x646a776d, 0x586c193b, 0x38be0c57,
                                 0xacb9746b, 0xf17b434, 0x9042b186, 0x34e76b77,
                                 0x233e272e, 0xa09bc8cf, 0x20f4ce1, 0x5c5c7c94,
                                 0x1}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x646a776d81b4657a, 0x38be0c57586c193b,
                                 0xf17b434acb9746b, 0x34e76b779042b186,
                                 0xa09bc8cf233e272e, 0x5c5c7c94020f4ce1, 0x1}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0xc105, 0x60df, 0xbce1, 0xdeb6, 0xa064,
                                 0xd165, 0x1ba1, 0xa750, 0x8171, 0xd58d,
                                 0x129e, 0xf676, 0x23f8, 0x374,  0xe6e4,
                                 0x8b27, 0x33a9, 0xaaf9, 0x9e,   0xee79,
                                 0x4c36, 0xf737, 0x9bcd, 0xf292, 0x1}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x60dfc105, 0xdeb6bce1, 0xd165a064, 0xa7501ba1,
                                 0xd58d8171, 0xf676129e, 0x37423f8, 0x8b27e6e4,
                                 0xaaf933a9, 0xee79009e, 0xf7374c36, 0xf2929bcd,
                                 0x1}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0xdeb6bce160dfc105, 0xa7501ba1d165a064,
                                 0xf676129ed58d8171, 0x8b27e6e4037423f8,
                                 0xee79009eaaf933a9, 0xf2929bcdf7374c36, 0x1}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_GEN3 = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x72b9, 0xec50, 0xfcd8, 0xeeaa, 0x93af,
                                 0x22,   0x40ba, 0x9a64, 0xf73,  0x570a,
                                 0x8067, 0xe391, 0x9594, 0x1879, 0x9942,
                                 0x1b43, 0x4e68, 0x48af, 0xdfb0, 0x491f,
                                 0xd44d, 0x295f, 0xf5f6, 0xbe27, 0x2}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0xec5072b9, 0xeeaafcd8, 0x2293af, 0x9a6440ba,
                                 0x570a0f73, 0xe3918067, 0x18799594, 0x1b439942,
                                 0x48af4e68, 0x491fdfb0, 0x295fd44d, 0xbe27f5f6,
                                 0x2}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0xeeaafcd8ec5072b9, 0x9a6440ba002293af,
                                 0xe3918067570a0f73, 0x1b43994218799594,
                                 0x491fdfb048af4e68, 0xbe27f5f6295fd44d, 0x2}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 24,
          ._mp_d =
              (mp_limb_t[]){0xc463, 0x38c4, 0x90b1, 0xd3da, 0x3084, 0x6b8a,
                            0x8bb6, 0x9961, 0x8870, 0xb196, 0xbd1,  0xd8b9,
                            0x5d93, 0xddc1, 0x5e40, 0x14d8, 0x99a9, 0x139f,
                            0x2b2,  0xd768, 0xa51f, 0x3865, 0x4c8d, 0xfd25}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 12,
          ._mp_d =
              (mp_limb_t[]){0x38c4c463, 0xd3da90b1, 0x6b8a3084, 0x99618bb6,
                            0xb1968870, 0xd8b90bd1, 0xddc15d93, 0x14d85e40,
                            0x139f99a9, 0xd76802b2, 0x3865a51f, 0xfd254c8d}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 6,
          ._mp_d = (mp_limb_t[]){0xd3da90b138c4c463, 0x99618bb66b8a3084,
                                 0xd8b90bd1b1968870, 0x14d85e40ddc15d93,
                                 0xd76802b2139f99a9, 0xfd254c8d3865a51f}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x9c25, 0x3dde, 0x5b20, 0x9803, 0xadda,
                                 0x1c55, 0x6333, 0xcff8, 0x84db, 0x73bf,
                                 0x2632, 0x8fa,  0x32b6, 0x4526, 0x2834,
                                 0x9008, 0xeefb, 0xae42, 0xf4d6, 0x185b,
                                 0xdc73, 0xc1b7, 0x981a, 0xe871, 0x1}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x3dde9c25, 0x98035b20, 0x1c55adda, 0xcff86333,
                                 0x73bf84db, 0x8fa2632, 0x452632b6, 0x90082834,
                                 0xae42eefb, 0x185bf4d6, 0xc1b7dc73, 0xe871981a,
                                 0x1}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x98035b203dde9c25, 0xcff863331c55adda,
                                 0x8fa263273bf84db, 0x90082834452632b6,
                                 0x185bf4d6ae42eefb, 0xe871981ac1b7dc73, 0x1}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x8d47, 0x13af, 0x327,  0x1155, 0x6c50,
                                 0xffdd, 0xbf45, 0x659b, 0xf08c, 0xa8f5,
                                 0x7f98, 0x9c6e, 0xcc9e, 0x5c61, 0x1da,
                                 0xde3d, 0xee5,  0x56d9, 0x6136, 0xa4a4,
                                 0x2190, 0x22a,  0x71b1, 0xab86, 0x2}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x13af8d47, 0x11550327, 0xffdd6c50, 0x659bbf45,
                                 0xa8f5f08c, 0x9c6e7f98, 0x5c61cc9e, 0xde3d01da,
                                 0x56d90ee5, 0xa4a46136, 0x22a2190, 0xab8671b1,
                                 0x2}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x1155032713af8d47, 0x659bbf45ffdd6c50,
                                 0x9c6e7f98a8f5f08c, 0xde3d01da5c61cc9e,
                                 0xa4a4613656d90ee5, 0xab8671b1022a2190, 0x2}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_GEN4 = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x106f, 0xeb5a, 0x9bc5, 0x147f, 0x5ff4,
                                 0xd9af, 0x878f, 0x9805, 0x18be, 0x5399,
                                 0x1a33, 0xcd86, 0x7122, 0x238f, 0x3361,
                                 0xb2bb, 0x62de, 0x8916, 0x39d5, 0x88f4,
                                 0x358b, 0xdb3,  0x60d1, 0xdb12, 0x4}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0xeb5a106f, 0x147f9bc5, 0xd9af5ff4, 0x9805878f,
                                 0x539918be, 0xcd861a33, 0x238f7122, 0xb2bb3361,
                                 0x891662de, 0x88f439d5, 0xdb3358b, 0xdb1260d1,
                                 0x4}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0x147f9bc5eb5a106f, 0x9805878fd9af5ff4,
                                 0xcd861a33539918be, 0xb2bb3361238f7122,
                                 0x88f439d5891662de, 0xdb1260d10db3358b, 0x4}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 25,
          ._mp_d = (mp_limb_t[]){0x69da, 0x1519, 0xb143, 0xa2fe, 0xb959,
                                 0x970f, 0x3f4a, 0xe149, 0x9fd6, 0x5a96,
                                 0x43a2, 0xaecb, 0xc4c6, 0x26cd, 0xd605,
                                 0x9290, 0x2314, 0x8b55, 0xeb02, 0xfb7a,
                                 0xcd06, 0xaba7, 0x7def, 0x3ecd, 0x4}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 13,
          ._mp_d = (mp_limb_t[]){0x151969da, 0xa2feb143, 0x970fb959, 0xe1493f4a,
                                 0x5a969fd6, 0xaecb43a2, 0x26cdc4c6, 0x9290d605,
                                 0x8b552314, 0xfb7aeb02, 0xaba7cd06, 0x3ecd7def,
                                 0x4}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 7,
          ._mp_d = (mp_limb_t[]){0xa2feb143151969da, 0xe1493f4a970fb959,
                                 0xaecb43a25a969fd6, 0x9290d60526cdc4c6,
                                 0xfb7aeb028b552314, 0x3ecd7defaba7cd06, 0x4}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 24,
          ._mp_d =
              (mp_limb_t[]){0x9ae3, 0x2bf9, 0x7d04, 0xfeb5, 0xd479, 0x88b9,
                            0x165b, 0xf801, 0x9f0,  0x79d0, 0x57d2, 0x265,
                            0x127c, 0x9601, 0xe149, 0xa13e, 0xb58c, 0xede,
                            0xfff2, 0x42fb, 0x4cbc, 0x466e, 0xce57, 0x7ca8}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 12,
          ._mp_d =
              (mp_limb_t[]){0x2bf99ae3, 0xfeb57d04, 0x88b9d479, 0xf801165b,
                            0x79d009f0, 0x26557d2, 0x9601127c, 0xa13ee149,
                            0xedeb58c, 0x42fbfff2, 0x466e4cbc, 0x7ca8ce57}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 6,
          ._mp_d = (mp_limb_t[]){0xfeb57d042bf99ae3, 0xf801165b88b9d479,
                                 0x26557d279d009f0, 0xa13ee1499601127c,
                                 0x42fbfff20edeb58c, 0x7ca8ce57466e4cbc}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 24,
          ._mp_d =
              (mp_limb_t[]){0xef92, 0x14a5, 0x643a, 0xeb80, 0xa00b, 0x2650,
                            0x7870, 0x67fa, 0xe741, 0xac66, 0xe5cc, 0xb279,
                            0xf110, 0x514b, 0x67bb, 0x46c5, 0xfa6f, 0x1671,
                            0x711,  0x64d0, 0xc052, 0x1dd6, 0x6d6,  0x8e9c}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 12,
          ._mp_d =
              (mp_limb_t[]){0x14a5ef92, 0xeb80643a, 0x2650a00b, 0x67fa7870,
                            0xac66e741, 0xb279e5cc, 0x514bf110, 0x46c567bb,
                            0x1671fa6f, 0x64d00711, 0x1dd6c052, 0x8e9c06d6}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 6,
          ._mp_d = (mp_limb_t[]){0xeb80643a14a5ef92, 0x67fa78702650a00b,
                                 0xb279e5ccac66e741, 0x46c567bb514bf110,
                                 0x64d007111671fa6f, 0x8e9c06d61dd6c052}}}
#endif
    }};
const quat_alg_elem_t COMMITMENT_IDEAL_UNDISTORTED_GEN = {
#if 0
#elif GMP_LIMB_BITS == 16
    {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x1}}}
#elif GMP_LIMB_BITS == 32
    {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x1}}}
#elif GMP_LIMB_BITS == 64
    {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x1}}}
#endif
    ,
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x1}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x1}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x1}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0, ._mp_size = 0, ._mp_d = (mp_limb_t[]){0x0}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0, ._mp_size = 0, ._mp_d = (mp_limb_t[]){0x0}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 0, ._mp_d = (mp_limb_t[]){0x0}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x2e2}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x2e2}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x2e2}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x153}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x153}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x153}}}
#endif
    }};
const quat_alg_elem_t COMMITMENT_IDEAL_DISTORTION_ENDO = {
#if 0
#elif GMP_LIMB_BITS == 16
    {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x1}}}
#elif GMP_LIMB_BITS == 32
    {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x1}}}
#elif GMP_LIMB_BITS == 64
    {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x1}}}
#endif
    ,
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x2af}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x2af}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x2af}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x77}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x77}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x77}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0, ._mp_size = 0, ._mp_d = (mp_limb_t[]){0x0}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0, ._mp_size = 0, ._mp_d = (mp_limb_t[]){0x0}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 0, ._mp_d = (mp_limb_t[]){0x0}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0, ._mp_size = 0, ._mp_d = (mp_limb_t[]){0x0}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0, ._mp_size = 0, ._mp_d = (mp_limb_t[]){0x0}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 0, ._mp_d = (mp_limb_t[]){0x0}}}
#endif
    }};
