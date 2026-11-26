// THE GOD EXPERIMENT — Portable Core
// core/AppState.hpp — Application controller state, kept OUT of MFC classes.
//
// This is the single mutable "view state" the GUI reads/writes: camera,
// render options, hover/selection, lifecycle. Future modules will feed
// read-only simulation snapshots into the renderer through narrow interfaces;
// no simulation object ever lives inside an MFC window class.
//
// Module 01 scope: viewport/camera presentation state ONLY. There is no
// civilization simulation here — that is deliberate (see MODULE_STATUS.md).

#pragma once

#include <cmath>
#include <cstdint>
#include <optional>

#include "godexperiment/core/Camera.hpp"
#include "godexperiment/core/Types.hpp"

namespace godex::core {

// ---------------------------------------------------------------------------
// RenderOptions — toggles surfaced by the left panel in Module 01.
// mapMode fields exist as disabled placeholders documented for Module 03+.
// ---------------------------------------------------------------------------
struct RenderOptions {
    bool showGrid{true};         // grid lines over tiles
    bool showCoordinates{true};  // axis labels + hovered tile coordinate text
    bool showOriginMarker{true}; // highlighted (0,0) origin cross
    bool showBackgroundPattern{true}; // subtle checker pattern under grid
};

// ---------------------------------------------------------------------------
// ViewportInfo — pixel size of the Direct2D target, updated on resize.
// ---------------------------------------------------------------------------
struct ViewportInfo {
    std::uint32_t widthPx{0};
    std::uint32_t heightPx{0};

    constexpr bool isValid() const noexcept { return widthPx > 0 && heightPx > 0; }
};

// ---------------------------------------------------------------------------
// HoverState / SelectionState — deliberately separate lifecycles:
// hover follows the mouse continuously; selection changes on click and can be
// cleared with Escape.
// ---------------------------------------------------------------------------
struct HoverState {
    std::optional<TileCoord> tile; // nullopt when pointer outside viewport
    Vec2F screenPos{0.0f, 0.0f};   // last raw pointer position (status bar)
};

struct SelectionState {
    std::optional<TileCoord> tile;
    void clear() noexcept { tile.reset(); }
    bool select(const TileCoord& c) noexcept { // returns true if changed
        if (tile && *tile == c) return false;
        tile = c;
        return true;
    }
};

// ---------------------------------------------------------------------------
// AppLifecycle — coarse application phase, useful for status bar & shutdown.
// ---------------------------------------------------------------------------
enum class AppPhase : std::uint8_t {
    Starting,
    Running,
    ShuttingDown,
};

// ---------------------------------------------------------------------------
// ViewState — aggregate owned by the application controller (CMainFrame in
// the MFC layer holds exactly one instance). All mutation goes through its
// methods so transitions stay testable without any window handle.
// ---------------------------------------------------------------------------
class ViewState {
public:
    ViewState() noexcept { m_camera.reset(); } // deterministic initial pose

    Camera& camera() noexcept { return m_camera; }
    const Camera& camera() const noexcept { return m_camera; }

    RenderOptions& options() noexcept { return m_options; }
    const RenderOptions& options() const noexcept { return m_options; }

    ViewportInfo& viewport() noexcept { return m_viewport; }
    const ViewportInfo& viewport() const noexcept { return m_viewport; }

    const HoverState& hover() const noexcept { return m_hover; }
    SelectionState& selection() noexcept { return m_selection; }
    const SelectionState& selection() const noexcept { return m_selection; }

    AppPhase phase() const noexcept { return m_phase; }
    void setPhase(AppPhase p) noexcept { m_phase = p; }

    // --- Interaction intents (pure state transitions, unit-testable) -------
    void onPointerMoved(float sx, float sy) noexcept {
        m_hover.screenPos = {sx, sy};
        if (!m_viewport.isValid() || sx < 0 || sy < 0 ||
            sx >= static_cast<float>(m_viewport.widthPx) ||
            sy >= static_cast<float>(m_viewport.heightPx)) {
            m_hover.tile.reset();
        } else {
            m_hover.tile = m_camera.screenToTile(sx, sy);
        }
    }
    void onPointerLeft() noexcept { m_hover.tile.reset(); }

    /// Click selects the tile under the cursor; returns whether selection changed.
    bool onClickSelect() noexcept {
        if (!m_hover.tile) return false;
        return m_selection.select(*m_hover.tile);
    }
    void clearSelection() noexcept { m_selection.clear(); }

    void onWheelZoom(float notchCount, float anchorX, float anchorY) noexcept {
        // notchCount > 0 => zoom in. Uses pow for multi-notch deltas.
        const float factor = std::pow(Camera::kZoomStep, notchCount);
        m_camera.zoomAt(factor, anchorX, anchorY);
    }
    void onDragPan(float dxPx, float dyPx) noexcept { m_camera.panPixels(dxPx, dyPx); }

    void resetCamera() noexcept { m_camera.reset(); }
    void toggleGrid() noexcept { m_options.showGrid = !m_options.showGrid; }

private:
    Camera m_camera;
    RenderOptions m_options{};
    ViewportInfo m_viewport{};
    HoverState m_hover{};
    SelectionState m_selection{};
    AppPhase m_phase{AppPhase::Starting};
};

} // namespace godex::core
