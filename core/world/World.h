// World.h — world contract (P2 OWNS the real implementation).
// Purpose: tile grid + resource/passability API consumed by P1 sim, P3 humans, P4 gui.
// NOTE: this is a CONTRACT-FAITHFUL PLACEHOLDER written by P1 so the engine runs
//       headlessly before P2 lands. P2 replaces generate()/findPath() with real
//       world generation and Dijkstra/A* navigation. The public API is frozen.
// Complexity: tile/passable/moveCost/take/add O(1); findPath O(n log n) (P2).
// Who uses it: everyone.
#pragma once
#include <vector>
#include "core/common/Types.h"
#include "core/common/Constants.h"
#include "core/common/Rng.h"
#include "core/common/Fnv.h"
#include "core/dsa/DynArray.h"

namespace tge {

struct Tile {
    Terrain terrain = Terrain::Grass;
    uint8_t elevation = 0, temperature = 50, moisture = 50, fertility = 50, vegetation = 0;
    uint16_t stock[4] = {0, 0, 0, 0}; // per Resource (Food, Water, Wood, Stone)
    uint8_t fire = 0;                 // 0 = none, >0 = burning ticks left
    uint8_t flood = 0;                // 0..100 water depth
    int16_t owner = -1;               // settlement id or -1
};

class World {
    int w_ = WORLD_W, h_ = WORLD_H;
    DynArray<Tile> tiles_;

    int idx(TileCoord c) const { return (int)c.y * w_ + (int)c.x; }
    bool inBounds(TileCoord c) const { return c.x >= 0 && c.y >= 0 && c.x < w_ && c.y < h_; }

public:
    World();
    int width() const { return w_; }
    int height() const { return h_; }
    const Tile& tile(TileCoord c) const { return tiles_[idx(c)]; }
    Tile& tileMut(TileCoord c) { return tiles_[idx(c)]; } // for P2 systems / God Mode

    bool passable(TileCoord c) const;
    int  moveCost(TileCoord c) const;
    int  takeResource(TileCoord c, Resource r, int amount); // returns what was REALLY taken
    void addResource(TileCoord c, Resource r, int amount);  // regrowth / God Mode only

    // P2 implements real pathfinding on our MinHeap/Queue/Graph. Placeholder: empty.
    std::vector<TileCoord> findPath(TileCoord from, TileCoord to) const;

    // One-time generation (floats allowed here ONLY). Deterministic via world RNG.
    void generate(Rng& world);

    void hashInto(Fnv& f) const; // canonical state for determinism
};

// SpatialGrid contract (P2 implements fully). Placeholder buckets per tile.
class SpatialGrid {
    int w_, h_;
    DynArray<DynArray<HumanId>> cells_;
public:
    SpatialGrid(int w, int h);
    void move(HumanId id, TileCoord from, TileCoord to);
    void nearby(TileCoord c, int radius, std::vector<HumanId>& out) const;
};

} // namespace tge
