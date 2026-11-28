// World.cpp — placeholder world implementation (P2 replaces generate/findPath).
#include "core/world/World.h"
#include "core/common/Constants.h"

namespace tge {

World::World() { tiles_.resize((size_t)w_ * h_); }

bool World::passable(TileCoord c) const {
    if (!inBounds(c)) return false;
    const Tile& t = tile(c);
    if (t.terrain == Terrain::DeepOcean) return false;
    if (t.fire > 0) return false;
    if (t.flood >= 50) return false;
    return true;
}

int World::moveCost(TileCoord c) const {
    if (!inBounds(c)) return 1000;
    switch (tile(c).terrain) {
        case Terrain::Grass: case Terrain::Beach: return 1;
        case Terrain::Forest: case Terrain::Desert: return 2;
        case Terrain::Hills: case Terrain::Ocean: case Terrain::Lake: return 3;
        case Terrain::Snow: return 4;
        case Terrain::Mountain: return 5;
        default: return 1000; // DeepOcean
    }
}

int World::takeResource(TileCoord c, Resource r, int amount) {
    if (!inBounds(c) || amount <= 0) return 0;
    Tile& t = tileMut(c);
    int ri = (int)r;
    int taken = amount < (int)t.stock[ri] ? amount : (int)t.stock[ri];
    t.stock[ri] = (uint16_t)(t.stock[ri] - taken);
    return taken; // never creates resources
}

void World::addResource(TileCoord c, Resource r, int amount) {
    if (!inBounds(c) || amount <= 0) return;
    Tile& t = tileMut(c);
    int ri = (int)r;
    int sum = (int)t.stock[ri] + amount;
    t.stock[ri] = (uint16_t)(sum > 9999 ? 9999 : sum);
}

std::vector<TileCoord> World::findPath(TileCoord, TileCoord) const {
    return {}; // P2 implements Dijkstra/A* on our MinHeap/Queue/Graph.
}

void World::generate(Rng& world) {
    for (int y = 0; y < h_; ++y) {
        for (int x = 0; x < w_; ++x) {
            Tile& t = tiles_[(size_t)y * w_ + x];
            // Distance from centre biases terrain: ocean rim, land interior.
            int dx = x - w_ / 2, dy = y - h_ / 2;
            int dist2 = dx * dx + dy * dy;
            int roll = world.range(0, 99);
            if (dist2 > 1400) t.terrain = (roll < 60) ? Terrain::DeepOcean : Terrain::Ocean;
            else if (dist2 > 1100) t.terrain = (roll < 50) ? Terrain::Ocean : Terrain::Beach;
            else {
                if (roll < 35) t.terrain = Terrain::Grass;
                else if (roll < 60) t.terrain = Terrain::Forest;
                else if (roll < 72) t.terrain = Terrain::Hills;
                else if (roll < 82) t.terrain = Terrain::Desert;
                else if (roll < 92) t.terrain = Terrain::Mountain;
                else t.terrain = Terrain::Lake;
            }
            t.elevation   = (uint8_t)world.range(0, 100);
            t.temperature = (uint8_t)world.range(0, 100);
            t.moisture    = (uint8_t)world.range(0, 100);
            t.fertility   = (uint8_t)world.range(0, 100);
            t.vegetation  = (uint8_t)(t.terrain == Terrain::Forest ? world.range(40, 100) : world.range(0, 40));
            t.stock[(int)Resource::Food]  = (uint16_t)(t.terrain == Terrain::Grass || t.terrain == Terrain::Forest ? world.range(20, 120) : world.range(0, 20));
            t.stock[(int)Resource::Water] = (uint16_t)(t.terrain == Terrain::Lake || t.terrain == Terrain::Ocean ? 500 : world.range(0, 60));
            t.stock[(int)Resource::Wood]  = (uint16_t)(t.terrain == Terrain::Forest ? world.range(40, 160) : world.range(0, 15));
            t.stock[(int)Resource::Stone] = (uint16_t)(t.terrain == Terrain::Mountain || t.terrain == Terrain::Hills ? world.range(40, 160) : world.range(0, 10));
            t.fire = 0; t.flood = 0; t.owner = -1;
        }
    }
}

void World::hashInto(Fnv& f) const {
    f.add32((uint32_t)w_);
    f.add32((uint32_t)h_);
    for (size_t i = 0; i < tiles_.size(); ++i) {
        const Tile& t = tiles_[i];
        f.add8((uint8_t)t.terrain);
        f.add8(t.elevation); f.add8(t.temperature); f.add8(t.moisture);
        f.add8(t.fertility); f.add8(t.vegetation);
        for (int r = 0; r < 4; ++r) f.add16(t.stock[r]);
        f.add8(t.fire); f.add8(t.flood);
        f.add16((uint16_t)t.owner);
    }
}

SpatialGrid::SpatialGrid(int w, int h) : w_(w), h_(h) { cells_.resize((size_t)w * h); }

void SpatialGrid::move(HumanId id, TileCoord from, TileCoord to) {
    if (from.x >= 0 && from.y >= 0 && from.x < w_ && from.y < h_) {
        auto& cell = cells_[(size_t)from.y * w_ + from.x];
        for (size_t i = 0; i < cell.size(); ++i)
            if (cell[i] == id) { cell.swapRemoveAt(i); break; }
    }
    if (to.x >= 0 && to.y >= 0 && to.x < w_ && to.y < h_)
        cells_[(size_t)to.y * w_ + to.x].push_back(id);
}

void SpatialGrid::nearby(TileCoord c, int radius, std::vector<HumanId>& out) const {
    for (int y = c.y - radius; y <= c.y + radius; ++y)
        for (int x = c.x - radius; x <= c.x + radius; ++x) {
            if (x < 0 || y < 0 || x >= w_ || y >= h_) continue;
            const auto& cell = cells_[(size_t)y * w_ + x];
            for (size_t i = 0; i < cell.size(); ++i) out.push_back(cell[i]);
        }
}

} // namespace tge
