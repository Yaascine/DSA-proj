// SaveLoad.cpp — canonical JSON (de)serialisation with nlohmann/json.
#include "core/persistence/SaveLoad.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

namespace tge {
namespace SaveLoad {

using nlohmann::json;

static json rngToJson(const Rng& r) {
    return json::array({r.s[0], r.s[1], r.s[2], r.s[3]});
}
static void rngFromJson(Rng& r, const json& j) {
    for (int i = 0; i < 4; ++i) r.s[i] = j[i].get<uint64_t>();
}

std::string toJson(const Simulation& sim) {
    json j;
    j["schemaVersion"] = SCHEMA_VERSION;
    j["seed"] = sim.seed();
    j["tick"] = sim.tick();
    j["nextHumanId"] = sim.nextHumanId();

    j["rng"]["world"] = rngToJson(sim.rngWorld());
    j["rng"]["population"] = rngToJson(sim.rngPop());
    j["rng"]["event"] = rngToJson(sim.rngEvent());
    j["rng"]["narration"] = rngToJson(sim.rngNarr());

    const Metrics& m = sim.metrics();
    j["metrics"] = {{"population", m.population}, {"births", m.births}, {"deaths", m.deaths},
                    {"thefts", m.thefts}, {"fires", m.fires}, {"floods", m.floods},
                    {"rumors", m.rumors}, {"scandals", m.scandals}, {"marriages", m.marriages},
                    {"rescues", m.rescues}};

    const World& w = sim.world();
    j["world"]["w"] = w.width();
    j["world"]["h"] = w.height();
    json tiles = json::array();
    for (int y = 0; y < w.height(); ++y)
        for (int x = 0; x < w.width(); ++x) {
            const Tile& t = w.tile({(int16_t)x, (int16_t)y});
            tiles.push_back({{"terrain", (int)t.terrain}, {"elevation", t.elevation},
                {"temperature", t.temperature}, {"moisture", t.moisture},
                {"fertility", t.fertility}, {"vegetation", t.vegetation},
                {"stock", {t.stock[0], t.stock[1], t.stock[2], t.stock[3]}},
                {"fire", t.fire}, {"flood", t.flood}, {"owner", t.owner}});
        }
    j["world"]["tiles"] = tiles;

    json humans = json::array();
    const auto& hs = sim.humans();
    for (size_t i = 0; i < hs.size(); ++i) { // ascending ID order
        const Human& h = hs[i];
        humans.push_back({{"id", h.id}, {"name", h.name}, {"ageDays", h.ageDays},
            {"alive", h.alive}, {"pos", {h.pos.x, h.pos.y}}, {"hunger", h.hunger},
            {"health", h.health}, {"wealth", h.wealth}, {"happiness", h.happiness},
            {"inv", {h.inv.food, h.inv.water, h.inv.wood, h.inv.stone}},
            {"action", (int)h.action}});
    }
    j["humans"] = humans;

    const Scheduler& sc = sim.scheduler();
    j["scheduler"]["nextSeq"] = sc.nextSeq();
    json items = json::array();
    const auto& hi = sc.heap().items();
    for (size_t i = 0; i < hi.size(); ++i)
        items.push_back({{"key", hi[i].key}, {"seq", hi[i].seq}, {"payload", hi[i].payload}});
    j["scheduler"]["items"] = items;

    json events = json::array();
    const auto& evs = sim.history().events();
    for (size_t i = 0; i < evs.size(); ++i) {
        const EventRecord& e = evs[i];
        json actors = json::array();
        for (size_t a = 0; a < e.actors.size(); ++a) actors.push_back(e.actors[a]);
        json causes = json::array();
        for (size_t c = 0; c < e.causes.size(); ++c) causes.push_back(e.causes[c]);
        events.push_back({{"id", e.id}, {"tick", e.tick}, {"type", (int)e.type},
            {"actors", actors}, {"where", {e.where.x, e.where.y}}, {"param", e.param},
            {"causes", causes}, {"flags", e.flags}});
    }
    j["history"] = events;

    return j.dump(2);
}

bool fromJson(Simulation& sim, const std::string& text) {
    json j = json::parse(text, nullptr, false);
    if (j.is_discarded()) return false;
    if (!j.contains("schemaVersion") || j["schemaVersion"].get<int>() != SCHEMA_VERSION)
        return false;

    sim.setTick(j["tick"].get<Tick>());
    sim.setNextHumanId(j["nextHumanId"].get<HumanId>());

    rngFromJson(sim.rngWorld(), j["rng"]["world"]);
    rngFromJson(sim.rngPop(), j["rng"]["population"]);
    rngFromJson(sim.rngEvent(), j["rng"]["event"]);
    rngFromJson(sim.rngNarr(), j["rng"]["narration"]);

    const json& mj = j["metrics"];
    Metrics m;
    m.population = mj.value("population", 0); m.births = mj.value("births", 0);
    m.deaths = mj.value("deaths", 0); m.thefts = mj.value("thefts", 0);
    m.fires = mj.value("fires", 0); m.floods = mj.value("floods", 0);
    m.rumors = mj.value("rumors", 0); m.scandals = mj.value("scandals", 0);
    m.marriages = mj.value("marriages", 0); m.rescues = mj.value("rescues", 0);
    sim.metrics() = m;

    World& w = sim.world();
    const json& tiles = j["world"]["tiles"];
    for (int y = 0; y < w.height(); ++y)
        for (int x = 0; x < w.width(); ++x) {
            const json& t = tiles[(size_t)y * w.width() + x];
            Tile& tile = w.tileMut({(int16_t)x, (int16_t)y});
            tile.terrain = (Terrain)t["terrain"].get<int>();
            tile.elevation = t["elevation"].get<uint8_t>();
            tile.temperature = t["temperature"].get<uint8_t>();
            tile.moisture = t["moisture"].get<uint8_t>();
            tile.fertility = t["fertility"].get<uint8_t>();
            tile.vegetation = t["vegetation"].get<uint8_t>();
            for (int r = 0; r < 4; ++r) tile.stock[r] = t["stock"][r].get<uint16_t>();
            tile.fire = t["fire"].get<uint8_t>();
            tile.flood = t["flood"].get<uint8_t>();
            tile.owner = t["owner"].get<int16_t>();
        }

    auto& hs = sim.humans();
    hs.clear();
    for (const json& h : j["humans"]) {
        Human hu;
        hu.id = h["id"].get<HumanId>();
        hu.name = h["name"].get<std::string>();
        hu.ageDays = h["ageDays"].get<int>();
        hu.alive = h["alive"].get<bool>();
        hu.pos = {(int16_t)h["pos"][0].get<int>(), (int16_t)h["pos"][1].get<int>()};
        hu.hunger = h["hunger"].get<int>(); hu.health = h["health"].get<int>();
        hu.wealth = h["wealth"].get<int>(); hu.happiness = h["happiness"].get<int>();
        hu.inv = {h["inv"][0].get<int>(), h["inv"][1].get<int>(),
                  h["inv"][2].get<int>(), h["inv"][3].get<int>()};
        hu.action = (ActionType)h["action"].get<int>();
        hs.push_back(std::move(hu));
    }

    DynArray<HeapItem> items;
    for (const json& it : j["scheduler"]["items"])
        items.push_back(HeapItem{it["key"].get<int64_t>(), it["seq"].get<int32_t>(), it["payload"].get<int32_t>()});
    sim.scheduler().restore(items, j["scheduler"]["nextSeq"].get<int32_t>());

    DynArray<EventRecord> evs;
    for (const json& e : j["history"]) {
        EventRecord rec;
        rec.id = e["id"].get<EventId>();
        rec.tick = e["tick"].get<Tick>();
        rec.type = (EventType)e["type"].get<int>();
        rec.where = {(int16_t)e["where"][0].get<int>(), (int16_t)e["where"][1].get<int>()};
        rec.param = e["param"].get<int>();
        rec.flags = e["flags"].get<uint32_t>();
        for (const json& a : e["actors"]) rec.actors.push_back(a.get<HumanId>());
        for (const json& c : e["causes"]) rec.causes.push_back(c.get<EventId>());
        evs.push_back(std::move(rec));
    }
    sim.history().restore(evs);
    return true;
}

bool save(const Simulation& sim, const std::string& path) {
    std::ofstream o(path);
    if (!o) return false;
    o << toJson(sim);
    return (bool)o;
}

bool loadInto(Simulation& sim, const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    std::stringstream ss;
    ss << in.rdbuf();
    return fromJson(sim, ss.str());
}

} // namespace SaveLoad
} // namespace tge
