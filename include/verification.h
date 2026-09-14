#ifndef SQISIGN_VERIFICATION_H
#define SQISIGN_VERIFICATION_H

#include <sqisign2dpush.h>
#include <sqisign_namespace.h>

#undef public_key_to_bytes
#define public_key_to_bytes_2(out, pk) SQISIGN_NAMESPACE(public_key_to_bytes)((out), PUBLICKEY_BYTES, (pk))
#define public_key_to_bytes_3(out, sz, pk) SQISIGN_NAMESPACE(public_key_to_bytes)((out), (sz), (pk))
#define GET_PK_TO_BYTES(_1, _2, _3, NAME, ...) NAME
#define public_key_to_bytes(...) GET_PK_TO_BYTES(__VA_ARGS__, public_key_to_bytes_3, public_key_to_bytes_2)(__VA_ARGS__)

#undef public_key_from_bytes
#define public_key_from_bytes_2(pk, in) SQISIGN_NAMESPACE(public_key_from_bytes)((pk), (in), PUBLICKEY_BYTES)
#define public_key_from_bytes_3(pk, in, sz) SQISIGN_NAMESPACE(public_key_from_bytes)((pk), (in), (sz))
#define GET_PK_FROM_BYTES(_1, _2, _3, NAME, ...) NAME
#define public_key_from_bytes(...) GET_PK_FROM_BYTES(__VA_ARGS__, public_key_from_bytes_3, public_key_from_bytes_2)(__VA_ARGS__)

#undef signature_to_bytes
#define signature_to_bytes_2(out, sig) SQISIGN_NAMESPACE(signature_to_bytes)((out), SIGNATURE_BYTES, (sig))
#define signature_to_bytes_3(out, sz, sig) SQISIGN_NAMESPACE(signature_to_bytes)((out), (sz), (sig))
#define GET_SIG_TO_BYTES(_1, _2, _3, NAME, ...) NAME
#define signature_to_bytes(...) GET_SIG_TO_BYTES(__VA_ARGS__, signature_to_bytes_3, signature_to_bytes_2)(__VA_ARGS__)

#undef signature_from_bytes
#define signature_from_bytes_2(sig, in) SQISIGN_NAMESPACE(signature_from_bytes)((sig), (in), SIGNATURE_BYTES)
#define signature_from_bytes_3(sig, in, sz) SQISIGN_NAMESPACE(signature_from_bytes)((sig), (in), (sz))
#define GET_SIG_FROM_BYTES(_1, _2, _3, NAME, ...) NAME
#define signature_from_bytes(...) GET_SIG_FROM_BYTES(__VA_ARGS__, signature_from_bytes_3, signature_from_bytes_2)(__VA_ARGS__)

#undef protocols_verify
#define protocols_verify_4(sig, pk, m, mlen) SQISIGN_NAMESPACE(protocols_verify)((sig), (pk), (m), (mlen), NULL)
#define protocols_verify_5(sig, pk, m, mlen, tm) SQISIGN_NAMESPACE(protocols_verify)((sig), (pk), (m), (mlen), (tm))
#define GET_PROTO_VERIFY(_1, _2, _3, _4, _5, NAME, ...) NAME
#define protocols_verify(...) GET_PROTO_VERIFY(__VA_ARGS__, protocols_verify_5, protocols_verify_4)(__VA_ARGS__)

#endif
