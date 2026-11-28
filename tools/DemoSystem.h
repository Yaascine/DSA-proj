// DemoSystem.h — DEMO-ONLY life system so the P1 engine runs headlessly today.
// Purpose: exercise the engine, history, cause graph and narrator with real events.
// NOTE: this is NOT the real game logic. P3 replaces it with humans/society systems;
//       P2 replaces the environment regrowth with real ecology. Kept in tools/ so it
//       is never mistaken for core gameplay. Deterministic: uses ctx RNG streams only.
// Complexity: O(tiles + humans) per tick.
// Who uses it: HeadlessMain runner + SeedHunt tool.
#pragma once
#include "core/sim/ISystem.h"
#include "core/sim/SimContext.h"
#include "core/sim/Simulation.h"
#include "core/common/Constants.h"
#include <vector>

namespace tge {

class DemoSystem : public ISystem {
public:
    void onTick(SimContext& ctx) override {
        World& w = *ctx.worldObj;
        auto& humans = *ctx.humans;
        History& h = *ctx.history;
        Rng& ev = *ctx.event;
        Rng& pop = *ctx.population;
        Metrics& m = *ctx.metrics;

        // --- environment (placeholder ecology) ---
        for (int y = 0; y < w.height(); ++y)
            for (int x = 0; x < w.width(); ++x) {
                Tile& t = w.tileMut({(int16_t)x, (int16_t)y});
                if (t.fire > 0) --t.fire;
                if (t.flood > 0) t.flood = (uint8_t)(t.flood > 5 ? t.flood - 5 : 0);
                if (t.terrain == Terrain::Grass || t.terrain == Terrain::Forest)
                    w.addResource({(int16_t)x, (int16_t)y}, Resource::Food, 1);
            }

        // --- humans decide & act in ascending ID order ---
        for (size_t i = 0; i < humans.size(); ++i) {
            Human& hu = humans[i];
            if (!hu.alive) continue;
            hu.ageDays++;
            hu.hunger += 3; if (hu.hunger > 100) hu.hunger = 100;

            int taken = w.takeResource(hu.pos, Resource::Food, 5);
            hu.hunger -= taken * 4; if (hu.hunger < 0) hu.hunger = 0;
            hu.inv.food += taken;

            // wander to a neighbouring passable tile
            int dx = ev.range(-1, 1), dy = ev.range(-1, 1);
            TileCoord np{(int16_t)(hu.pos.x + dx), (int16_t)(hu.pos.y + dy)};
            if (w.passable(np)) hu.pos = np;

            EventId hungerEv = -1;
            if (hu.hunger >= 80) {
                hungerEv = h.record(EventType::Hunger, ctx.tick, {hu.id}, hu.pos, hu.hunger, {}, EF_VERIFIED);
                // theft cites the hunger event EXPLICITLY (no inferred causation)
                if (ev.chance(10)) {
                    for (size_t j = 0; j < humans.size(); ++j) {
                        Human& v = humans[j];
                        if (!v.alive || v.id == hu.id) continue;
                        int dx2 = v.pos.x - hu.pos.x, dy2 = v.pos.y - hu.pos.y;
                        if (dx2 < -2 || dx2 > 2 || dy2 < -2 || dy2 > 2) continue;
                        int stolen = v.inv.food > 3 ? 3 : v.inv.food;
                        if (stolen > 0) {
                            v.inv.food -= stolen; hu.inv.food += stolen; hu.wealth += stolen;
                            h.record(EventType::Theft, ctx.tick, {hu.id, v.id}, hu.pos, stolen, {hungerEv}, EF_VERIFIED);
                            ++m.thefts;
                        }
                        break;
                    }
                }
            }

            if (ev.chance(2)) {
                hu.health -= ev.range(5, 20);
                h.record(EventType::Sickness, ctx.tick, {hu.id}, hu.pos, hu.health, {}, EF_VERIFIED);
            }
            if (ev.chance(3)) {
                HumanId listener = (HumanId)ev.range(0, (int)humans.size() - 1);
                HumanId subject = (HumanId)ev.range(0, (int)humans.size() - 1);
                h.record(EventType::Gossip, ctx.tick, {hu.id, listener, subject}, hu.pos, ev.range(10, 90), {}, EF_RUMOR_ONLY);
                ++m.rumors;
            }
            if (ev.chance(1) && hu.wealth > 40) {
                h.record(EventType::Embezzle, ctx.tick, {hu.id}, hu.pos, ev.range(5, 30), {}, EF_VERIFIED);
                if (ev.chance(40)) {
                    h.record(EventType::Discovered, ctx.tick, {hu.id}, hu.pos, hu.wealth, {}, EF_VERIFIED);
                    ++m.scandals;
                }
            }

            // death checks
            int cause = -1;
            if (hu.hunger >= 100) cause = 0;
            else if (hu.health <= 0) cause = 4;
            else if (hu.ageDays > 80 * TICKS_PER_YEAR) cause = 1;
            if (cause >= 0) {
                hu.alive = false;
                std::vector<EventId> causes;
                if (cause == 0 && hungerEv >= 0) causes.push_back(hungerEv);
                h.record(EventType::Death, ctx.tick, {hu.id}, hu.pos, cause, causes, EF_VERIFIED);
                ++m.deaths;
            }
        }

        // --- births (population stream) ---
        if (pop.chance(15)) {
            int alive = 0;
            for (size_t i = 0; i < humans.size(); ++i) if (humans[i].alive) ++alive;
            if (alive > 0 && alive < 200) {
                Human child;
                child.id = (HumanId)humans.size();
                child.name = Simulation::randomName(pop);
                child.ageDays = 0;
                child.pos = humans[pop.range(0, (int)humans.size() - 1)].pos;
                humans.push_back(std::move(child));
                h.record(EventType::Birth, ctx.tick, {child.id}, child.pos, 0, {}, EF_VERIFIED);
                ++m.births;
            }
        }
    }
};

} // namespace tge
