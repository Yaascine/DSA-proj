// prng_ref.h — C linkage bridge to the reference PRNG (test-only).
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void ref_seed(uint64_t seed);
uint64_t ref_next(void);
#ifdef __cplusplus
}
#endif
