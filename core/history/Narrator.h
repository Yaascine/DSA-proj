// Narrator.h — deadpan nature-documentary narration + obituaries (P1).
// Purpose: turn recorded events into text. STRICT: only facts from EventRecords;
//          never invent events. Uses the narration RNG stream exclusively.
// Tone: jokes about human behaviour and bad systems only. Never real groups,
//       religions or nationalities. Never sexual violence (the model has none).
// Complexity: narrate O(events for actor) for grudges; obituary O(events for human).
// Who uses it: P1 headless demo / P4 gui ticker.
#pragma once
#include <string>
#include "core/common/Rng.h"
#include "core/dsa/DynArray.h"
#include "core/humans/Human.h"
#include "core/history/History.h"
#include "core/history/Grammar.h"

namespace tge {

class Narrator {
    Grammar grammar_;
    void buildRules();

    static std::string deathCause(int param);
    std::string humanName(const DynArray<Human>& humans, HumanId id) const;
    int countEventsOfType(const History& h, HumanId id, EventType a, EventType b) const;

public:
    Narrator();
    std::string narrate(const EventRecord& e, const History& history,
                        const DynArray<Human>& humans, Rng& narration) const;
    std::string obituary(HumanId id, const History& history,
                         const DynArray<Human>& humans, Rng& narration) const;
    const Grammar& grammar() const { return grammar_; }
};

} // namespace tge
