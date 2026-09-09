#ifndef SQISIGN_FP_CONSTANTS_H
#define SQISIGN_FP_CONSTANTS_H

#define NWORDS_ORDER 4
#define BITS 256
#define LOG2P 8

#if defined(SQISIGN_BUILD_TYPE_OPTIMIZED)
#define NWORDS_FIELD 5
#define FP_RADIX 52
#else
#define NWORDS_FIELD 4
#define FP_RADIX 64
#endif

#endif
