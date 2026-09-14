#include <endomorphism_action.h>
#include <stddef.h>
#include <stdint.h>
const ec_basis_t BASIS_EVEN = {
    {{{0x7e3cf83255413170, 0x14e01b8ce4947ea9, 0xfbb705c1fdd5320f,
       0xf7b7ee21c3726cea, 0x68ce4275cf6f9c8, 0x8a59f97e9a02a},
      {0x168a1e0b11b4590f, 0x3100fc1a33612df4, 0x82815ae2bce2cc09,
       0x3ed3a4bf72216cda, 0x7e3f04b437e9466, 0x12e408c4c266565}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0x1de157850e481643, 0x73bcf9cbab736c4, 0x7aafd03e439723c3,
       0x7e45f274bc4348d7, 0xff76271d4d373ddc, 0xd95bf13b091d92},
      {0x8c81b0e71b2eb5f7, 0xc99d0b7e8f13d3d1, 0xd33ec8c27a47f95a,
       0x19e1d7374d52b244, 0xbcbcfd399aa02b99, 0xc0b85ab4acaa65}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0x77c5296fd43a3f98, 0x13fee03da442ceb, 0xa4f4f545a4e520ac,
       0xd3e2684d8c90c34e, 0xa740c76053af433d, 0x11a932975550cb3},
      {0x3e5c5bcb07958048, 0x7ac63430754f00c, 0xde0673df774157a8,
       0x650cbf958b6e251e, 0xe66af086bf2d9928, 0x786e894a36b07e}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_THREE = {
    {{{0x95b56e604c719b92, 0x1e8c04e326b839af, 0x5fa51b25789ede13,
       0x43cbb7db01e66d7f, 0x7eb553f672ceca6b, 0x3473857b1521e8},
      {0x5cb6d9431bd8daf7, 0xa1a65cf2c936dc7a, 0x412952ce268986e5,
       0x174696d67b31cd80, 0x14add077340f694b, 0x1079ee2f5ba3f05}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0xae4c119cbc50b827, 0xd980fa6e93549e5e, 0xb930ba870f77fbe3,
       0x2186a1a7de455b60, 0x376504979a364c1e, 0x1460abf523047b7},
      {0x7e35d4e126396fed, 0x9d6c422783eab694, 0x3bc016f7ac2e1a22,
       0xa29540cf0f246071, 0x666451d3568423f1, 0x969acf500446e5}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0x9526575be712aa45, 0xd7fa0bde8ec4aa9b, 0x7a3df3c2616c88de,
       0x6327a185796d61ca, 0x39b0b203fefd469c, 0x98027836d151f5},
      {0xbf2c1135ade82791, 0xcc2659640d499f7, 0x4a14e9f702cd7190,
       0x5957b990bdb81404, 0xe7dd3869e8e44b6a, 0xfac3ce4f765f1e}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_ODD_PLUS = {
    {{{0x95b56e604c719b92, 0x1e8c04e326b839af, 0x5fa51b25789ede13,
       0x43cbb7db01e66d7f, 0x7eb553f672ceca6b, 0x3473857b1521e8},
      {0x5cb6d9431bd8daf7, 0xa1a65cf2c936dc7a, 0x412952ce268986e5,
       0x174696d67b31cd80, 0x14add077340f694b, 0x1079ee2f5ba3f05}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0xae4c119cbc50b827, 0xd980fa6e93549e5e, 0xb930ba870f77fbe3,
       0x2186a1a7de455b60, 0x376504979a364c1e, 0x1460abf523047b7},
      {0x7e35d4e126396fed, 0x9d6c422783eab694, 0x3bc016f7ac2e1a22,
       0xa29540cf0f246071, 0x666451d3568423f1, 0x969acf500446e5}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0x9526575be712aa45, 0xd7fa0bde8ec4aa9b, 0x7a3df3c2616c88de,
       0x6327a185796d61ca, 0x39b0b203fefd469c, 0x98027836d151f5},
      {0xbf2c1135ade82791, 0xcc2659640d499f7, 0x4a14e9f702cd7190,
       0x5957b990bdb81404, 0xe7dd3869e8e44b6a, 0xfac3ce4f765f1e}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_ODD_MINUS = {
    {{{0x963636a5bc1e55a1, 0xe887a502b4c00e4b, 0x677470ec8d1d761d,
       0xc6e78b28c914a447, 0xd032bebe71cde7a0, 0x47fb6affe6d368},
      {0x8b85cfdf101ecadd, 0x4b0cfced2f258e52, 0xc918e46a37f19d02,
       0x572b00a6df921a58, 0x83b2ed77cd39098c, 0xb140d02b75e27d}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0xd28c9f43e0ff81e6, 0xcfe5538a16c34d0a, 0x8510e78b7809b0d,
       0x1188054c43883d5d, 0x7296993a2c346bf5, 0xf72e7c2dee0643},
      {0xed82b1407263048e, 0xfc242a3e6a1f649c, 0x2a5f2103990db120,
       0x40ac1f7cf4c10307, 0x3c4f25101a8c1a05, 0x19ff5790e7fb79}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0x243f88ee7cd58bb2, 0x75cbb5491a677bbc, 0xb6138edcdd29f9c1,
       0x709214c6e56ed6ab, 0x72329ce4dbe08ce, 0x41f6eb98bcedd5},
      {0x5969fc001128e894, 0x39d9965a37699be9, 0xda5832620e12881c,
       0x8c48f3d44721b816, 0xbd8d1fd9f3e31ff2, 0xfddcf37b4205ad}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_COMMITMENT_PLUS = {
    {{{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
     {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
     {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
     {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_COMMITMENT_MINUS = {
    {{{0x963636a5bc1e55a1, 0xe887a502b4c00e4b, 0x677470ec8d1d761d,
       0xc6e78b28c914a447, 0xd032bebe71cde7a0, 0x47fb6affe6d368},
      {0x8b85cfdf101ecadd, 0x4b0cfced2f258e52, 0xc918e46a37f19d02,
       0x572b00a6df921a58, 0x83b2ed77cd39098c, 0xb140d02b75e27d}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0xd28c9f43e0ff81e6, 0xcfe5538a16c34d0a, 0x8510e78b7809b0d,
       0x1188054c43883d5d, 0x7296993a2c346bf5, 0xf72e7c2dee0643},
      {0xed82b1407263048e, 0xfc242a3e6a1f649c, 0x2a5f2103990db120,
       0x40ac1f7cf4c10307, 0x3c4f25101a8c1a05, 0x19ff5790e7fb79}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0x243f88ee7cd58bb2, 0x75cbb5491a677bbc, 0xb6138edcdd29f9c1,
       0x709214c6e56ed6ab, 0x72329ce4dbe08ce, 0x41f6eb98bcedd5},
      {0x5969fc001128e894, 0x39d9965a37699be9, 0xda5832620e12881c,
       0x8c48f3d44721b816, 0xbd8d1fd9f3e31ff2, 0xfddcf37b4205ad}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_CHALLENGE = {
    {{{0x4e6222c23ef625fc, 0xb0b33e7a8f805230, 0x4c2b375b8a288801,
       0xc28b1213a523b41b, 0xf366f313b6d3bacc, 0xb6003bb7870035},
      {0x3ae4302291cd0276, 0x1ed4e13161d925fe, 0x41f35f7baf8f316b,
       0x5e9976a776cac91e, 0x5296ef2301f3d9d0, 0x2509d219a9ed10}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0x9514dd76113c80f1, 0xd55fbd028ec447cb, 0xf998558a5e026c5b,
       0x296bfa5358d268fb, 0x777627686d6d7c93, 0xab21588b523d2b},
      {0xfa21cf889a6ecf33, 0x6956e4a42a5b70c9, 0x6c3a7af24027ef9f,
       0x93b3f047fad7a45f, 0xc1d2c0956a6a4d48, 0xf3febeb304a73c}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
    {{{0xf4dcfb4156d4573b, 0xc6ee0d2bf85efb6a, 0x8e9b9e9e9ab8ba83,
       0x2ed7c6ad9d84ccfc, 0x3e5195943961fdf, 0x29ef869455fe79},
      {0x1b514948dc46a954, 0x55ae9caee042f720, 0xf527eda80ec0799,
       0x8833e7eace7d14be, 0xb669ce9e25efb861, 0xd47d2e26742a10}},
     {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}}};
const ec_curve_t CURVE_E0 = {{{0}}, {{1}}, {{{2}}, {{4}}}};
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
