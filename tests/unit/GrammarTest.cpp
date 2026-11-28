// GrammarTest.cpp — template expansion, modifiers, determinism, narration-only RNG.
#include "doctest.h"
#include "core/history/Grammar.h"

TEST_CASE("Grammar expands symbols and applies modifiers") {
    tge::Grammar g;
    g.add("animal", "fox");
    g.add("sentence", "The #animal.capitalize# saw #animal.a#.");
    tge::Rng narr(1);
    std::map<std::string, std::string> vars;
    std::string out = g.expand("sentence", vars, narr);
    CHECK(out == "The Fox saw a fox.");
}

TEST_CASE("Grammar a/an modifier picks by vowel") {
    tge::Grammar g;
    g.add("x", "#w.a#");
    tge::Rng narr(1);
    std::map<std::string, std::string> vars;
    vars["w"] = "apple";
    CHECK(g.expand("x", vars, narr) == "an apple");
    vars["w"] = "pear";
    CHECK(g.expand("x", vars, narr) == "a pear");
}

TEST_CASE("Grammar plural modifier") {
    tge::Grammar g;
    g.add("x", "#w.s#");
    tge::Rng narr(1);
    std::map<std::string, std::string> vars;
    vars["w"] = "grudge";
    CHECK(g.expand("x", vars, narr) == "grudges");
    vars["w"] = "box";
    CHECK(g.expand("x", vars, narr) == "boxes");
}

TEST_CASE("Grammar uses vars before rules and leaves unknown symbols empty") {
    tge::Grammar g;
    g.add("x", "#known#/#unknown#");
    tge::Rng narr(1);
    std::map<std::string, std::string> vars;
    vars["known"] = "yes";
    CHECK(g.expand("x", vars, narr) == "yes/");
}

TEST_CASE("Grammar expansion is deterministic for a fixed narration seed") {
    tge::Grammar g;
    g.add("v", "alpha");
    g.add("v", "beta");
    g.add("v", "gamma");
    g.add("x", "#v#");
    std::map<std::string, std::string> vars;
    tge::Rng a(99), b(99);
    for (int i = 0; i < 50; ++i) CHECK(g.expand("x", vars, a) == g.expand("x", vars, b));
}
