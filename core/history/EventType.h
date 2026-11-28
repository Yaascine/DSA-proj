// EventType.h — the catalogue of recordable events (~25 types).
// Purpose: stable enum + display names shared by history, narrator and diagnostics.
// Complexity: O(1).
// Who uses it: P1 history/narrator; P2/P3 record events; P4 shows them.
#pragma once
#include <cstdint>

namespace tge {

enum class EventType : uint8_t {
    Birth = 0, Death, Theft, Hoarding, Gossip, Embezzle, Discovered, LeaderElected,
    Fire, Flood, Rescue, Drought, CropFailure, Hunger, Famine, Marriage, Feud,
    Migration, SettlementFounded, Harvest, Hunt, Sickness, Recovery, Trade,
    RumorSpread, COUNT
};

inline const char* eventTypeName(EventType t) {
    switch (t) {
        case EventType::Birth: return "Birth";
        case EventType::Death: return "Death";
        case EventType::Theft: return "Theft";
        case EventType::Hoarding: return "Hoarding";
        case EventType::Gossip: return "Gossip";
        case EventType::Embezzle: return "Embezzle";
        case EventType::Discovered: return "Discovered";
        case EventType::LeaderElected: return "LeaderElected";
        case EventType::Fire: return "Fire";
        case EventType::Flood: return "Flood";
        case EventType::Rescue: return "Rescue";
        case EventType::Drought: return "Drought";
        case EventType::CropFailure: return "CropFailure";
        case EventType::Hunger: return "Hunger";
        case EventType::Famine: return "Famine";
        case EventType::Marriage: return "Marriage";
        case EventType::Feud: return "Feud";
        case EventType::Migration: return "Migration";
        case EventType::SettlementFounded: return "SettlementFounded";
        case EventType::Harvest: return "Harvest";
        case EventType::Hunt: return "Hunt";
        case EventType::Sickness: return "Sickness";
        case EventType::Recovery: return "Recovery";
        case EventType::Trade: return "Trade";
        case EventType::RumorSpread: return "RumorSpread";
        default: return "Unknown";
    }
}

} // namespace tge
