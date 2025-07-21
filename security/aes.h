#ifndef BLOODHORN_AES_H
#define BLOODHORN_AES_H
#include <stdint.h>
void aes_encrypt_block(const uint8_t* in, uint8_t* out, const uint8_t* key);
#endif 