#include "hmac.h"
#include "crypto.h"
#include <stdint.h>
#include <string.h>
void hmac_sha256(const uint8_t* key, int keylen, const uint8_t* data, int datalen, uint8_t* out) {
    uint8_t k_ipad[64], k_opad[64], tk[32];
    memset(k_ipad, 0x36, 64); memset(k_opad, 0x5c, 64);
    if (keylen > 64) { sha256_hash(key, keylen, tk); key = tk; keylen = 32; }
    for (int i = 0; i < keylen; ++i) { k_ipad[i] ^= key[i]; k_opad[i] ^= key[i]; }
    uint8_t inner[32];
    uint8_t buf[64+1024];
    memcpy(buf, k_ipad, 64); memcpy(buf+64, data, datalen);
    sha256_hash(buf, 64+datalen, inner);
    memcpy(buf, k_opad, 64); memcpy(buf+64, inner, 32);
    sha256_hash(buf, 64+32, out);
} 