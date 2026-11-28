// Human.h — human contract (P3 OWNS the full model).
// Purpose: plain-data human record consumed by P1 sim/history and P4 gui.
// NOTE: CONTRACT-FAITHFUL PLACEHOLDER by P1. P3 extends attributes/behaviour.
//       All simulation values are integers 0..100 unless noted. Entities reference
//       each other by ID, never by pointer.
// Complexity: O(1) plain data.
// Who uses it: P1 sim/history, P3 society, P4 gui.
#pragma once
#include <string>
#include "core/common/Types.h"
#include "core/common/Fnv.h"

namespace tge {

struct Inventory {
    int food = 0, water = 0, wood = 0, stone = 0;
};

enum class ActionType : uint8_t {
    Idle = 0, Move, Eat, Work, Steal, Socialize, Flee, Lead, COUNT
};

struct Human {
    HumanId id = -1;
    std::string name;
    int ageDays = 0;
    bool alive = true;
    TileCoord pos{0, 0};

    // Attributes (0..100). P3 finalises the full set.
    int hunger = 0;     // 0 = full, 100 = starving
    int health = 100;
    int wealth = 0;     // can exceed 100 (accumulated goods); kept integer
    int happiness = 50;

    Inventory inv;
    ActionType action = ActionType::Idle;

    void hashInto(Fnv& f) const {
        f.add32((uint32_t)id);
        f.addStr(name.data(), name.size());
        f.add32((uint32_t)ageDays);
        f.addBool(alive);
        f.add16((uint16_t)pos.x); f.add16((uint16_t)pos.y);
        f.add32((uint32_t)hunger); f.add32((uint32_t)health);
        f.add32((uint32_t)wealth); f.add32((uint32_t)happiness);
        f.add32((uint32_t)inv.food); f.add32((uint32_t)inv.water);
        f.add32((uint32_t)inv.wood); f.add32((uint32_t)inv.stone);
        f.add8((uint8_t)action);
    }
};

// Decision explanation for the GUI "why did they do that?" panel (P3 fills it).
struct Explanation {
    ActionType chosen = ActionType::Idle;
    struct Line { const char* what = ""; int value = 0, weight = 0, contribution = 0; };
    Line lines[8];
    int lineCount = 0;
};

} // namespace tge
