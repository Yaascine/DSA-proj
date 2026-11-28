/* prng_ref.c — REFERENCE implementation (Blackman & Vigna, prng.di.unimi.it).
 * Compiled as C and linked into the test binary so the C++ Rng port can be
 * checked byte-for-byte against the published reference outputs.
 * This file is test-only; the simulation uses core/common/Rng.h. */
#include <stdint.h>

static uint64_t s[4];

static uint64_t splitmix64(uint64_t* x) {
    uint64_t z = (*x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

void ref_seed(uint64_t seed) {
    uint64_t x = seed;
    for (int i = 0; i < 4; ++i) s[i] = splitmix64(&x);
}

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

uint64_t ref_next(void) {
    const uint64_t result = rotl(s[1] * 5, 7) * 9;
    const uint64_t t = s[1] << 17;
    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rotl(s[3], 45);
    return result;
}
