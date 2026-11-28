// Fnv.h — FNV-1a 64-bit hash accumulator.
// Purpose: build the canonical stateHash() over every simulation field in ID order.
//          If two runs differ, bisect by hashing per system to locate the drift.
// Complexity: O(bytes fed).
// Who uses it: P1 sim/stateHash, P1 persistence determinism tests.
#pragma once
#include <cstdint>
#include <cstddef>

namespace tge {

struct Fnv {
    uint64_t h = 14695981039346656037ULL; // FNV offset basis
    void add(uint8_t b) { h ^= b; h *= 1099511628211ULL; } // FNV prime
    void add64(uint64_t v) { for (int i = 0; i < 8; ++i) add((uint8_t)(v >> (8 * i))); }
    void add32(uint32_t v) { for (int i = 0; i < 4; ++i) add((uint8_t)(v >> (8 * i))); }
    void add16(uint16_t v) { for (int i = 0; i < 2; ++i) add((uint8_t)(v >> (8 * i))); }
    void add8(uint8_t v)   { add(v); }
    void addBool(bool b)   { add(b ? 1 : 0); }
    void addBytes(const void* p, size_t n) {
        const uint8_t* b = (const uint8_t*)p;
        for (size_t i = 0; i < n; ++i) add(b[i]);
    }
    // Length-prefixed string so "ab"+"c" and "a"+"bc" hash differently.
    void addStr(const char* s, size_t n) { add64((uint64_t)n); addBytes(s, n); }
};

} // namespace tge
