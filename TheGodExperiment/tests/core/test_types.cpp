// THE GOD EXPERIMENT — Tests
// Test 01/07 helper: core types unit tests.
#include "TestFramework.hpp"
#include "godexperiment/core/Types.hpp"

using namespace godex::core;

TEST_CASE(EntityId_basic) {
    EntityId a{7}, b{7}, c{9};
    CHECK(a == b);
    CHECK(a != c);
    CHECK(a < c);
    CHECK(EntityId::null().isNull());
    CHECK(!a.isNull());
    CHECK_EQ(a.value(), 7ull);
    CHECK_EQ(a.toString(), std::string("7"));
}

TEST_CASE(TileCoord_compare_and_arith) {
    TileCoord p{2, 3}, q{2, 3}, r{-1, 5};
    CHECK(p == q);
    CHECK(p != r);
    CHECK(r < p);            // x-major ordering
    auto s = p + TileCoord{1, -1};
    CHECK(s == TileCoord(3, 2));
    CHECK(p.distanceTo(TileCoord(2, 6)) == 3.0);
}

TEST_CASE(TileCoord_overflow_guard) {
    const TileCoord big{std::numeric_limits<std::int32_t>::max(), 0};
    auto ok = TileCoord::tryAdd(big, TileCoord{1, 0});
    CHECK(!ok.has_value());  // overflow detected, no wraparound
    auto fine = TileCoord::tryAdd(TileCoord{1, 2}, TileCoord{3, 4});
    CHECK(fine.has_value());
    CHECK(fine && *fine == TileCoord(4, 6));
}

TEST_CASE(WorldDimensions_validation) {
    CHECK(WorldDimensions{128, 128}.isValid());
    CHECK(WorldDimensions{0, 10}.isValid() == false);
    CHECK(WorldDimensions{10, 0}.isValid() == false);
    CHECK(WorldDimensions{WorldDimensions::kMaxAxis + 1, 5}.isValid() == false);
    CHECK_EQ((WorldDimensions{100, 50}.area()), 5000ull);
    CHECK(WorldDimensions{10, 10}.contains(TileCoord{9, 9}));
    CHECK(!WorldDimensions{10, 10}.contains(TileCoord{10, 0}));
    CHECK(!WorldDimensions{10, 10}.contains(TileCoord{-1, 0}));
}

TEST_CASE(NormalizedValue_bounds) {
    CHECK(NormalizedValue::validate(0.5f).has_value());
    CHECK(NormalizedValue::validate(-0.01f) == std::nullopt);
    CHECK(NormalizedValue::validate(1.01f) == std::nullopt);
    CHECK(NormalizedValue::validate(std::nanf("")) == std::nullopt);
    CHECK_EQ(NormalizedValue::clamp(1.7f).value(), 1.0f);
    CHECK_EQ(NormalizedValue::clamp(-2.0f).value(), 0.0f);
    auto v = NormalizedValue::validate(0.75f);
    CHECK(v && v->toInt(0, 100) == 75);
}

TEST_CASE(SeedSet_defaults_and_equality) {
    SeedSet a{};
    SeedSet b{1, 2, 3};
    CHECK(a == SeedSet{});
    CHECK(!(a == b));
    CHECK_EQ(b.worldSeed, 1ull);
}

int main() { return testfw::runAll("core_types"); }
