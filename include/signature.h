#ifndef SQISIGN_SIGNATURE_H
#define SQISIGN_SIGNATURE_H

#include <verification.h>

#undef protocols_keygen
#define protocols_keygen_2(pk, sk) (SQISIGN_NAMESPACE(protocols_keygen)((pk), (sk), NULL), 1)
#define protocols_keygen_3(pk, sk, tm) (SQISIGN_NAMESPACE(protocols_keygen)((pk), (sk), (tm)), 1)
#define GET_PROTO_KEYGEN(_1, _2, _3, NAME, ...) NAME
#define protocols_keygen(...) GET_PROTO_KEYGEN(__VA_ARGS__, protocols_keygen_3, protocols_keygen_2)(__VA_ARGS__)

#undef protocols_sign
#define protocols_sign_5(sig, pk, sk, m, mlen) (SQISIGN_NAMESPACE(protocols_sign)((sig), (pk), (sk), (m), (mlen), 0, NULL) == 0 ? 1 : 0)
#define protocols_sign_7(sig, pk, sk, m, mlen, vb, tm) (SQISIGN_NAMESPACE(protocols_sign)((sig), (pk), (sk), (m), (mlen), (vb), (tm)) == 0 ? 1 : 0)
#define GET_PROTO_SIGN(_1, _2, _3, _4, _5, _6, _7, NAME, ...) NAME
#define protocols_sign(...) GET_PROTO_SIGN(__VA_ARGS__, protocols_sign_7, _unused6, protocols_sign_5)(__VA_ARGS__)

#undef secret_key_to_bytes
#define secret_key_to_bytes_3(out, sk, pk) SQISIGN_NAMESPACE(secret_key_to_bytes)((out), SECRETKEY_BYTES, (sk), (pk))
#define secret_key_to_bytes_4(out, sz, sk, pk) SQISIGN_NAMESPACE(secret_key_to_bytes)((out), (sz), (sk), (pk))
#define GET_SK_TO_BYTES(_1, _2, _3, _4, NAME, ...) NAME
#define secret_key_to_bytes(...) GET_SK_TO_BYTES(__VA_ARGS__, secret_key_to_bytes_4, secret_key_to_bytes_3)(__VA_ARGS__)

#undef secret_key_from_bytes
#define secret_key_from_bytes_3(sk, pk, in) SQISIGN_NAMESPACE(secret_key_from_bytes)((sk), (pk), (in), SECRETKEY_BYTES)
#define secret_key_from_bytes_4(sk, pk, in, sz) SQISIGN_NAMESPACE(secret_key_from_bytes)((sk), (pk), (in), (sz))
#define GET_SK_FROM_BYTES(_1, _2, _3, _4, NAME, ...) NAME
#define secret_key_from_bytes(...) GET_SK_FROM_BYTES(__VA_ARGS__, secret_key_from_bytes_4, secret_key_from_bytes_3)(__VA_ARGS__)

#endif
