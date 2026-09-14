#include <hd_splitting_transforms.h>

#define FP2_ZERO 0
#define FP2_ONE 1
#define FP2_I 2
#define FP2_MINUS_ONE 3
#define FP2_MINUS_I 4

const int EVEN_INDEX[10][2] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 0},
                               {1, 2}, {2, 0}, {2, 1}, {3, 0}, {3, 3}};
const int CHI_EVAL[4][4] = {
    {1, 1, 1, 1}, {1, -1, 1, -1}, {1, 1, -1, -1}, {1, -1, -1, 1}};
const fp2_t FP2_CONSTANTS[5] = {
    {
#if 0
#elif RADIX == 16
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 32
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0x0, 0x0, 0x0, 0x0}
#else
        {0x0, 0x0, 0x0, 0x0}
#endif
#endif
        ,
#if 0
#elif RADIX == 16
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 32
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0x0, 0x0, 0x0, 0x0}
#else
        {0x0, 0x0, 0x0, 0x0}
#endif
#endif
    },
    {
#if 0
#elif RADIX == 16
        {0x29,   0x0,    0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,    0x0,
         0x193e, 0x19c1, 0x629, 0xa85, 0x46c, 0x551, 0x47e, 0x485, 0x1a80, 0x56}
#elif RADIX == 32
        {0x52, 0x0, 0x0, 0x0, 0x1c9f0000, 0xa629ce0, 0x1448d8a8, 0x4291f8a,
         0x56d401}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0x2, 0x0, 0xb0267d62780af870, 0x3a501900cfd43406}
#else
        {0x2, 0x0, 0xb0267d62780af870, 0x3a501900cfd43406}
#endif
#endif
        ,
#if 0
#elif RADIX == 16
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 32
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0x0, 0x0, 0x0, 0x0}
#else
        {0x0, 0x0, 0x0, 0x0}
#endif
#endif
    },
    {
#if 0
#elif RADIX == 16
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 32
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0x0, 0x0, 0x0, 0x0}
#else
        {0x0, 0x0, 0x0, 0x0}
#endif
#endif
        ,
#if 0
#elif RADIX == 16
        {0x29,   0x0,    0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,    0x0,
         0x193e, 0x19c1, 0x629, 0xa85, 0x46c, 0x551, 0x47e, 0x485, 0x1a80, 0x56}
#elif RADIX == 32
        {0x52, 0x0, 0x0, 0x0, 0x1c9f0000, 0xa629ce0, 0x1448d8a8, 0x4291f8a,
         0x56d401}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0x2, 0x0, 0xb0267d62780af870, 0x3a501900cfd43406}
#else
        {0x2, 0x0, 0xb0267d62780af870, 0x3a501900cfd43406}
#endif
#endif
    },
    {
#if 0
#elif RADIX == 16
        {0x1fd6, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff,
         0x1fff, 0x1fff, 0x1fff, 0x7b3,  0xe33,  0xec2,  0xbdb,
         0xe33,  0x176e, 0x1397, 0x1777, 0x1b7c, 0x6e}
#elif RADIX == 32
        {0x1fffffad, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0xb9d7fff, 0x1644c51c,
         0x3a9c70a, 0xbc9e332, 0xc03f2}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0xfffffffffffffffd, 0xffffffffffffffff, 0xf7c643ec4bef8b57,
         0x2887da7ec841b1f5}
#else
        {0xfffffffffffffffd, 0xffffffffffffffff, 0xf7c643ec4bef8b57,
         0x2887da7ec841b1f5}
#endif
#endif
        ,
#if 0
#elif RADIX == 16
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 32
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0x0, 0x0, 0x0, 0x0}
#else
        {0x0, 0x0, 0x0, 0x0}
#endif
#endif
    },
    {
#if 0
#elif RADIX == 16
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 32
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0x0, 0x0, 0x0, 0x0}
#else
        {0x0, 0x0, 0x0, 0x0}
#endif
#endif
        ,
#if 0
#elif RADIX == 16
        {0x1fd6, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff,
         0x1fff, 0x1fff, 0x1fff, 0x7b3,  0xe33,  0xec2,  0xbdb,
         0xe33,  0x176e, 0x1397, 0x1777, 0x1b7c, 0x6e}
#elif RADIX == 32
        {0x1fffffad, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0xb9d7fff, 0x1644c51c,
         0x3a9c70a, 0xbc9e332, 0xc03f2}
#elif RADIX == 64
#if defined(SQISIGN_GF_IMPL_BROADWELL)
        {0xfffffffffffffffd, 0xffffffffffffffff, 0xf7c643ec4bef8b57,
         0x2887da7ec841b1f5}
#else
        {0xfffffffffffffffd, 0xffffffffffffffff, 0xf7c643ec4bef8b57,
         0x2887da7ec841b1f5}
#endif
#endif
    }};
const precomp_basis_change_matrix_t SPLITTING_TRANSFORMS[10] = {
    {{{FP2_ONE, FP2_I, FP2_ONE, FP2_I},
      {FP2_ONE, FP2_MINUS_I, FP2_MINUS_ONE, FP2_I},
      {FP2_ONE, FP2_I, FP2_MINUS_ONE, FP2_MINUS_I},
      {FP2_MINUS_ONE, FP2_I, FP2_MINUS_ONE, FP2_I}}},
    {{{FP2_ONE, FP2_ZERO, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ZERO, FP2_ZERO, FP2_ONE},
      {FP2_ZERO, FP2_ZERO, FP2_ONE, FP2_ZERO},
      {FP2_ZERO, FP2_MINUS_ONE, FP2_ZERO, FP2_ZERO}}},
    {{{FP2_ONE, FP2_ZERO, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ONE, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ZERO, FP2_ZERO, FP2_ONE},
      {FP2_ZERO, FP2_ZERO, FP2_MINUS_ONE, FP2_ZERO}}},
    {{{FP2_ONE, FP2_ZERO, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ONE, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ZERO, FP2_ONE, FP2_ZERO},
      {FP2_ZERO, FP2_ZERO, FP2_ZERO, FP2_MINUS_ONE}}},
    {{{FP2_ONE, FP2_ONE, FP2_ONE, FP2_ONE},
      {FP2_ONE, FP2_MINUS_ONE, FP2_MINUS_ONE, FP2_ONE},
      {FP2_ONE, FP2_ONE, FP2_MINUS_ONE, FP2_MINUS_ONE},
      {FP2_MINUS_ONE, FP2_ONE, FP2_MINUS_ONE, FP2_ONE}}},
    {{{FP2_ONE, FP2_ZERO, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ONE, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ZERO, FP2_ZERO, FP2_ONE},
      {FP2_ZERO, FP2_ZERO, FP2_ONE, FP2_ZERO}}},
    {{{FP2_ONE, FP2_ONE, FP2_ONE, FP2_ONE},
      {FP2_ONE, FP2_MINUS_ONE, FP2_ONE, FP2_MINUS_ONE},
      {FP2_ONE, FP2_MINUS_ONE, FP2_MINUS_ONE, FP2_ONE},
      {FP2_MINUS_ONE, FP2_MINUS_ONE, FP2_ONE, FP2_ONE}}},
    {{{FP2_ONE, FP2_ONE, FP2_ONE, FP2_ONE},
      {FP2_ONE, FP2_MINUS_ONE, FP2_ONE, FP2_MINUS_ONE},
      {FP2_ONE, FP2_MINUS_ONE, FP2_MINUS_ONE, FP2_ONE},
      {FP2_ONE, FP2_ONE, FP2_MINUS_ONE, FP2_MINUS_ONE}}},
    {{{FP2_ONE, FP2_ONE, FP2_ONE, FP2_ONE},
      {FP2_ONE, FP2_MINUS_ONE, FP2_ONE, FP2_MINUS_ONE},
      {FP2_ONE, FP2_ONE, FP2_MINUS_ONE, FP2_MINUS_ONE},
      {FP2_MINUS_ONE, FP2_ONE, FP2_ONE, FP2_MINUS_ONE}}},
    {{{FP2_ONE, FP2_ZERO, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ONE, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ZERO, FP2_ONE, FP2_ZERO},
      {FP2_ZERO, FP2_ZERO, FP2_ZERO, FP2_ONE}}}};
const precomp_basis_change_matrix_t NORMALIZATION_TRANSFORMS[6] = {
    {{{FP2_ONE, FP2_ZERO, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ONE, FP2_ZERO, FP2_ZERO},
      {FP2_ZERO, FP2_ZERO, FP2_ONE, FP2_ZERO},
      {FP2_ZERO, FP2_ZERO, FP2_ZERO, FP2_ONE}}},
    {{{FP2_ZERO, FP2_ZERO, FP2_ZERO, FP2_ONE},
      {FP2_ZERO, FP2_ZERO, FP2_ONE, FP2_ZERO},
      {FP2_ZERO, FP2_ONE, FP2_ZERO, FP2_ZERO},
      {FP2_ONE, FP2_ZERO, FP2_ZERO, FP2_ZERO}}},
    {{{FP2_ONE, FP2_ONE, FP2_ONE, FP2_ONE},
      {FP2_ONE, FP2_MINUS_ONE, FP2_ONE, FP2_MINUS_ONE},
      {FP2_ONE, FP2_ONE, FP2_MINUS_ONE, FP2_MINUS_ONE},
      {FP2_ONE, FP2_MINUS_ONE, FP2_MINUS_ONE, FP2_ONE}}},
    {{{FP2_ONE, FP2_MINUS_ONE, FP2_MINUS_ONE, FP2_ONE},
      {FP2_MINUS_ONE, FP2_MINUS_ONE, FP2_ONE, FP2_ONE},
      {FP2_MINUS_ONE, FP2_ONE, FP2_MINUS_ONE, FP2_ONE},
      {FP2_ONE, FP2_ONE, FP2_ONE, FP2_ONE}}},
    {{{FP2_MINUS_ONE, FP2_I, FP2_I, FP2_ONE},
      {FP2_I, FP2_MINUS_ONE, FP2_ONE, FP2_I},
      {FP2_I, FP2_ONE, FP2_MINUS_ONE, FP2_I},
      {FP2_ONE, FP2_I, FP2_I, FP2_MINUS_ONE}}},
    {{{FP2_ONE, FP2_I, FP2_I, FP2_MINUS_ONE},
      {FP2_I, FP2_ONE, FP2_MINUS_ONE, FP2_I},
      {FP2_I, FP2_MINUS_ONE, FP2_ONE, FP2_I},
      {FP2_MINUS_ONE, FP2_I, FP2_I, FP2_ONE}}}};
