// Command.h — God-Mode commands (FROZEN CONTRACT, P1 queue; P4 pushes; P2/P3 apply).
// Purpose: the ONLY way the GUI changes the simulation. The GUI never mutates state.
// Field convention (kept generic so P2/P3 can interpret):
//   a,b = tile x,y (or attribute selector for EditAttribute)
//   c   = amount / resource / duration
//   d   = extra parameter
//   target = HumanId (or -1)
// Complexity: O(1) plain data.
// Who uses it: P4 gui pushes; P1 sim queues; P2/P3 handlers apply.
#pragma once
#include "core/common/Types.h"

namespace tge {

enum class CmdType : uint8_t {
    SpawnHuman = 0, EditAttribute, AddResource, StartFire, StartFlood, PlantRumor, SetLeader
};

struct Command {
    CmdType type = CmdType::SpawnHuman;
    int a = 0, b = 0, c = 0, d = 0;
    HumanId target = -1;
};

} // namespace tge
