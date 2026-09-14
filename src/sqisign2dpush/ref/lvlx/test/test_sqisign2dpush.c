#include <sqisign2dpush.h>
#include <rng.h>

#include <stdio.h>
#include <string.h>

#include "test_sqisign2dpush.h"

#define CHECK(condition)                                                   \
  do {                                                                     \
    if (!(condition)) {                                                    \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition);    \
      ok = 0;                                                              \
      goto cleanup;                                                        \
    }                                                                      \
  } while (0)

static int test_sqisign_roundtrip(void)
{
  int ok = 1;
  public_key_t pk, decoded_pk;
  secret_key_t sk, decoded_sk;
  signature_t sig, decoded_sig;
  unsigned char message[32] = {0};
  unsigned char pk_bytes[PUBLICKEY_BYTES];
  unsigned char decoded_pk_bytes[PUBLICKEY_BYTES];
  unsigned char sk_bytes[SECRETKEY_BYTES];
  unsigned char decoded_sk_bytes[SECRETKEY_BYTES];
  unsigned char sig_bytes[SIGNATURE_BYTES];
  unsigned char decoded_sig_bytes[SIGNATURE_BYTES];

  secret_key_init(&sk);
  secret_key_init(&decoded_sk);
  secret_sig_init(&sig);
  secret_sig_init(&decoded_sig);

  protocols_keygen(&pk, &sk, NULL);

  CHECK(public_key_to_bytes(pk_bytes, sizeof(pk_bytes), &pk));
  CHECK(public_key_from_bytes(&decoded_pk, pk_bytes, sizeof(pk_bytes)));
  CHECK(public_key_to_bytes(decoded_pk_bytes, sizeof(decoded_pk_bytes),
                            &decoded_pk));
  CHECK(!memcmp(pk_bytes, decoded_pk_bytes, sizeof(pk_bytes)));

  CHECK(secret_key_to_bytes(sk_bytes, sizeof(sk_bytes), &sk, &pk));
  CHECK(secret_key_from_bytes(&decoded_sk, &decoded_pk, sk_bytes,
                              sizeof(sk_bytes)));
  CHECK(secret_key_to_bytes(decoded_sk_bytes, sizeof(decoded_sk_bytes),
                            &decoded_sk, &decoded_pk));
  CHECK(!memcmp(sk_bytes, decoded_sk_bytes, sizeof(sk_bytes)));

  CHECK(!protocols_sign(&sig, &decoded_pk, &decoded_sk, message,
                        sizeof(message), 0, NULL));
  CHECK(signature_to_bytes(sig_bytes, sizeof(sig_bytes), &sig));
  CHECK(signature_from_bytes(&decoded_sig, sig_bytes, sizeof(sig_bytes)));
  CHECK(signature_to_bytes(decoded_sig_bytes, sizeof(decoded_sig_bytes),
                           &decoded_sig));
  CHECK(!memcmp(sig_bytes, decoded_sig_bytes, sizeof(sig_bytes)));

  CHECK(protocols_verify(&decoded_sig, &decoded_pk, message,
                         sizeof(message), NULL));

cleanup:
  secret_key_finalize(&sk);
  secret_key_finalize(&decoded_sk);
  secret_sig_finalize(&sig);
  secret_sig_finalize(&decoded_sig);
  return ok;
}

int main(void)
{
  randombytes_init((unsigned char *)"some", (unsigned char *)"string", 128);

  printf("Running SQIsign2DPush keygen/sign/verify round-trip test\n");
  for (int run = 0; run < 10; ++run) {
    if (!test_sqisign_roundtrip()) {
      fprintf(stderr, "Round-trip test failed on run %d\n", run);
      return 1;
    }
  }

  printf("All keygen/sign/verify round-trip tests passed\n");
  return 0;
}