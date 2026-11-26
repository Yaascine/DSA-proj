// THE GOD EXPERIMENT — Portable Core
// core/Random.cpp — out-of-line pieces of the RNG stream registry.

#include "godexperiment/core/Types.hpp" // must precede Random.hpp: completes std::hash specializations
#include "godexperiment/core/Random.hpp"

namespace godex::core {

RngStreams::RngStreams(const SeedSet& seeds) noexcept
    : m_worldSeed{seeds.worldSeed},
      m_populationSeed{seeds.populationSeed},
      m_eventSeed{seeds.eventSeed} {}

std::uint64_t RngStreams::baseFor(std::string_view name) const noexcept {
    // Canonical streams map directly to their dedicated seed; any extra
    // auxiliary stream (e.g. "combat", "weather") derives from the world
    // seed so it stays reproducible without extending SeedSet each time.
    if (name == "world") return m_worldSeed;
    if (name == "population") return m_populationSeed;
    if (name == "event") return m_eventSeed;
    return m_worldSeed;
}

Pcg32& RngStreams::stream(std::string_view name) {
    const std::string key{name};
    auto it = m_streams.find(key);
    if (it != m_streams.end()) return it->second;
    const std::uint64_t derived = splitmix64(baseFor(name) ^ nameSalt(name));
    auto [inserted, ok] = m_streams.emplace(key, Pcg32{derived});
    (void)ok;
    return inserted->second;
}

} // namespace godex::core
