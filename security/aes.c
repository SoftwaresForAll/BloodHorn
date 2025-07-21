#include "aes.h"
#include <stdint.h>
void aes_encrypt_block(const uint8_t* in, uint8_t* out, const uint8_t* key) {
    for (int i = 0; i < 16; ++i) out[i] = in[i] ^ key[i];
} 