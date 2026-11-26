// THE GOD EXPERIMENT — Portable Core
// core/Camera.hpp — 2D viewport camera math (platform-independent).
//
// Coordinate convention:
//   * World space: tile units. Tile (x, y) occupies [x, x+1) x [y, y+1).
//     X increases rightward, Y increases downward, origin top-left.
//   * Screen space: pixels, origin at viewport top-left, Y downward.
//   * scale = pixels per tile (float). Camera position = world coords of the
//     screen-center anchor... we instead store top-left mapping for exactness:
//       screenX = (worldX - posX) * scale
//       screenY = (worldY - posY) * scale
//
// The camera knows nothing about world size or generation; it only transforms
// coordinates and enforces zoom limits.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "godexperiment/core/Types.hpp"

namespace godex::core {

struct Vec2F {
    float x{0.0f};
    float y{0.0f};
};

class Camera {
public:
    // Zoom limits: 4 px per tile (pixel-art readable minimum) .. 96 px/tile.
    static constexpr float kMinScale = 4.0f;
    static constexpr float kMaxScale = 96.0f;
    static constexpr float kZoomStep = 1.15f;          // per wheel notch
    static constexpr int   kDefaultTileSizePxHint = 24; // initial fit

    constexpr Camera() noexcept = default;

    // --- Transformation -----------------------------------------------------
    constexpr Vec2F worldToScreen(const Vec2F& world) const noexcept {
        return {(world.x - m_pos.x) * m_scale, (world.y - m_pos.y) * m_scale};
    }
    constexpr Vec2F screenToWorld(const Vec2F& screen) const noexcept {
        // Guard against degenerate scale (should never happen due to clamping).
        const float inv = m_scale != 0.0f ? 1.0f / m_scale : 0.0f;
        return {screen.x * inv + m_pos.x, screen.y * inv + m_pos.y};
    }

    /// Tile containing a screen pixel (floor division, works for negatives).
    TileCoord screenToTile(float sx, float sy) const noexcept {
        const Vec2F w = screenToWorld({sx, sy});
        return TileCoord{FloorToInt(w.x), FloorToInt(w.y)};
    }

    /// Visible world rectangle [left,right] x [top,bottom] in tile units.
    void visibleTileRange(std::uint32_t vpW, std::uint32_t vpH,
                          TileCoord& outMin, TileCoord& outMax) const noexcept {
        const Vec2F tl = screenToWorld({0.0f, 0.0f});
        const Vec2F br = screenToWorld({static_cast<float>(vpW), static_cast<float>(vpH)});
        outMin = TileCoord{FloorToInt(tl.x), FloorToInt(tl.y)};
        outMax = TileCoord{FloorToInt(br.x), FloorToInt(br.y)};
    }

    // --- Manipulation -------------------------------------------------------
    void panPixels(float dxPx, float dyPx) noexcept {
        m_pos.x += dxPx / m_scale;
        m_pos.y += dyPx / m_scale;
    }

    /// Zoom by `factor` keeping the given SCREEN point fixed on screen.
    void zoomAt(float factor, float anchorScreenX, float anchorScreenY) noexcept {
        const Vec2F before = screenToWorld({anchorScreenX, anchorScreenY});
        m_scale = ClampScale(m_scale * factor);
        const Vec2F after = screenToWorld({anchorScreenX, anchorScreenY});
        m_pos.x += before.x - after.x;
        m_pos.y += before.y - after.y;
    }

    void zoomIn(float anchorX, float anchorY) noexcept { zoomAt(kZoomStep, anchorX, anchorY); }
    void zoomOut(float anchorX, float anchorY) noexcept { zoomAt(1.0f / kZoomStep, anchorX, anchorY); }

    void reset() noexcept {
        m_pos = {0.0f, 0.0f};
        m_scale = kDefaultTileSizePxHint;
    }

    // --- Accessors ----------------------------------------------------------
    constexpr float scale() const noexcept { return m_scale; }
    constexpr const Vec2F& position() const noexcept { return m_pos; }
    void setPosition(const Vec2F& p) noexcept { m_pos = p; }
    void setScale(float s) noexcept { m_scale = ClampScale(s); }

    static constexpr float ClampScale(float s) noexcept {
        if (s < kMinScale) return kMinScale;
        if (s > kMaxScale) return kMaxScale;
        return s;
    }

    /// floor-to-int with defined behavior for the ranges we accept
    /// (guards against NaN / beyond-int32 values from extreme offsets).
    static TileCoord::component FloorToInt(float v) noexcept {
        if (!(v > -2.0e9f)) return std::numeric_limits<TileCoord::component>::min();
        if (!(v < 2.0e9f)) return std::numeric_limits<TileCoord::component>::max();
        return static_cast<TileCoord::component>(std::floor(v));
    }

private:
    Vec2F m_pos{0.0f, 0.0f};                 // world coord of screen (0,0)
    float m_scale{kDefaultTileSizePxHint};   // pixels per tile
};

} // namespace godex::core
