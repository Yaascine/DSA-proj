// THE GOD EXPERIMENT — Tests
// Test 04 + 05: viewport selection math and application state transitions.
#include "TestFramework.hpp"
#include "godexperiment/core/AppState.hpp"

using namespace godex::core;

// ---------------------------------------------------------------------------
// Test 04 — Viewport selection mathematics
// ---------------------------------------------------------------------------
TEST_CASE(Selection_correct_tile_at_various_zooms) {
    ViewState vs;
    vs.viewport() = {800, 600};
    // default scale 24 => pixel (100,100) is tile (4,4)
    vs.onPointerMoved(100.0f, 100.0f);
    CHECK(vs.hover().tile.has_value());
    CHECK(vs.hover().tile && *vs.hover().tile == TileCoord(4, 4));

    vs.camera().setScale(7.0f); // noninteger zoom
    vs.onPointerMoved(100.0f, 100.0f);
    CHECK(vs.hover().tile && *vs.hover().tile == TileCoord(14, 14));
}

TEST_CASE(Selection_negative_coordinates) {
    ViewState vs;
    vs.viewport() = {800, 600};
    vs.camera().setPosition({10.0f, 10.0f}); // world origin at screen -240px
    vs.onPointerMoved(0.0f, 0.0f);           // -> world (10,10) tile (10,10)
    CHECK(vs.hover().tile && *vs.hover().tile == TileCoord(10, 10));
    vs.camera().setPosition({0.5f, 0.5f});
    vs.onPointerMoved(0.0f, 0.0f); // world (0.5,0.5) still tile (0,0)
    CHECK(vs.hover().tile && *vs.hover().tile == TileCoord(0, 0));
    vs.camera().setPosition({2.0f, 2.0f});
    vs.onPointerMoved(-1.0f, -1.0f); // outside viewport => no hover tile
    CHECK(!vs.hover().tile.has_value());
}

TEST_CASE(Selection_tile_boundaries) {
    ViewState vs;
    vs.viewport() = {800, 600};
    const float s = vs.camera().scale();
    vs.onPointerMoved(s - 0.001f, s - 0.001f);
    CHECK(vs.hover().tile && *vs.hover().tile == TileCoord(0, 0));
    vs.onPointerMoved(s, s);
    CHECK(vs.hover().tile && *vs.hover().tile == TileCoord(1, 1));
}

TEST_CASE(Selection_extreme_camera_offsets) {
    ViewState vs;
    vs.viewport() = {800, 600};
    vs.camera().setPosition({1.0e7f, -1.0e7f});
    vs.onPointerMoved(400.0f, 300.0f);
    CHECK(vs.hover().tile.has_value()); // must not crash / produce garbage NaN
    if (vs.hover().tile) {
        CHECK(vs.hover().tile->x >= 10000000 - 100);
        CHECK(vs.hover().tile->y <= -10000000 + 100);
    }
}

TEST_CASE(Selection_visible_range_is_culled) {
    ViewState vs;
    TileCoord lo, hi;
    vs.camera().visibleTileRange(240, 240, lo, hi); // 10 tiles wide at scale 24
    CHECK(lo == TileCoord(0, 0));
    CHECK(hi == TileCoord(9, 9)); // inclusive last fully-visible tile index
    // A viewport that ends mid-tile includes that partial tile.
    vs.camera().visibleTileRange(241, 241, lo, hi);
    CHECK(hi == TileCoord(10, 10));
}

// ---------------------------------------------------------------------------
// Test 05 — Application state
// ---------------------------------------------------------------------------
TEST_CASE(State_initial_camera) {
    ViewState vs;
    CHECK_NEAR(vs.camera().scale(), 24.0f, 1e-6);
    CHECK_NEAR(vs.camera().position().x, 0.0f, 1e-6);
    CHECK(vs.options().showGrid == true);
    CHECK(!vs.selection().tile.has_value());
    CHECK(!vs.hover().tile.has_value());
}

TEST_CASE(State_reset_camera) {
    ViewState vs;
    vs.viewport() = {800, 600};
    vs.camera().setPosition({123.0f, -45.0f});
    vs.camera().setScale(50.0f);
    vs.resetCamera();
    CHECK_NEAR(vs.camera().scale(), 24.0f, 1e-6);
    CHECK_NEAR(vs.camera().position().x, 0.0f, 1e-6);
    CHECK_NEAR(vs.camera().position().y, 0.0f, 1e-6);
}

TEST_CASE(State_selection_updates) {
    ViewState vs;
    vs.viewport() = {800, 600};
    vs.onPointerMoved(30.0f, 30.0f);      // tile (1,1)
    CHECK(vs.onClickSelect() == true);     // changed
    CHECK(vs.selection().tile && *vs.selection().tile == TileCoord(1, 1));
    CHECK(vs.onClickSelect() == false);    // same tile => no change event
    vs.onPointerMoved(300.0f, 300.0f);     // tile (12,12)
    CHECK(vs.onClickSelect() == true);
    CHECK(vs.selection().tile && *vs.selection().tile == TileCoord(12, 12));
    vs.clearSelection();                   // Escape
    CHECK(!vs.selection().tile.has_value());
}

TEST_CASE(State_grid_toggle) {
    ViewState vs;
    const bool initial = vs.options().showGrid;
    vs.toggleGrid();
    CHECK(vs.options().showGrid == !initial);
    vs.toggleGrid();
    CHECK(vs.options().showGrid == initial);
}

TEST_CASE(State_valid_transitions_and_wheel_zoom) {
    ViewState vs;
    vs.viewport() = {800, 600};
    vs.setPhase(AppPhase::Running);
    CHECK(vs.phase() == AppPhase::Running);
    const float before = vs.camera().scale();
    vs.onWheelZoom(1.0f, 400.0f, 300.0f);
    CHECK(vs.camera().scale() > before);   // zoomed in
    vs.onWheelZoom(-1.0f, 400.0f, 300.0f);
    CHECK_NEAR(vs.camera().scale(), before, 1e-4); // back to start
    vs.setPhase(AppPhase::ShuttingDown);
    CHECK(vs.phase() == AppPhase::ShuttingDown);
}

TEST_CASE(State_drag_pan_changes_hover) {
    ViewState vs;
    vs.viewport() = {800, 600};
    vs.onPointerMoved(100.0f, 100.0f);
    const auto t0 = *vs.hover().tile;
    // Dragging the map right/down by one tile (screen delta +24,+24 px) makes
    // the same screen pixel show a tile one step LEFT/UP in world space.
    vs.onDragPan(24.0f, 24.0f);
    vs.onPointerMoved(100.0f, 100.0f);
    const auto t1 = *vs.hover().tile;
    CHECK(t1 == TileCoord(t0.x - 1, t0.y - 1));
}

int main() { return testfw::runAll("state_and_selection"); }
