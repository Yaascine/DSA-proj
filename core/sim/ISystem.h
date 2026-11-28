// ISystem.h — pluggable simulation system interface (FROZEN CONTRACT, P1).
// Purpose: P2 (environment/navigation) and P3 (humans/society) register systems that
//          the Simulation ticks in a FIXED order. One concept per system.
// Complexity: O(system work).
// Who uses it: P1 Simulation calls; P2/P3 implement.
#pragma once

namespace tge {

struct SimContext; // forward declaration (defined in SimContext.h)

struct ISystem {
    virtual void onTick(SimContext& ctx) = 0;
    virtual ~ISystem() {}
};

} // namespace tge
