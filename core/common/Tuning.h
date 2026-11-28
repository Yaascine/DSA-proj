// Tuning.h — gameplay tuning knobs (SKELETON, P1 owns the file, team fills values).
// Purpose: centralise balance numbers so designers tweak one header, not logic.
//          All values are integers to keep the simulation deterministic.
// Complexity: O(1).
// Who uses it: P2 environment, P3 humans/society. P1 only provides the skeleton.
// NOTE: placeholder values below are illustrative; owners finalise them.
#pragma once

namespace tge {

struct Tuning {
    // Environment (P2)
    int foodRegrowthPerTick   = 2;
    int fireSpreadChancePct   = 15;
    int floodRecedePerTick    = 5;

    // Humans (P3)
    int hungerPerTick         = 3;
    int starveDamageThreshold = 90;
    int theftChancePct        = 8;
    int rumorSpreadChancePct  = 20;

    // Society (P3)
    int leaderVoteThreshold   = 5;
    int embezzleChancePct     = 5;
};

} // namespace tge
