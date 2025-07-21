#include "secure_boot.h"
#include "crypto.h"
int secure_boot_verify(const uint8_t* kernel, int ksize, const uint8_t* sig, const uint8_t* pubkey) {
    return verify_signature(kernel, ksize, sig, pubkey);
} 