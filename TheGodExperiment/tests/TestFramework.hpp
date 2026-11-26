// THE GOD EXPERIMENT — Tests
// tests/TestFramework.hpp — ultra-light native C++ test harness (no deps).
// TEST_CASE registration + CHECK macros; main() provided by TestMain.cpp.

#pragma once

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace testfw {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

struct Failure {
    std::string message;
};

inline int& failureCount() { static int n = 0; return n; }

struct Registrar {
    Registrar(const char* name, std::function<void()> fn) {
        registry().push_back({name, std::move(fn)});
    }
};

#define TEST_CASE(name)                                                        \
    static void testfn_##name();                                               \
    static ::testfw::Registrar reg_##name(#name, &testfn_##name);              \
    static void testfn_##name()

#define CHECK(...)                                                             \
    do {                                                                       \
        if (!(__VA_ARGS__)) {                                                  \
            ++::testfw::failureCount();                                        \
            std::printf("    FAIL %s:%d: %s\n", __FILE__, __LINE__,             \
                        #__VA_ARGS__);                                         \
        }                                                                      \
    } while (0)

#define CHECK_EQ(...) /* handled below */                                      \
    static_assert(true, "")

// Variadic-safe equality check: splits on the LAST top-level comma is hard in
// the preprocessor; instead define CHECK_EQ via __VA_ARGS__ requiring callers
// wrap braced-init args. Simpler: use two-arg macro with parenthesized args.
#undef CHECK_EQ
#define CHECK_EQ(a, b)                                                         \
    do {                                                                       \
        const auto va = (a); const auto vb = (b);                              \
        if (!(va == vb)) {                                                     \
            ++::testfw::failureCount();                                        \
            std::printf("    FAIL %s:%d: %s == %s\n", __FILE__, __LINE__,      \
                        #a, #b);                                               \
        }                                                                      \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                  \
    do {                                                                       \
        const double va = (a), vb = (b);                                       \
        if (std::fabs(va - vb) > (eps)) {                                      \
            ++::testfw::failureCount();                                        \
            std::printf("    FAIL %s:%d: |%s - %s| <= %g (got %g vs %g)\n",    \
                        __FILE__, __LINE__, #a, #b, (double)(eps), va, vb);    \
        }                                                                      \
    } while (0)

inline int runAll(const char* suiteName) {
    std::printf("[RUN ] %s: %d cases\n", suiteName, (int)registry().size());
    for (auto& tc : registry()) {
        const int before = failureCount();
        tc.fn();
        const bool ok = failureCount() == before;
        std::printf("[%s]   %s\n", ok ? " OK" : "FAIL", tc.name.c_str());
    }
    const int fails = failureCount();
    std::printf("[DONE] %s: %d assertion failures\n", suiteName, fails);
    return fails == 0 ? 0 : 1;
}

} // namespace testfw
