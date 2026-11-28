// Grammar.h — tiny Tracery-inspired template engine (P1, ~100 lines of logic).
// Purpose: expand "#symbol#" templates into narration text. Variants are chosen
//          with the NARRATION RNG stream ONLY, so editing a joke never changes history.
// Modifiers: #name.capitalize#, #name.a# (a/an), #name.s# (plural).
// Complexity: O(output length) with a recursion-depth guard.
// Who uses it: P1 Narrator. NOT used by simulation logic.
// (std::map/string/vector are fine here — the narrator is not a graded DSA structure.)
#pragma once
#include <string>
#include <map>
#include <vector>
#include "core/common/Rng.h"

namespace tge {

class Grammar {
    std::map<std::string, std::vector<std::string>> rules_;

    std::string expandText(const std::string& text,
                           const std::map<std::string, std::string>& vars,
                           Rng& narration, int depth) const;
    static std::string applyModifier(const std::string& value, const std::string& mod);

public:
    void add(const std::string& symbol, const std::string& text);
    bool has(const std::string& symbol) const { return rules_.count(symbol) > 0; }
    std::string expand(const std::string& symbol,
                       const std::map<std::string, std::string>& vars,
                       Rng& narration) const;
};

} // namespace tge
