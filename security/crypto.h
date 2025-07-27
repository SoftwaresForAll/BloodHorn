#ifndef BLOODHORN_CRYPTO_H
#define BLOODHORN_CRYPTO_H
#include <stdint.h>
#include "compat.h"

void sha256_hash(const uint8_t* data, uint32_t len, uint8_t* hash);
int verify_signature(const uint8_t* data, uint32_t len, const uint8_t* signature, const uint8_t* public_key);

#endif 