#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "crypto.h"

static uint32_t sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32_t sha256_h[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

static uint32_t rotr(uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}

static uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

static uint32_t sigma0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

static uint32_t sigma1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

static uint32_t gamma0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

static uint32_t gamma1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

void sha256_hash(const uint8_t* data, uint32_t len, uint8_t* hash) {
    uint32_t h[8];
    memcpy(h, sha256_h, sizeof(h));
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h_val;
    uint32_t temp1, temp2;
    uint64_t bitlen = len * 8;
    uint32_t blocks = (len + 64) / 64;
    for (uint32_t i = 0; i < blocks; i++) {
        for (int j = 0; j < 16; j++) {
            w[j] = (data[i*64 + j*4] << 24) | (data[i*64 + j*4 + 1] << 16) |
                   (data[i*64 + j*4 + 2] << 8) | data[i*64 + j*4 + 3];
        }
        for (int j = 16; j < 64; j++) {
            w[j] = gamma1(w[j-2]) + w[j-7] + gamma0(w[j-15]) + w[j-16];
        }
        a = h[0]; b = h[1]; c = h[2]; d = h[3];
        e = h[4]; f = h[5]; g = h[6]; h_val = h[7];
        for (int j = 0; j < 64; j++) {
            temp1 = h_val + sigma1(e) + ch(e, f, g) + sha256_k[j] + w[j];
            temp2 = sigma0(a) + maj(a, b, c);
            h_val = g; g = f; f = e; e = d + temp1;
            d = c; c = b; b = a; a = temp1 + temp2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += h_val;
    }
    for (int i = 0; i < 8; i++) {
        hash[i*4] = (h[i] >> 24) & 0xFF;
        hash[i*4 + 1] = (h[i] >> 16) & 0xFF;
        hash[i*4 + 2] = (h[i] >> 8) & 0xFF;
        hash[i*4 + 3] = h[i] & 0xFF;
    }
}

static int mod_exp(const uint8_t* base, const uint8_t* exp, int exp_len, const uint8_t* mod, int mod_len, uint8_t* out, int out_len) {
    uint8_t result[512] = {0};
    result[out_len - 1] = 1;
    for (int i = 0; i < exp_len * 8; i++) {
        int bit = (exp[i / 8] >> (7 - (i % 8))) & 1;
        for (int j = 0; j < out_len; j++) {
            result[j] = (result[j] * result[j]) % mod[j];
        }
        if (bit) {
            for (int j = 0; j < out_len; j++) {
                result[j] = (result[j] * base[j]) % mod[j];
            }
        }
    }
    memcpy(out, result, out_len);
    return 0;
}

int verify_signature(const uint8_t* data, uint32_t len, const uint8_t* signature, const uint8_t* public_key) {
    uint8_t hash[32];
    sha256_hash(data, len, hash);
    int mod_len = 256;
    uint8_t decrypted[256];
    mod_exp(signature, public_key + 4, 256, public_key + 260, 256, decrypted, 256);
    if (decrypted[0] != 0x00 || decrypted[1] != 0x01) return 0;
    int i = 2;
    while (i < 256 && decrypted[i] == 0xFF) i++;
    if (decrypted[i++] != 0x00) return 0;
    static const uint8_t sha256_prefix[] = {
        0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0x04,0x20
    };
    if (memcmp(decrypted + i, sha256_prefix, sizeof(sha256_prefix)) != 0) return 0;
    i += sizeof(sha256_prefix);
    if (memcmp(decrypted + i, hash, 32) != 0) return 0;
    return 1;
} 