// THE GOD EXPERIMENT — Tests
// Test 03: camera calculations (pure math, no MFC).
#include "TestFramework.hpp"
#include "godexperiment/core/Camera.hpp"

using namespace godex::core;

TEST_CASE(Camera_default_pose) {
    Camera c; // after default ctor fields are zero-init; use reset for pose
    c.reset();
    CHECK_NEAR(c.scale(), 24.0f, 1e-6);
    CHECK_NEAR(c.position().x, 0.0f, 1e-6);
    CHECK_NEAR(c.position().y, 0.0f, 1e-6);
}

TEST_CASE(Camera_world_to_screen) {
    Camera c; c.reset();
    c.setPosition({10.0f, 20.0f});
    c.setScale(8.0f);
    const Vec2F s = c.worldToScreen({12.0f, 25.0f});
    CHECK_NEAR(s.x, (12 - 10) * 8.0, 1e-4);   // 16
    CHECK_NEAR(s.y, (25 - 20) * 8.0, 1e-4);   // 40
}

TEST_CASE(Camera_screen_to_world_inverse) {
    Camera c; c.reset();
    c.setPosition({-3.5f, 7.25f});
    c.setScale(13.0f);
    const Vec2F p{123.0f, 456.0f};
    const Vec2F w = c.screenToWorld(p);
    const Vec2F back = c.worldToScreen(w);
    CHECK_NEAR(back.x, p.x, 1e-3);
    CHECK_NEAR(back.y, p.y, 1e-3);
}

TEST_CASE(Camera_roundtrip_random_poses) {
    // Round-trip within tolerance across many poses/zooms.
    float scales[] = {4.0f, 7.3f, 24.0f, 55.5f, 96.0f};
    for (float sc : scales) {
        Camera c; c.reset(); c.setScale(sc);
        c.setPosition({-1000.25f, 999.75f});
        for (int i = 0; i < 50; ++i) {
            Vec2F p{static_cast<float>(i) * 13.7f, static_cast<float>(i) * -9.3f};
            Vec2F rt = c.worldToScreen(c.screenToWorld(p));
            CHECK_NEAR(rt.x, p.x, 0.05);
            CHECK_NEAR(rt.y, p.y, 0.05);
        }
    }
}

TEST_CASE(Camera_pan_translation) {
    Camera c; c.reset();
    const auto before = c.position();
    // Convention: dragging the mouse by (+dx,+dy) moves the WORLD with the
    // cursor, so the camera's top-left world anchor DEcreases.
    c.panPixels(48.0f, -24.0f); // scale=24 => anchor shifts (-2, +1) tiles
    CHECK_NEAR(c.position().x, before.x + 48.0f / 24.0f, 1e-5);
    CHECK_NEAR(c.position().y, before.y - 24.0f / 24.0f, 1e-5);
}

TEST_CASE(Camera_zoom_limits) {
    Camera c; c.reset();
    for (int i = 0; i < 200; ++i) c.zoomIn(0, 0);
    CHECK_NEAR(c.scale(), Camera::kMaxScale, 1e-5);
    for (int i = 0; i < 400; ++i) c.zoomOut(0, 0);
    CHECK_NEAR(c.scale(), Camera::kMinScale, 1e-5);
    CHECK(Camera::ClampScale(-5.0f) == Camera::kMinScale);
    CHECK(Camera::ClampScale(1e9f) == Camera::kMaxScale);
}

TEST_CASE(Camera_zoom_around_mouse_anchor) {
    Camera c; c.reset();
    const float ax = 300.0f, ay = 200.0f;
    const Vec2F worldAtAnchorBefore = c.screenToWorld({ax, ay});
    c.zoomAt(1.5f, ax, ay);
    const Vec2F worldAtAnchorAfter = c.screenToWorld({ax, ay});
    CHECK_NEAR(worldAtAnchorBefore.x, worldAtAnchorAfter.x, 1e-3);
    CHECK_NEAR(worldAtAnchorBefore.y, worldAtAnchorAfter.y, 1e-3);
}

TEST_CASE(Camera_screen_to_tile_negative_and_floor) {
    Camera c; c.reset();
    // scale 24: pixel (23.9, 23.9) -> tile (0,0); (-0.1,-0.1) -> tile (-1,-1)
    CHECK(c.screenToTile(23.9f, 23.9f) == TileCoord(0, 0));
    CHECK(c.screenToTile(-0.1f, -0.1f) == TileCoord(-1, -1));
    CHECK(c.screenToTile(24.0f, 48.0f) == TileCoord(1, 2));
}

TEST_CASE(Camera_extreme_offset_safety) {
    Camera c; c.reset();
    c.setPosition({1.5e8f, -1.5e8f}); // huge offset
    const TileCoord t = c.screenToTile(100.0f, 100.0f);
    // Must produce finite int32 coords without UB.
    CHECK(t.x >= std::numeric_limits<std::int32_t>::min());
    CHECK(t.x <= std::numeric_limits<std::int32_t>::max());
}

int main() { return testfw::runAll("camera"); }
