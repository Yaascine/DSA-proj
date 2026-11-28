// Simulation.cpp — deterministic engine: commands, scheduler, systems, snapshot, hash.
#include "core/sim/Simulation.h"
#include "core/common/Constants.h"
#include <utility>

namespace tge {

namespace {
const char* NAMES[] = {
    "Ada", "Bram", "Cora", "Dorn", "Edda", "Finn", "Greta", "Hale", "Iris", "Joran",
    "Kessa", "Liam", "Mira", "Nolan", "Opal", "Piers", "Quinn", "Rosa", "Sten", "Tessa",
    "Ulric", "Vera", "Wren", "Xan", "Yara", "Zev", "Anya", "Bo", "Cyrus", "Dalia",
    "Emil", "Freya", "Gideon", "Hana", "Ivo", "Juno", "Kael", "Lena", "Magnus", "Nia"
};
constexpr int NAME_COUNT = (int)(sizeof(NAMES) / sizeof(NAMES[0]));
}

const char* Simulation::randomName(Rng& pop) { return NAMES[pop.range(0, NAME_COUNT - 1)]; }

Simulation::Simulation(uint64_t seed, int initialPop)
    : seed_(seed),
      rngWorld_(seed),
      rngPop_(seed + 0x1000ULL),
      rngEvent_(seed + 0x2000ULL),
      rngNarr_((seed + 0x2000ULL) ^ 0xABCDEFULL) {
    humans_.setStats(&dsa_);
    commands_.setStats(&dsa_);
    scheduler_.setStats(&dsa_);
    world_.generate(rngWorld_);
    for (int i = 0; i < initialPop; ++i) {
        TileCoord pos{0, 0};
        for (int tries = 0; tries < 200; ++tries) {
            TileCoord c{(int16_t)rngPop_.range(0, WORLD_W - 1), (int16_t)rngPop_.range(0, WORLD_H - 1)};
            if (world_.passable(c)) { pos = c; break; }
        }
        spawnHuman(pos, false);
    }
    metrics_.births = 0; // initial population is the starting state, not sim births
    publishSnapshot();
}

HumanId Simulation::spawnHuman(TileCoord pos, bool playerCaused) {
    Human h;
    h.id = nextHumanId_++;
    h.name = randomName(rngPop_);
    h.pos = pos;
    h.ageDays = rngPop_.range(18 * TICKS_PER_YEAR, 55 * TICKS_PER_YEAR);
    h.hunger = rngPop_.range(0, 30);
    h.health = 100;
    h.wealth = rngPop_.range(0, 60);
    h.happiness = rngPop_.range(30, 70);
    humans_.push_back(std::move(h));
    history_.record(EventType::Birth, tick_, {h.id}, pos, 0, {},
                    playerCaused ? EF_PLAYER_CAUSED : EF_VERIFIED);
    ++metrics_.births;
    return h.id;
}

HumanId Simulation::birthHuman(TileCoord pos) {
    // Newborn: age 0, default stats. ID comes from the SAME authority as spawnHuman,
    // so births and God-Mode spawns can never collide.
    Human h;
    h.id = nextHumanId_++;
    h.name = randomName(rngPop_);
    h.pos = pos;
    h.ageDays = 0;
    humans_.push_back(std::move(h));
    history_.record(EventType::Birth, tick_, {h.id}, pos, 0, {}, EF_VERIFIED);
    ++metrics_.births;
    return h.id;
}

void Simulation::applyCommands() {
    while (!commands_.empty()) applyOne(commands_.pop());
}

void Simulation::applyOne(const Command& c) {
    auto inBounds = [&](int x, int y) {
        return x >= 0 && y >= 0 && x < world_.width() && y < world_.height();
    };
    switch (c.type) {
        case CmdType::SpawnHuman: {
            TileCoord pos{(int16_t)c.a, (int16_t)c.b};
            if (!inBounds(c.a, c.b) || !world_.passable(pos)) {
                for (int tries = 0; tries < 200; ++tries) {
                    TileCoord t{(int16_t)rngPop_.range(0, WORLD_W - 1), (int16_t)rngPop_.range(0, WORLD_H - 1)};
                    if (world_.passable(t)) { pos = t; break; }
                }
            }
            spawnHuman(pos, true);
            break;
        }
        case CmdType::AddResource:
            if (inBounds(c.a, c.b)) world_.addResource({(int16_t)c.a, (int16_t)c.b}, (Resource)c.c, c.d);
            break;
        case CmdType::StartFire:
            if (inBounds(c.a, c.b)) {
                world_.tileMut({(int16_t)c.a, (int16_t)c.b}).fire = (uint8_t)c.c;
                history_.record(EventType::Fire, tick_, {}, {(int16_t)c.a, (int16_t)c.b}, c.c, {}, EF_PLAYER_CAUSED);
                ++metrics_.fires;
            }
            break;
        case CmdType::StartFlood:
            if (inBounds(c.a, c.b)) {
                world_.tileMut({(int16_t)c.a, (int16_t)c.b}).flood = (uint8_t)c.c;
                history_.record(EventType::Flood, tick_, {}, {(int16_t)c.a, (int16_t)c.b}, c.c, {}, EF_PLAYER_CAUSED);
                ++metrics_.floods;
            }
            break;
        case CmdType::EditAttribute:
            if (c.target >= 0 && (size_t)c.target < humans_.size()) {
                Human& h = humans_[c.target];
                int* field = nullptr;
                switch (c.a) {
                    case 0: field = &h.hunger; break;
                    case 1: field = &h.health; break;
                    case 2: field = &h.wealth; break;
                    case 3: field = &h.happiness; break;
                }
                if (field) {
                    int v = *field + c.c;
                    if (c.a == 2) { if (v < 0) v = 0; }
                    else { if (v < ATTR_MIN) v = ATTR_MIN; if (v > ATTR_MAX) v = ATTR_MAX; }
                    *field = v;
                }
            }
            break;
        case CmdType::PlantRumor:
            history_.record(EventType::RumorSpread, tick_, {c.target}, {0, 0}, c.c, {}, EF_PLAYER_CAUSED | EF_RUMOR_ONLY);
            ++metrics_.rumors;
            break;
        case CmdType::SetLeader:
            history_.record(EventType::LeaderElected, tick_, {c.target}, {0, 0}, c.c, {}, EF_PLAYER_CAUSED);
            break;
    }
}

void Simulation::step() {
    due_.clear();
    applyCommands();                       // 1) commands
    scheduler_.popDue(tick_, due_);        // 2) due scheduled events

    SimContext ctx;                        // 3) systems in fixed order
    ctx.tick = tick_;
    ctx.sim = this;
    ctx.world = &rngWorld_; ctx.population = &rngPop_;
    ctx.event = &rngEvent_; ctx.narration = &rngNarr_;
    ctx.worldObj = &world_; ctx.humans = &humans_;
    ctx.history = &history_; ctx.scheduler = &scheduler_;
    ctx.dueEvents = &due_; ctx.metrics = &metrics_;
    for (size_t i = 0; i < systems_.size(); ++i) systems_[i]->onTick(ctx);

    publishSnapshot();                     // 4) publish
    ++tick_;                               // 5) advance
}

void Simulation::publishSnapshot() {
    int pop = 0;
    for (size_t i = 0; i < humans_.size(); ++i) if (humans_[i].alive) ++pop;
    metrics_.population = pop;

    snap_.tick = tick_;
    snap_.world = &world_;
    snap_.humans = &humans_;
    snap_.history = &history_;
    snap_.metrics = metrics_;
    snap_.recentEvents.clear();
    size_t n = history_.size();
    size_t start = n > SNAPSHOT_EVENT_WINDOW ? n - SNAPSHOT_EVENT_WINDOW : 0;
    for (size_t i = start; i < n; ++i) snap_.recentEvents.push_back((EventId)i);
    DsaStats agg;
    agg.add(dsa_);
    agg.add(history_.stats());
    snap_.dsa = agg;
    snap_.stateHash = stateHash();
}

uint64_t Simulation::stateHash() const {
    Fnv f;
    f.add64((uint64_t)tick_);
    f.add64(seed_);
    const Rng* streams[4] = {&rngWorld_, &rngPop_, &rngEvent_, &rngNarr_};
    for (int s = 0; s < 4; ++s)
        for (int i = 0; i < 4; ++i) f.add64(streams[s]->s[i]);
    world_.hashInto(f);
    f.add64((uint64_t)humans_.size());
    for (size_t i = 0; i < humans_.size(); ++i) humans_[i].hashInto(f); // ascending ID order
    scheduler_.hashInto(f);
    history_.hashInto(f);
    f.add32((uint32_t)metrics_.population);
    f.add32((uint32_t)metrics_.births); f.add32((uint32_t)metrics_.deaths);
    f.add32((uint32_t)metrics_.thefts); f.add32((uint32_t)metrics_.fires);
    f.add32((uint32_t)metrics_.floods); f.add32((uint32_t)metrics_.rumors);
    f.add32((uint32_t)metrics_.scandals); f.add32((uint32_t)metrics_.marriages);
    f.add32((uint32_t)metrics_.rescues);
    f.add64((uint64_t)commands_.size());
    return f.h;
}

} // namespace tge
