#include <endomorphism_action.h>
#include <stddef.h>
#include <stdint.h>
const ec_basis_t BASIS_EVEN = {
    {{{0x9d591c1ffb5911c0, 0xe6e6e0b16c276758, 0xbb65a0496c5e6198,
       0x5f5029224fef2ec6, 0x2ef045c1ebc879f8, 0xc3c42b733dbdb6d6,
       0xb27d99c86f4c7e2d, 0x2588a46f3d5a04f},
      {0x1e868581825a78ba, 0x98f329caaef97981, 0x46ad5be3a093fe4c,
       0x27a9d612b3ebcf4e, 0xf434d5c5f301b17d, 0x5cde8cd92ba5ec9f,
       0x49a6dbcb891cce95, 0x3ba128f3bbda0708}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0x41c3e2ee58183df7, 0x29cc7d6d14d31b7f, 0x75b346ecc9168482,
       0xb45519ee882acf36, 0x491f1b0de1b52447, 0x2def7302a24651c2,
       0xf1d74afe9917a8e0, 0xd62ad7e4a915b38},
      {0xe8bbf11e42c187ad, 0x1620b63d73fd935e, 0xeada7cc403252ca1,
       0x7614bf18bd7848d2, 0x689723c0742b946d, 0x520169cc1920846c,
       0xf83ebcfcd4a193c8, 0x1d6fa0f70801349}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0x9971e6833aa68642, 0x130b997dd4272fd5, 0xac4ad07ada87b087,
       0x2075d2551a0bfdb2, 0x92255da365ac9c90, 0xa905a3ccf4f39491,
       0xedb9679698beced1, 0x2695a4649dbaa349},
      {0xff810b56a9df989, 0xde118290c40b7dcd, 0x7c907acea4c97bc1,
       0xa0d9ebcf29c1fac2, 0x1fbe7f21554803c, 0x7a080e2cef9d659a,
       0x6b121e5736ca2cf7, 0xcde2dc800224811}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_THREE = {
    {{{0xe40b345d041276c3, 0xd744781e9e6b65be, 0x9d213db630bf8674,
       0xc77ca2db467c5f5a, 0xeb9d800d1af35d9f, 0xa62e76db8def4ffa,
       0x5c9c95af509479c1, 0x45d68bd832020a7f},
      {0x5be59d1f544a2123, 0x816e359236fc5396, 0xb9293b8fe41d0255,
       0xe3698ea6177c252f, 0x83a0a9087f130039, 0x2474307ab1c696c9,
       0xa1136a698698ac91, 0x129a1e537ef6f1c}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0xad650220e4bfa4b2, 0x1b50c7ccb1b88b3f, 0xb4e691ab5ec7defd,
       0xaa23a31d9b6acb3, 0xc94aa0324d46bb39, 0x6cc8d2141aedbeeb,
       0x5d02ea61e594e4f2, 0x1e96fddc8f6d169e},
      {0xaa4a441f6fdc94c5, 0x13cc48ab56ec2b20, 0x279b5f4143794b4c,
       0x6e6131faf228a969, 0x552df60a2e7a3f63, 0x5ab265bc70182e81,
       0x9de05d9046be0e36, 0x3a4c8e759d369ac5}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0xeabc85ba57b7866, 0x27d279369f3f3391, 0x6a4c32b14be0f00d,
       0x48f91f1a09f9fb7a, 0x6c48663a91f67346, 0x6ddb3ce46fbfdc96,
       0x7a305a8b739734e7, 0xe287e87e7d91db5},
      {0xe3c7911aa2e250a1, 0xa94a9ccc36e6e3a2, 0x72e918b9437876d1,
       0x552b26db125b69a3, 0x41fb6341a59dba60, 0x5d59b061c59fcd42,
       0xcb974c46119337d1, 0x220b65b647f5c30d}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_ODD_PLUS = {
    {{{0xe40b345d041276c3, 0xd744781e9e6b65be, 0x9d213db630bf8674,
       0xc77ca2db467c5f5a, 0xeb9d800d1af35d9f, 0xa62e76db8def4ffa,
       0x5c9c95af509479c1, 0x45d68bd832020a7f},
      {0x5be59d1f544a2123, 0x816e359236fc5396, 0xb9293b8fe41d0255,
       0xe3698ea6177c252f, 0x83a0a9087f130039, 0x2474307ab1c696c9,
       0xa1136a698698ac91, 0x129a1e537ef6f1c}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0xad650220e4bfa4b2, 0x1b50c7ccb1b88b3f, 0xb4e691ab5ec7defd,
       0xaa23a31d9b6acb3, 0xc94aa0324d46bb39, 0x6cc8d2141aedbeeb,
       0x5d02ea61e594e4f2, 0x1e96fddc8f6d169e},
      {0xaa4a441f6fdc94c5, 0x13cc48ab56ec2b20, 0x279b5f4143794b4c,
       0x6e6131faf228a969, 0x552df60a2e7a3f63, 0x5ab265bc70182e81,
       0x9de05d9046be0e36, 0x3a4c8e759d369ac5}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0xeabc85ba57b7866, 0x27d279369f3f3391, 0x6a4c32b14be0f00d,
       0x48f91f1a09f9fb7a, 0x6c48663a91f67346, 0x6ddb3ce46fbfdc96,
       0x7a305a8b739734e7, 0xe287e87e7d91db5},
      {0xe3c7911aa2e250a1, 0xa94a9ccc36e6e3a2, 0x72e918b9437876d1,
       0x552b26db125b69a3, 0x41fb6341a59dba60, 0x5d59b061c59fcd42,
       0xcb974c46119337d1, 0x220b65b647f5c30d}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_ODD_MINUS = {
    {{{0xf45e484453d80540, 0xf420792230f16c8, 0x427bb0135cab0b98,
       0x97a9098307909b31, 0x3813c491cf3076dc, 0xb483fd41da561883,
       0x3d2b053f89c813a3, 0x38e9d8f5662ef867},
      {0x53dd3a36cd083b51, 0x43f21025eba75a9a, 0xbe5025da7fc832e3,
       0x32dfb2c5d5f0df2e, 0xac3ac86cb4e98b81, 0x929eec8d7f881f7d,
       0x52eac5ae8f67b043, 0x484b83a60a3fc193}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0x9b529e5fd449ce30, 0x6455c3c8ab0f2e29, 0xdaf88e9ce19b6f20,
       0xf7aadf30c8393c9, 0x7edd2f7fdd7401a7, 0x3565b5cc17ad80c2,
       0x807c9cc1a5e54cb0, 0x2e9a67b8b5f9ba0f},
      {0x3fcc38a1fbdaad55, 0xdea7de4d0edae0a9, 0x23bec824c8d7e09b,
       0xe32ae68fbda186b6, 0xfe8c4ee4ed82dfcb, 0x8d70deb553f9d85c,
       0xc1368fce3843aa49, 0x11057da701db6be2}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0xc5f237b2be7e78a0, 0x144186389b61f159, 0x845d315e796360f6,
       0x8b25036043fff07e, 0x75248dba378895c5, 0x3c4db4b2152d79bf,
       0x5df9e6d3a2c0f5e, 0x2a27806abc994d8c},
      {0xd0b1990b79ba3d67, 0x1c7c278bd4972d1c, 0x809414cdaca890a7,
       0x714d764323cebc3, 0x43750544274b8f02, 0x81a1d16562274e2d,
       0xe01a6b9da35af448, 0x331e7c9993b1e077}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_COMMITMENT_PLUS = {
    {{{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
     {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
     {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
     {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_COMMITMENT_MINUS = {
    {{{0xf45e484453d80540, 0xf420792230f16c8, 0x427bb0135cab0b98,
       0x97a9098307909b31, 0x3813c491cf3076dc, 0xb483fd41da561883,
       0x3d2b053f89c813a3, 0x38e9d8f5662ef867},
      {0x53dd3a36cd083b51, 0x43f21025eba75a9a, 0xbe5025da7fc832e3,
       0x32dfb2c5d5f0df2e, 0xac3ac86cb4e98b81, 0x929eec8d7f881f7d,
       0x52eac5ae8f67b043, 0x484b83a60a3fc193}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0x9b529e5fd449ce30, 0x6455c3c8ab0f2e29, 0xdaf88e9ce19b6f20,
       0xf7aadf30c8393c9, 0x7edd2f7fdd7401a7, 0x3565b5cc17ad80c2,
       0x807c9cc1a5e54cb0, 0x2e9a67b8b5f9ba0f},
      {0x3fcc38a1fbdaad55, 0xdea7de4d0edae0a9, 0x23bec824c8d7e09b,
       0xe32ae68fbda186b6, 0xfe8c4ee4ed82dfcb, 0x8d70deb553f9d85c,
       0xc1368fce3843aa49, 0x11057da701db6be2}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0xc5f237b2be7e78a0, 0x144186389b61f159, 0x845d315e796360f6,
       0x8b25036043fff07e, 0x75248dba378895c5, 0x3c4db4b2152d79bf,
       0x5df9e6d3a2c0f5e, 0x2a27806abc994d8c},
      {0xd0b1990b79ba3d67, 0x1c7c278bd4972d1c, 0x809414cdaca890a7,
       0x714d764323cebc3, 0x43750544274b8f02, 0x81a1d16562274e2d,
       0xe01a6b9da35af448, 0x331e7c9993b1e077}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}}};
const ec_basis_t BASIS_CHALLENGE = {
    {{{0xdbe526f582b78d14, 0x67ef201b3d63d961, 0x335cfd122955ccca,
       0x3aa49c0e9ad6be3f, 0x2b7493e8bb9c21b0, 0x65b55ba2ed770924,
       0xf8973fb51e82a450, 0x2e8fd3bfe3f72591},
      {0x90331d5aa6d3f5ca, 0x729134fc0ee36929, 0xff776a0a9b010920,
       0xd186e587500e0bf1, 0x4a5c8950a18d8f16, 0x38efcae06a4c7656,
       0xae1c8e6cd475a30, 0x28f5d508172e0ae7}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0xbd6fd90c9291c29a, 0xed35db109cbdbad5, 0x49ee7cc7823a2c82,
       0x8ac3374a0728417f, 0xffc8d561c2df7e99, 0x943be833b42ade34,
       0xfc7e976948a99479, 0x121c8097a80c4a59},
      {0x372cde0a9a06d836, 0x56a55f7771f0b594, 0xd1d60421b2299677,
       0x595e21e52ea9c54d, 0xb1d64c019a7ae166, 0xc673e730a07dd23a,
       0x3c47bd1009c43814, 0x19a4446f3656c62}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}},
    {{{0x810bc1d1ea3c51b6, 0x8cad8c769b7d93fd, 0x6f92e9171d95401,
       0x679973b0ae040af4, 0x95c7c0e616e14a87, 0x557345f0ebfcc097,
       0x858ac25bd127b099, 0x23ca50122b61cf43},
      {0x35ddab33ea522afc, 0xe3748abafc07d455, 0x8f9255928c28db63,
       0xf4a03b720485dc6a, 0x2e7216193bde6224, 0x2c62820a114616df,
       0x4469ce90819e4356, 0x4a52d273ada41194}},
     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}}};
const ec_curve_t CURVE_E0 = {{{0}}, {{1}}, {{{2}}, {{4}}}};
const ibz_mat_2x2_t ACTION_I = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xf478, 0x3907, 0xb7ee, 0x179b, 0xab5e, 0x3167,
                                 0x748f, 0xa04a, 0x9bad, 0x229f, 0xd0,   0x4609,
                                 0xa61c, 0x5d4a, 0x9ed7, 0x82a6, 0x6efa, 0xe05f,
                                 0x57a,  0xc546, 0xd441, 0x8d80, 0xcfa1, 0x98e4,
                                 0x8bf8, 0xd680, 0xd047, 0x5994, 0x2a9,  0x8838,
                                 0xc6ca, 0x8b95, 0xd584, 0xa6a2, 0x8}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x3907f478, 0x179bb7ee, 0x3167ab5e, 0xa04a748f,
                                 0x229f9bad, 0x460900d0, 0x5d4aa61c, 0x82a69ed7,
                                 0xe05f6efa, 0xc546057a, 0x8d80d441, 0x98e4cfa1,
                                 0xd6808bf8, 0x5994d047, 0x883802a9, 0x8b95c6ca,
                                 0xa6a2d584, 0x8}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x179bb7ee3907f478, 0xa04a748f3167ab5e,
                                 0x460900d0229f9bad, 0x82a69ed75d4aa61c,
                                 0xc546057ae05f6efa, 0x98e4cfa18d80d441,
                                 0x5994d047d6808bf8, 0x8b95c6ca883802a9,
                                 0x8a6a2d584}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xc229, 0xaee7, 0xa371, 0x9a39, 0x5dc8, 0xf73,
                                 0x78f7, 0xcb5b, 0x9524, 0xa4c0, 0xb171, 0xac10,
                                 0x37ff, 0xb2b8, 0xfe03, 0xfe0f, 0xdca5, 0x1296,
                                 0x6ccb, 0xf092, 0xccfb, 0x6b66, 0x7258, 0x2773,
                                 0xa232, 0x5412, 0xa6da, 0x9504, 0x536d, 0x6a44,
                                 0xecc0, 0x4e91, 0xadbc, 0x7d4f, 0x9}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0xaee7c229, 0x9a39a371, 0xf735dc8, 0xcb5b78f7,
                                 0xa4c09524, 0xac10b171, 0xb2b837ff, 0xfe0ffe03,
                                 0x1296dca5, 0xf0926ccb, 0x6b66ccfb, 0x27737258,
                                 0x5412a232, 0x9504a6da, 0x6a44536d, 0x4e91ecc0,
                                 0x7d4fadbc, 0x9}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x9a39a371aee7c229, 0xcb5b78f70f735dc8,
                                 0xac10b171a4c09524, 0xfe0ffe03b2b837ff,
                                 0xf0926ccb1296dca5, 0x277372586b66ccfb,
                                 0x9504a6da5412a232, 0x4e91ecc06a44536d,
                                 0x97d4fadbc}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 34,
          ._mp_d = (mp_limb_t[]){0x47a7, 0xfaa2, 0x6e9b, 0xc740, 0x89fd, 0xe131,
                                 0x1d4a, 0x51b3, 0x3abb, 0xf35b, 0xc809, 0x7780,
                                 0xff12, 0x84a0, 0xfa24, 0x7c2e, 0x3738, 0x1ab9,
                                 0xfa9e, 0x8f06, 0x32cb, 0x9775, 0x18c8, 0x97b9,
                                 0x9ca8, 0xd23c, 0x8172, 0x107f, 0x3de5, 0x1fce,
                                 0xad19, 0x2d85, 0xe89f, 0x4116}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 17,
          ._mp_d = (mp_limb_t[]){0xfaa247a7, 0xc7406e9b, 0xe13189fd, 0x51b31d4a,
                                 0xf35b3abb, 0x7780c809, 0x84a0ff12, 0x7c2efa24,
                                 0x1ab93738, 0x8f06fa9e, 0x977532cb, 0x97b918c8,
                                 0xd23c9ca8, 0x107f8172, 0x1fce3de5, 0x2d85ad19,
                                 0x4116e89f}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0xc7406e9bfaa247a7, 0x51b31d4ae13189fd,
                                 0x7780c809f35b3abb, 0x7c2efa2484a0ff12,
                                 0x8f06fa9e1ab93738, 0x97b918c8977532cb,
                                 0x107f8172d23c9ca8, 0x2d85ad191fce3de5,
                                 0x4116e89f}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xb88,  0xc6f8, 0x4811, 0xe864, 0x54a1, 0xce98,
                                 0x8b70, 0x5fb5, 0x6452, 0xdd60, 0xff2f, 0xb9f6,
                                 0x59e3, 0xa2b5, 0x6128, 0x7d59, 0xa685, 0xe88d,
                                 0xfb28, 0xc3e0, 0x91b8, 0x38ff, 0xc55b, 0xd84d,
                                 0x1cd2, 0x126b, 0xc57c, 0x26f2, 0x4854, 0x1353,
                                 0x86a,  0x961e, 0x1f00, 0x77b0, 0x2}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0xc6f80b88, 0xe8644811, 0xce9854a1, 0x5fb58b70,
                                 0xdd606452, 0xb9f6ff2f, 0xa2b559e3, 0x7d596128,
                                 0xe88da685, 0xc3e0fb28, 0x38ff91b8, 0xd84dc55b,
                                 0x126b1cd2, 0x26f2c57c, 0x13534854, 0x961e086a,
                                 0x77b01f00, 0x2}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0xe8644811c6f80b88, 0x5fb58b70ce9854a1,
                                 0xb9f6ff2fdd606452, 0x7d596128a2b559e3,
                                 0xc3e0fb28e88da685, 0xd84dc55b38ff91b8,
                                 0x26f2c57c126b1cd2, 0x961e086a13534854,
                                 0x277b01f00}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_J = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xe1c0, 0x3e0f, 0x1f73, 0x4601, 0x5f20, 0xb761,
                                 0xd792, 0x84a6, 0xc1a6, 0x2ef0, 0xf751, 0x6006,
                                 0x5801, 0x3d54, 0x45d8, 0xa31,  0xd310, 0x163b,
                                 0xe5c9, 0xc1e,  0x171c, 0x2859, 0x8f3e, 0xd6e,
                                 0x54e4, 0xc6f0, 0x2378, 0x1b14, 0xba41, 0xeb16,
                                 0xc9be, 0x3bfc, 0x37a6, 0xc7b0, 0x5}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x3e0fe1c0, 0x46011f73, 0xb7615f20, 0x84a6d792,
                                 0x2ef0c1a6, 0x6006f751, 0x3d545801, 0xa3145d8,
                                 0x163bd310, 0xc1ee5c9, 0x2859171c, 0xd6e8f3e,
                                 0xc6f054e4, 0x1b142378, 0xeb16ba41, 0x3bfcc9be,
                                 0xc7b037a6, 0x5}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x46011f733e0fe1c0, 0x84a6d792b7615f20,
                                 0x6006f7512ef0c1a6, 0xa3145d83d545801,
                                 0xc1ee5c9163bd310, 0xd6e8f3e2859171c,
                                 0x1b142378c6f054e4, 0x3bfcc9beeb16ba41,
                                 0x5c7b037a6}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0x12f7, 0x4e1,  0x32ad, 0xf04f, 0x90bd, 0x1d98,
                                 0xecc6, 0x8ad,  0xf7c9, 0xbae0, 0xb8aa, 0xce3,
                                 0x2245, 0xd556, 0xae41, 0x13b,  0xfc1a, 0x68d9,
                                 0x4910, 0xeb83, 0x8a45, 0x661e, 0x6550, 0x162f,
                                 0x8256, 0x2fd2, 0x5271, 0x4a95, 0x7106, 0xbdfc,
                                 0xdde6, 0x9576, 0x22f0, 0xb24b, 0x5}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x4e112f7, 0xf04f32ad, 0x1d9890bd, 0x8adecc6,
                                 0xbae0f7c9, 0xce3b8aa, 0xd5562245, 0x13bae41,
                                 0x68d9fc1a, 0xeb834910, 0x661e8a45, 0x162f6550,
                                 0x2fd28256, 0x4a955271, 0xbdfc7106, 0x9576dde6,
                                 0xb24b22f0, 0x5}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0xf04f32ad04e112f7, 0x8adecc61d9890bd,
                                 0xce3b8aabae0f7c9, 0x13bae41d5562245,
                                 0xeb83491068d9fc1a, 0x162f6550661e8a45,
                                 0x4a9552712fd28256, 0x9576dde6bdfc7106,
                                 0x5b24b22f0}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xdec7, 0x56e3, 0x9f2,  0x96d5, 0x66c4, 0x1fed,
                                 0xbe03, 0xe9dc, 0x170f, 0x1424, 0x34dd, 0xabff,
                                 0xc48c, 0x1ae1, 0xf984, 0xae17, 0xe0a,  0xac3,
                                 0xc5f5, 0x36b9, 0xaece, 0xcc9b, 0x8a3c, 0x67ae,
                                 0xb90b, 0x65b,  0xd547, 0x1599, 0xd7,   0xbee3,
                                 0xc05,  0xdb2,  0x6f3f, 0x8fcb, 0x6}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x56e3dec7, 0x96d509f2, 0x1fed66c4, 0xe9dcbe03,
                                 0x1424170f, 0xabff34dd, 0x1ae1c48c, 0xae17f984,
                                 0xac30e0a, 0x36b9c5f5, 0xcc9baece, 0x67ae8a3c,
                                 0x65bb90b, 0x1599d547, 0xbee300d7, 0xdb20c05,
                                 0x8fcb6f3f, 0x6}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x96d509f256e3dec7, 0xe9dcbe031fed66c4,
                                 0xabff34dd1424170f, 0xae17f9841ae1c48c,
                                 0x36b9c5f50ac30e0a, 0x67ae8a3ccc9baece,
                                 0x1599d547065bb90b, 0xdb20c05bee300d7,
                                 0x68fcb6f3f}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0x1e40, 0xc1f0, 0xe08c, 0xb9fe, 0xa0df, 0x489e,
                                 0x286d, 0x7b59, 0x3e59, 0xd10f, 0x8ae,  0x9ff9,
                                 0xa7fe, 0xc2ab, 0xba27, 0xf5ce, 0x426f, 0xb2b1,
                                 0x1ada, 0x7d08, 0x4ede, 0x9e27, 0x5be,  0x63c4,
                                 0x53e7, 0x21fb, 0x724b, 0x6573, 0x90bc, 0xb074,
                                 0x575,  0xe5b7, 0xbcde, 0x56a2, 0x5}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0xc1f01e40, 0xb9fee08c, 0x489ea0df, 0x7b59286d,
                                 0xd10f3e59, 0x9ff908ae, 0xc2aba7fe, 0xf5ceba27,
                                 0xb2b1426f, 0x7d081ada, 0x9e274ede, 0x63c405be,
                                 0x21fb53e7, 0x6573724b, 0xb07490bc, 0xe5b70575,
                                 0x56a2bcde, 0x5}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0xb9fee08cc1f01e40, 0x7b59286d489ea0df,
                                 0x9ff908aed10f3e59, 0xf5ceba27c2aba7fe,
                                 0x7d081adab2b1426f, 0x63c405be9e274ede,
                                 0x6573724b21fb53e7, 0xe5b70575b07490bc,
                                 0x556a2bcde}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_K = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0x4ddf, 0x7bf,  0x82e8, 0x6ffd, 0x1315, 0xcd4a,
                                 0xe0e1, 0xe2d4, 0x2166, 0x836c, 0xb67,  0x348a,
                                 0xb722, 0x56e4, 0x6af1, 0x666,  0xce30, 0xf25e,
                                 0xa2bd, 0x637d, 0x8e9c, 0x3fa5, 0x39f6, 0x50c8,
                                 0x9fa3, 0x325c, 0x4fa6, 0xd6cf, 0x6e3,  0x621d,
                                 0x582c, 0xe487, 0x8683, 0xd2a3, 0x2}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x7bf4ddf, 0x6ffd82e8, 0xcd4a1315, 0xe2d4e0e1,
                                 0x836c2166, 0x348a0b67, 0x56e4b722, 0x6666af1,
                                 0xf25ece30, 0x637da2bd, 0x3fa58e9c, 0x50c839f6,
                                 0x325c9fa3, 0xd6cf4fa6, 0x621d06e3, 0xe487582c,
                                 0xd2a38683, 0x2}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x6ffd82e807bf4ddf, 0xe2d4e0e1cd4a1315,
                                 0x348a0b67836c2166, 0x6666af156e4b722,
                                 0x637da2bdf25ece30, 0x50c839f63fa58e9c,
                                 0xd6cf4fa6325c9fa3, 0xe487582c621d06e3,
                                 0x2d2a38683}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 34,
          ._mp_d = (mp_limb_t[]){0xa808, 0x2576, 0xbc1,  0x19ea, 0xa336, 0xa58,
                                 0x1122, 0xa83d, 0xc497, 0x7085, 0x697e, 0x2a41,
                                 0x2437, 0x727f, 0x211e, 0xcfca, 0xcbf,  0x94ae,
                                 0xc13e, 0x131d, 0x545c, 0x51a6, 0x1222, 0xf822,
                                 0x1f05, 0x1361, 0xf518, 0xf600, 0xb1aa, 0x943b,
                                 0x1e0c, 0x243e, 0x1520, 0xbf7c}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 17,
          ._mp_d = (mp_limb_t[]){0x2576a808, 0x19ea0bc1, 0xa58a336, 0xa83d1122,
                                 0x7085c497, 0x2a41697e, 0x727f2437, 0xcfca211e,
                                 0x94ae0cbf, 0x131dc13e, 0x51a6545c, 0xf8221222,
                                 0x13611f05, 0xf600f518, 0x943bb1aa, 0x243e1e0c,
                                 0xbf7c1520}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x19ea0bc12576a808, 0xa83d11220a58a336,
                                 0x2a41697e7085c497, 0xcfca211e727f2437,
                                 0x131dc13e94ae0cbf, 0xf822122251a6545c,
                                 0xf600f51813611f05, 0x243e1e0c943bb1aa,
                                 0xbf7c1520}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0x6af8, 0xede1, 0x52e4, 0x16b4, 0x481f, 0x7bc0,
                                 0xdb3e, 0xda8a, 0x9a7d, 0x3929, 0xca67, 0x30ef,
                                 0x863e, 0x8220, 0xbb4b, 0xba77, 0x2fc9, 0x30fd,
                                 0xbb98, 0xd448, 0x1f3c, 0xc199, 0x7ed2, 0x2c93,
                                 0x48ba, 0xbca,  0x6844, 0x55ba, 0x4381, 0x6a9e,
                                 0xf4ae, 0xcba3, 0xc012, 0xb60a, 0x6}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0xede16af8, 0x16b452e4, 0x7bc0481f, 0xda8adb3e,
                                 0x39299a7d, 0x30efca67, 0x8220863e, 0xba77bb4b,
                                 0x30fd2fc9, 0xd448bb98, 0xc1991f3c, 0x2c937ed2,
                                 0xbca48ba, 0x55ba6844, 0x6a9e4381, 0xcba3f4ae,
                                 0xb60ac012, 0x6}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x16b452e4ede16af8, 0xda8adb3e7bc0481f,
                                 0x30efca6739299a7d, 0xba77bb4b8220863e,
                                 0xd448bb9830fd2fc9, 0x2c937ed2c1991f3c,
                                 0x55ba68440bca48ba, 0xcba3f4ae6a9e4381,
                                 0x6b60ac012}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xb221, 0xf840, 0x7d17, 0x9002, 0xecea, 0x32b5,
                                 0x1f1e, 0x1d2b, 0xde99, 0x7c93, 0xf498, 0xcb75,
                                 0x48dd, 0xa91b, 0x950e, 0xf999, 0x474f, 0xd68e,
                                 0x5de5, 0x25a9, 0xd75e, 0x86da, 0x5b06, 0x206a,
                                 0x928,  0xb68f, 0x461d, 0xa9b8, 0x4419, 0x396e,
                                 0x7708, 0x3d2c, 0x6e01, 0x4baf, 0x8}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0xf840b221, 0x90027d17, 0x32b5ecea, 0x1d2b1f1e,
                                 0x7c93de99, 0xcb75f498, 0xa91b48dd, 0xf999950e,
                                 0xd68e474f, 0x25a95de5, 0x86dad75e, 0x206a5b06,
                                 0xb68f0928, 0xa9b8461d, 0x396e4419, 0x3d2c7708,
                                 0x4baf6e01, 0x8}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x90027d17f840b221, 0x1d2b1f1e32b5ecea,
                                 0xcb75f4987c93de99, 0xf999950ea91b48dd,
                                 0x25a95de5d68e474f, 0x206a5b0686dad75e,
                                 0xa9b8461db68f0928, 0x3d2c7708396e4419,
                                 0x84baf6e01}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_GEN2 = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xf478, 0x3907, 0xb7ee, 0x179b, 0xab5e, 0x3167,
                                 0x748f, 0xa04a, 0x9bad, 0x229f, 0xd0,   0x4609,
                                 0xa61c, 0x5d4a, 0x9ed7, 0x82a6, 0x6efa, 0xe05f,
                                 0x57a,  0xc546, 0xd441, 0x8d80, 0xcfa1, 0x98e4,
                                 0x8bf8, 0xd680, 0xd047, 0x5994, 0x2a9,  0x8838,
                                 0xc6ca, 0x8b95, 0xd584, 0xa6a2, 0x8}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x3907f478, 0x179bb7ee, 0x3167ab5e, 0xa04a748f,
                                 0x229f9bad, 0x460900d0, 0x5d4aa61c, 0x82a69ed7,
                                 0xe05f6efa, 0xc546057a, 0x8d80d441, 0x98e4cfa1,
                                 0xd6808bf8, 0x5994d047, 0x883802a9, 0x8b95c6ca,
                                 0xa6a2d584, 0x8}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x179bb7ee3907f478, 0xa04a748f3167ab5e,
                                 0x460900d0229f9bad, 0x82a69ed75d4aa61c,
                                 0xc546057ae05f6efa, 0x98e4cfa18d80d441,
                                 0x5994d047d6808bf8, 0x8b95c6ca883802a9,
                                 0x8a6a2d584}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xc229, 0xaee7, 0xa371, 0x9a39, 0x5dc8, 0xf73,
                                 0x78f7, 0xcb5b, 0x9524, 0xa4c0, 0xb171, 0xac10,
                                 0x37ff, 0xb2b8, 0xfe03, 0xfe0f, 0xdca5, 0x1296,
                                 0x6ccb, 0xf092, 0xccfb, 0x6b66, 0x7258, 0x2773,
                                 0xa232, 0x5412, 0xa6da, 0x9504, 0x536d, 0x6a44,
                                 0xecc0, 0x4e91, 0xadbc, 0x7d4f, 0x9}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0xaee7c229, 0x9a39a371, 0xf735dc8, 0xcb5b78f7,
                                 0xa4c09524, 0xac10b171, 0xb2b837ff, 0xfe0ffe03,
                                 0x1296dca5, 0xf0926ccb, 0x6b66ccfb, 0x27737258,
                                 0x5412a232, 0x9504a6da, 0x6a44536d, 0x4e91ecc0,
                                 0x7d4fadbc, 0x9}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x9a39a371aee7c229, 0xcb5b78f70f735dc8,
                                 0xac10b171a4c09524, 0xfe0ffe03b2b837ff,
                                 0xf0926ccb1296dca5, 0x277372586b66ccfb,
                                 0x9504a6da5412a232, 0x4e91ecc06a44536d,
                                 0x97d4fadbc}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 34,
          ._mp_d = (mp_limb_t[]){0x47a7, 0xfaa2, 0x6e9b, 0xc740, 0x89fd, 0xe131,
                                 0x1d4a, 0x51b3, 0x3abb, 0xf35b, 0xc809, 0x7780,
                                 0xff12, 0x84a0, 0xfa24, 0x7c2e, 0x3738, 0x1ab9,
                                 0xfa9e, 0x8f06, 0x32cb, 0x9775, 0x18c8, 0x97b9,
                                 0x9ca8, 0xd23c, 0x8172, 0x107f, 0x3de5, 0x1fce,
                                 0xad19, 0x2d85, 0xe89f, 0x4116}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 17,
          ._mp_d = (mp_limb_t[]){0xfaa247a7, 0xc7406e9b, 0xe13189fd, 0x51b31d4a,
                                 0xf35b3abb, 0x7780c809, 0x84a0ff12, 0x7c2efa24,
                                 0x1ab93738, 0x8f06fa9e, 0x977532cb, 0x97b918c8,
                                 0xd23c9ca8, 0x107f8172, 0x1fce3de5, 0x2d85ad19,
                                 0x4116e89f}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0xc7406e9bfaa247a7, 0x51b31d4ae13189fd,
                                 0x7780c809f35b3abb, 0x7c2efa2484a0ff12,
                                 0x8f06fa9e1ab93738, 0x97b918c8977532cb,
                                 0x107f8172d23c9ca8, 0x2d85ad191fce3de5,
                                 0x4116e89f}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xb88,  0xc6f8, 0x4811, 0xe864, 0x54a1, 0xce98,
                                 0x8b70, 0x5fb5, 0x6452, 0xdd60, 0xff2f, 0xb9f6,
                                 0x59e3, 0xa2b5, 0x6128, 0x7d59, 0xa685, 0xe88d,
                                 0xfb28, 0xc3e0, 0x91b8, 0x38ff, 0xc55b, 0xd84d,
                                 0x1cd2, 0x126b, 0xc57c, 0x26f2, 0x4854, 0x1353,
                                 0x86a,  0x961e, 0x1f00, 0x77b0, 0x2}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0xc6f80b88, 0xe8644811, 0xce9854a1, 0x5fb58b70,
                                 0xdd606452, 0xb9f6ff2f, 0xa2b559e3, 0x7d596128,
                                 0xe88da685, 0xc3e0fb28, 0x38ff91b8, 0xd84dc55b,
                                 0x126b1cd2, 0x26f2c57c, 0x13534854, 0x961e086a,
                                 0x77b01f00, 0x2}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0xe8644811c6f80b88, 0x5fb58b70ce9854a1,
                                 0xb9f6ff2fdd606452, 0x7d596128a2b559e3,
                                 0xc3e0fb28e88da685, 0xd84dc55b38ff91b8,
                                 0x26f2c57c126b1cd2, 0x961e086a13534854,
                                 0x277b01f00}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_GEN3 = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xeb1c, 0xbb8b, 0x6bb0, 0x2ece, 0x853f, 0xf464,
                                 0xa610, 0x1278, 0x2eaa, 0xa8c8, 0xfc10, 0xd307,
                                 0x7f0e, 0xcd4f, 0xf257, 0x466b, 0x1645, 0x16d7,
                                 0xf550, 0xa41e, 0xc2b1, 0x77ac, 0x64f1, 0x9a90,
                                 0x9c08, 0x5a42, 0xaefe, 0x7a10, 0xb8f6, 0x6be1,
                                 0x60aa, 0xd2ef, 0xc52,  0xa800, 0x1}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0xbb8beb1c, 0x2ece6bb0, 0xf464853f, 0x1278a610,
                                 0xa8c82eaa, 0xd307fc10, 0xcd4f7f0e, 0x466bf257,
                                 0x16d71645, 0xa41ef550, 0x77acc2b1, 0x9a9064f1,
                                 0x5a429c08, 0x7a10aefe, 0x6be1b8f6, 0xd2ef60aa,
                                 0xa8000c52, 0x1}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x2ece6bb0bb8beb1c, 0x1278a610f464853f,
                                 0xd307fc10a8c82eaa, 0x466bf257cd4f7f0e,
                                 0xa41ef55016d71645, 0x9a9064f177acc2b1,
                                 0x7a10aefe5a429c08, 0xd2ef60aa6be1b8f6,
                                 0x1a8000c52}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0x6a90, 0x59e4, 0x6b0f, 0x4544, 0xf743, 0x9685,
                                 0xb2de, 0xea04, 0xc676, 0x2fd0, 0x350e, 0x5c7a,
                                 0x2d22, 0xc407, 0xd622, 0xffa5, 0x6c5f, 0xbdb8,
                                 0xdaed, 0xee0a, 0xaba0, 0x68c2, 0x6bd4, 0x1ed1,
                                 0x9244, 0xc1f2, 0xfca5, 0xefcc, 0x6239, 0x9420,
                                 0x6553, 0x7204, 0x6856, 0x97cd, 0x7}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x59e46a90, 0x45446b0f, 0x9685f743, 0xea04b2de,
                                 0x2fd0c676, 0x5c7a350e, 0xc4072d22, 0xffa5d622,
                                 0xbdb86c5f, 0xee0adaed, 0x68c2aba0, 0x1ed16bd4,
                                 0xc1f29244, 0xefccfca5, 0x94206239, 0x72046553,
                                 0x97cd6856, 0x7}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x45446b0f59e46a90, 0xea04b2de9685f743,
                                 0x5c7a350e2fd0c676, 0xffa5d622c4072d22,
                                 0xee0adaedbdb86c5f, 0x1ed16bd468c2aba0,
                                 0xefccfca5c1f29244, 0x7204655394206239,
                                 0x797cd6856}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0x1337, 0x28c3, 0xbc47, 0x2f0a, 0x7861, 0x8f,
                                 0xeda7, 0x9dc7, 0xa8e5, 0x83bf, 0xfe73, 0x91bf,
                                 0x61cf, 0x4fc1, 0x79d4, 0x9523, 0x22a1, 0x92be,
                                 0x6049, 0xe2e0, 0x70cc, 0xb208, 0xd182, 0xffb3,
                                 0x2ad9, 0xec4c, 0xab5c, 0x130c, 0x9f5e, 0x6f58,
                                 0xdc8f, 0x1d9b, 0x2bef, 0x6871, 0x3}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x28c31337, 0x2f0abc47, 0x8f7861, 0x9dc7eda7,
                                 0x83bfa8e5, 0x91bffe73, 0x4fc161cf, 0x952379d4,
                                 0x92be22a1, 0xe2e06049, 0xb20870cc, 0xffb3d182,
                                 0xec4c2ad9, 0x130cab5c, 0x6f589f5e, 0x1d9bdc8f,
                                 0x68712bef, 0x3}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x2f0abc4728c31337, 0x9dc7eda7008f7861,
                                 0x91bffe7383bfa8e5, 0x952379d44fc161cf,
                                 0xe2e0604992be22a1, 0xffb3d182b20870cc,
                                 0x130cab5cec4c2ad9, 0x1d9bdc8f6f589f5e,
                                 0x368712bef}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0x14e4, 0x4474, 0x944f, 0xd131, 0x7ac0, 0xb9b,
                                 0x59ef, 0xed87, 0xd155, 0x5737, 0x3ef,  0x2cf8,
                                 0x80f1, 0x32b0, 0xda8,  0xb994, 0xff3a, 0xb215,
                                 0xb53,  0xe508, 0xa348, 0x4ed3, 0x300b, 0xd6a2,
                                 0xcc2,  0x8ea9, 0xe6c5, 0x676,  0x9207, 0x2fa9,
                                 0x6e8a, 0x4ec4, 0xe832, 0x7652, 0x9}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x447414e4, 0xd131944f, 0xb9b7ac0, 0xed8759ef,
                                 0x5737d155, 0x2cf803ef, 0x32b080f1, 0xb9940da8,
                                 0xb215ff3a, 0xe5080b53, 0x4ed3a348, 0xd6a2300b,
                                 0x8ea90cc2, 0x676e6c5, 0x2fa99207, 0x4ec46e8a,
                                 0x7652e832, 0x9}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0xd131944f447414e4, 0xed8759ef0b9b7ac0,
                                 0x2cf803ef5737d155, 0xb9940da832b080f1,
                                 0xe5080b53b215ff3a, 0xd6a2300b4ed3a348,
                                 0x676e6c58ea90cc2, 0x4ec46e8a2fa99207,
                                 0x97652e832}}}
#endif
    }};
const ibz_mat_2x2_t ACTION_GEN4 = {
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xa6f0, 0x3df,  0xc174, 0xb7fe, 0x98a,  0xe6a5,
                                 0x7070, 0x716a, 0x10b3, 0xc1b6, 0x5b3,  0x1a45,
                                 0x5b91, 0xab72, 0x3578, 0x333,  0xf1d8, 0xdda5,
                                 0x51b0, 0x7652, 0xfa4b, 0x8312, 0x6779, 0x60fd,
                                 0x2437, 0xda4,  0x72b5, 0xabab, 0x28f0, 0x7ed4,
                                 0x93b0, 0x831d, 0x3d84, 0xf87b, 0x6}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x3dfa6f0, 0xb7fec174, 0xe6a5098a, 0x716a7070,
                                 0xc1b610b3, 0x1a4505b3, 0xab725b91, 0x3333578,
                                 0xdda5f1d8, 0x765251b0, 0x8312fa4b, 0x60fd6779,
                                 0xda42437, 0xabab72b5, 0x7ed428f0, 0x831d93b0,
                                 0xf87b3d84, 0x6}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0xb7fec17403dfa6f0, 0x716a7070e6a5098a,
                                 0x1a4505b3c1b610b3, 0x3333578ab725b91,
                                 0x765251b0dda5f1d8, 0x60fd67798312fa4b,
                                 0xabab72b50da42437, 0x831d93b07ed428f0,
                                 0x6f87b3d84}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0x5404, 0x92bb, 0x5e0,  0xcf5,  0x519b, 0x52c,
                                 0x8891, 0xd41e, 0xe24b, 0x3842, 0xb4bf, 0x9520,
                                 0x921b, 0x393f, 0x108f, 0xe7e5, 0x911f, 0x2ecd,
                                 0x60f1, 0x4e22, 0x5d2b, 0x8c13, 0x538f, 0xb4aa,
                                 0x63e8, 0xfe26, 0x456d, 0x3b44, 0x7e54, 0x97e3,
                                 0xf6a0, 0xa2f8, 0x84d2, 0xeee7, 0x5}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x92bb5404, 0xcf505e0, 0x52c519b, 0xd41e8891,
                                 0x3842e24b, 0x9520b4bf, 0x393f921b, 0xe7e5108f,
                                 0x2ecd911f, 0x4e2260f1, 0x8c135d2b, 0xb4aa538f,
                                 0xfe2663e8, 0x3b44456d, 0x97e37e54, 0xa2f8f6a0,
                                 0xeee784d2, 0x5}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0xcf505e092bb5404, 0xd41e8891052c519b,
                                 0x9520b4bf3842e24b, 0xe7e5108f393f921b,
                                 0x4e2260f12ecd911f, 0xb4aa538f8c135d2b,
                                 0x3b44456dfe2663e8, 0xa2f8f6a097e37e54,
                                 0x5eee784d2}}}
#endif
    },
    {
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0xb57c, 0x76f0, 0x2972, 0x8b5a, 0x240f, 0x3de0,
                                 0x6d9f, 0xed45, 0xcd3e, 0x9c94, 0xe533, 0x1877,
                                 0x431f, 0xc110, 0xdda5, 0xdd3b, 0x97e4, 0x187e,
                                 0x5dcc, 0x6a24, 0x8f9e, 0x60cc, 0xbf69, 0x1649,
                                 0x245d, 0x5e5,  0x3422, 0xaadd, 0x21c0, 0x354f,
                                 0xfa57, 0x65d1, 0x6009, 0x5b05, 0x3}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0x76f0b57c, 0x8b5a2972, 0x3de0240f, 0xed456d9f,
                                 0x9c94cd3e, 0x1877e533, 0xc110431f, 0xdd3bdda5,
                                 0x187e97e4, 0x6a245dcc, 0x60cc8f9e, 0x1649bf69,
                                 0x5e5245d, 0xaadd3422, 0x354f21c0, 0x65d1fa57,
                                 0x5b056009, 0x3}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x8b5a297276f0b57c, 0xed456d9f3de0240f,
                                 0x1877e5339c94cd3e, 0xdd3bdda5c110431f,
                                 0x6a245dcc187e97e4, 0x1649bf6960cc8f9e,
                                 0xaadd342205e5245d, 0x65d1fa57354f21c0,
                                 0x35b056009}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 35,
          ._mp_d = (mp_limb_t[]){0x5911, 0xfc20, 0x3e8b, 0x4801, 0xf675, 0x195a,
                                 0x8f8f, 0x8e95, 0xef4c, 0x3e49, 0xfa4c, 0xe5ba,
                                 0xa46e, 0x548d, 0xca87, 0xfccc, 0x23a7, 0xeb47,
                                 0xaef2, 0x12d4, 0x6baf, 0x436d, 0x2d83, 0x1035,
                                 0x8494, 0xdb47, 0x230e, 0xd4dc, 0x220c, 0x1cb7,
                                 0x3b84, 0x9e96, 0xb700, 0x25d7, 0x4}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 18,
          ._mp_d = (mp_limb_t[]){0xfc205911, 0x48013e8b, 0x195af675, 0x8e958f8f,
                                 0x3e49ef4c, 0xe5bafa4c, 0x548da46e, 0xfcccca87,
                                 0xeb4723a7, 0x12d4aef2, 0x436d6baf, 0x10352d83,
                                 0xdb478494, 0xd4dc230e, 0x1cb7220c, 0x9e963b84,
                                 0x25d7b700, 0x4}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0,
          ._mp_size = 9,
          ._mp_d = (mp_limb_t[]){0x48013e8bfc205911, 0x8e958f8f195af675,
                                 0xe5bafa4c3e49ef4c, 0xfcccca87548da46e,
                                 0x12d4aef2eb4723a7, 0x10352d83436d6baf,
                                 0xd4dc230edb478494, 0x9e963b841cb7220c,
                                 0x425d7b700}}}
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
        {{._mp_alloc = 0,
          ._mp_size = 3,
          ._mp_d = (mp_limb_t[]){0xaa3e, 0x795f, 0xa}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 2,
          ._mp_d = (mp_limb_t[]){0x795faa3e, 0xa}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0xa795faa3e}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 3,
          ._mp_d = (mp_limb_t[]){0xa677, 0xf6b, 0x8}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 2,
          ._mp_d = (mp_limb_t[]){0xf6ba677, 0x8}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x80f6ba677}}}
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
        {{._mp_alloc = 0,
          ._mp_size = 3,
          ._mp_d = (mp_limb_t[]){0x7e89, 0x6803, 0x3}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 2,
          ._mp_d = (mp_limb_t[]){0x68037e89, 0x3}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x368037e89}}}
#endif
        ,
#if 0
#elif GMP_LIMB_BITS == 16
        {{._mp_alloc = 0,
          ._mp_size = 3,
          ._mp_d = (mp_limb_t[]){0x1d54, 0xb1ee, 0x4}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 2,
          ._mp_d = (mp_limb_t[]){0xb1ee1d54, 0x4}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x4b1ee1d54}}}
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
        {{._mp_alloc = 0,
          ._mp_size = 3,
          ._mp_d = (mp_limb_t[]){0xb056, 0xf3ab, 0x20}}}
#elif GMP_LIMB_BITS == 32
        {{._mp_alloc = 0,
          ._mp_size = 2,
          ._mp_d = (mp_limb_t[]){0xf3abb056, 0x20}}}
#elif GMP_LIMB_BITS == 64
        {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]){0x20f3abb056}}}
#endif
    }};
