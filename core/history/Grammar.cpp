// Grammar.cpp — symbol expansion with modifiers, narration-RNG variant selection.
#include "core/history/Grammar.h"

namespace tge {

void Grammar::add(const std::string& symbol, const std::string& text) {
    rules_[symbol].push_back(text);
}

std::string Grammar::applyModifier(const std::string& value, const std::string& mod) {
    if (value.empty()) return value;
    if (mod == "capitalize") {
        std::string r = value;
        if (r[0] >= 'a' && r[0] <= 'z') r[0] = (char)(r[0] - 'a' + 'A');
        return r;
    }
    if (mod == "a") {
        char c = value[0];
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        bool vowel = (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
        return (vowel ? "an " : "a ") + value;
    }
    if (mod == "s") {
        // Simple pluralisation; irregular forms are avoided by the line writers.
        size_t n = value.size();
        if (n >= 1 && (value[n - 1] == 's' || value[n - 1] == 'x' || value[n - 1] == 'z'))
            return value + "es";
        return value + "s";
    }
    return value;
}

std::string Grammar::expandText(const std::string& text,
                                const std::map<std::string, std::string>& vars,
                                Rng& narration, int depth) const {
    if (depth > 16) return text; // cycle guard
    std::string out;
    size_t i = 0;
    while (i < text.size()) {
        if (text[i] != '#') { out += text[i++]; continue; }
        size_t close = text.find('#', i + 1);
        if (close == std::string::npos) { out += text.substr(i); break; }
        std::string token = text.substr(i + 1, close - i - 1);
        i = close + 1;

        std::string name = token, mod;
        size_t dot = token.find('.');
        if (dot != std::string::npos) { name = token.substr(0, dot); mod = token.substr(dot + 1); }

        std::string value;
        auto vit = vars.find(name);
        if (vit != vars.end()) value = vit->second;
        else {
            auto rit = rules_.find(name);
            if (rit != rules_.end() && !rit->second.empty()) {
                const auto& variants = rit->second;
                const std::string& pick = variants[narration.range(0, (int)variants.size() - 1)];
                value = expandText(pick, vars, narration, depth + 1);
            } else value = ""; // unknown symbol -> empty (never invent facts)
        }
        out += applyModifier(value, mod);
    }
    return out;
}

std::string Grammar::expand(const std::string& symbol,
                            const std::map<std::string, std::string>& vars,
                            Rng& narration) const {
    auto it = rules_.find(symbol);
    if (it == rules_.end() || it->second.empty()) return "";
    const auto& variants = it->second;
    const std::string& pick = variants[narration.range(0, (int)variants.size() - 1)];
    return expandText(pick, vars, narration, 0);
}

} // namespace tge
