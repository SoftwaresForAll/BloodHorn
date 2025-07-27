#include "entropy.h"
#include "compat.h"
#include <stdint.h>
#include <stddef.h>
#include "crypto.h"
static int rdrand(uint32_t* out) {
    int ok;
    asm volatile ("rdrand %0; setc %1" : "=r"(*out), "=qm"(ok));
    return ok;
}
static int rdseed(uint32_t* out) {
    int ok;
    asm volatile ("rdseed %0; setc %1" : "=r"(*out), "=qm"(ok));
    return ok;
}
static uint32_t uefi_random(void) {
    uint32_t v = 0;
    if (*(volatile uint32_t*)0x80000000) v = *(volatile uint32_t*)0x80000000;
    return v;
}
static uint32_t timing_jitter(void) {
    volatile uint32_t t = 0;
    for (int i = 0; i < 1000; ++i) t += i * (uintptr_t)&t;
    return t ^ (uintptr_t)&t;
}
uint32_t entropy_get(void) {
    uint8_t pool[32];
    uint32_t v = 0, ok = 0;
    if (rdrand(&v)) ok = 1;
    else if (rdseed(&v)) ok = 1;
    else v = 0;
    *(uint32_t*)pool = v;
    *(uint32_t*)(pool+4) = uefi_random();
    *(uint32_t*)(pool+8) = timing_jitter();
    uint32_t t;
    asm volatile ("rdtsc" : "=a"(t));
    *(uint32_t*)(pool+12) = t;
    for (int i = 16; i < 32; i += 4) *(uint32_t*)(pool+i) = (uintptr_t)&pool[i];
    uint8_t hash[32];
    sha256_hash(pool, 32, hash);
    return *(uint32_t*)hash;
} 