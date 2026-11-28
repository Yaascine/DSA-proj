// Narrator.cpp — event lines (25 types x 3 variants), obituaries from recorded facts.
#include "core/history/Narrator.h"
#include "core/common/Constants.h"
#include <map>

namespace tge {

Narrator::Narrator() { buildRules(); }

std::string Narrator::deathCause(int p) {
    switch (p) {
        case 0: return "starvation";
        case 1: return "old age";
        case 2: return "fire";
        case 3: return "flood";
        case 4: return "sickness";
        case 5: return "a feud";
        case 6: return "exposure";
        default: return "misadventure";
    }
}

std::string Narrator::humanName(const DynArray<Human>& humans, HumanId id) const {
    if (id >= 0 && (size_t)id < humans.size()) return humans[id].name;
    return "someone";
}

int Narrator::countEventsOfType(const History& h, HumanId id, EventType a, EventType b) const {
    int n = 0;
    for (EventId eid : h.eventsFor(id)) {
        EventType t = h.get(eid).type;
        if (t == a || t == b) ++n;
    }
    return n;
}

void Narrator::buildRules() {
    auto add3 = [&](const char* sym, const char* a, const char* b, const char* c) {
        grammar_.add(sym, a); grammar_.add(sym, b); grammar_.add(sym, c);
    };
    add3("Birth",
        "#name# arrived on day #tick#. The world was not consulted.",
        "Day #tick#: #name# joined the population. Paperwork: none.",
        "#name# was born near #place#. Expectations: cautiously low.");
    add3("Death",
        "#name# died of #cause# at #age#. #survivors# survive them. #grudges# grudges survive them too.",
        "Day #tick#: #name# died of #cause#. The ledger closes the account.",
        "#name# (#age#) died of #cause# near #place#. Survived by #survivors# and #grudges# unresolved arguments.");
    add3("Theft",
        "#actor# took #n# #item# from #victim#. #actor# calls it redistribution. #victim# calls it theft.",
        "Day #tick#: #victim# is down #n# #item#. #actor# is up #n# #item# and down one friend.",
        "#actor# relieved #victim# of #n# #item#. The friendship was not relieved.");
    add3("Hoarding",
        "#actor# is sitting on #n# units of food during the famine. Bold strategy.",
        "#actor# hoarded #n# food while neighbours went without. A plan, technically.",
        "Day #tick#: #actor# guards #n# food. Sharing remains theoretical.");
    add3("Gossip",
        "#speaker# told #listener# about #subject#. Accuracy: #accuracy#.",
        "#speaker# informed #listener# about #subject#. Verification: pending forever.",
        "Day #tick#: a rumour about #subject# reached #listener# via #speaker#. Truth: optional.");
    add3("Embezzle",
        "The storehouse is short #n# food. #leader# has never looked healthier.",
        "#leader# skimmed #n# from the storehouse. The books balance creatively.",
        "Day #tick#: #n# food vanished. #leader# calls it administrative overhead.");
    add3("Discovered",
        "#leader# was caught skimming the storehouse. The ledger had opinions.",
        "Day #tick#: #leader#'s #n#-unit discrepancy was discovered. Trust: recalculated.",
        "#leader# was found out. The storehouse remembers everything.");
    add3("LeaderElected",
        "#name# became leader. #votes# people voted. #names# were not consulted.",
        "Day #tick#: #name# took charge with #votes# votes. The rest is silence.",
        "#name# is now leader. #votes# ballots, several unasked opinions.");
    add3("Fire",
        "A fire started near #place#. It has no plan, but it is executing it well.",
        "Day #tick#: fire near #place#. Enthusiasm: high. Direction: none.",
        "Smoke over #place#. The fire is self-motivated.");
    add3("Flood",
        "Water rose near #place#. It has no plan, but it is executing it well.",
        "Day #tick#: flooding at #place#. The river had feedback.",
        "#place# is underwater. The water is committed.");
    add3("Rescue",
        "#actor# carried #victim# out of the flood. #victim# will not be asked about the bread incident.",
        "Day #tick#: #actor# rescued #victim#. Gratitude: noted. Bread: forgiven.",
        "#actor# pulled #victim# to safety. The debt is informal.");
    add3("Drought",
        "The rain stopped near #place#. The clouds filed no explanation.",
        "Day #tick#: drought at #place#. The sky is on a break.",
        "#place# dries out. Water: reconsidered.");
    add3("CropFailure",
        "The crops failed near #place#. They had one job.",
        "Day #tick#: harvest collapse at #place#. The soil shrugs.",
        "#place# yielded nothing. The fields are introspective.");
    add3("Hunger",
        "#actor# is hungry (#n#). The stomach has submitted a formal complaint.",
        "Day #tick#: #actor#'s hunger reached #n#. Dinner: hypothetical.",
        "#actor# goes without. Hunger: #n# and climbing.");
    add3("Famine",
        "Famine near #place#. #n# affected. The storehouse is a rumour.",
        "Day #tick#: famine at #place#, #n# hungry. Plans: none.",
        "#place# starves. #n# people, zero calories.");
    add3("Marriage",
        "#actor# and #victim# married on day #tick#. The alliance is now binding.",
        "#actor# wed #victim#. Two households, one argument.",
        "Day #tick#: #actor# and #victim# joined. Witnesses: several. Regrets: pending.");
    add3("Feud",
        "#actor# and #victim# are feuding. The original reason is lost to time.",
        "Day #tick#: #actor# declared against #victim#. Memory: long. Forgiveness: short.",
        "#actor# vs #victim#. The grudge is now hereditary.");
    add3("Migration",
        "#actor# left for #place#. The old tile keeps the memories.",
        "Day #tick#: #actor# migrated to #place#. Reasons: practical.",
        "#actor# moved on. #place# gains one opinion.");
    add3("SettlementFounded",
        "#actor# founded a settlement near #place#. Ambition: #n# units.",
        "Day #tick#: a settlement rises at #place#, led by #actor#. Paperwork: imagined.",
        "#actor# planted a flag near #place#. The flag is optimistic.");
    add3("Harvest",
        "#actor# gathered #n# food near #place#. The soil cooperated briefly.",
        "Day #tick#: #actor# harvested #n#. A rare win.",
        "#actor# brought in #n# food. The fields exhale.");
    add3("Hunt",
        "#actor# hunted near #place#, returning with #n#. The forest files a complaint.",
        "Day #tick#: #actor# bagged #n#. The prey had other plans.",
        "#actor# returned with #n# from the hunt. Effort: visible.");
    add3("Sickness",
        "#actor# fell ill (#n#). The body has opened negotiations.",
        "Day #tick#: #actor# is sick, severity #n#. Rest: prescribed. Ignored: likely.",
        "#actor# caught something. Health: #n# and bargaining.");
    add3("Recovery",
        "#actor# recovered (#n#). The body closed negotiations successfully.",
        "Day #tick#: #actor# is well again. The illness left no forwarding address.",
        "#actor# bounced back to #n# health. Resilience: noted.");
    add3("Trade",
        "#actor# traded #n# goods with #victim#. Both sides claim victory.",
        "Day #tick#: #actor# and #victim# exchanged #n#. Value: subjective.",
        "#actor# swapped #n# with #victim#. The market is a rumour with numbers.");
    add3("RumorSpread",
        "A rumour reached #actor# (#n# hops). Accuracy: unverified.",
        "Day #tick#: the story reached #actor# after #n# retellings. Truth: diluted.",
        "#actor# heard it from someone. #n# people removed from the source.");
    grammar_.add("__obituary__",
        "#name# (#birthYear#-#deathYear#), #occupation#. "
        "Survived by #children# children and #grudges# grudges. "
        "Cause of death: #cause#. Best known for: #topEvent#. "
        "Rumored last words: \"#rumorQuote#\" (unverified).");
}

std::string Narrator::narrate(const EventRecord& e, const History& history,
                              const DynArray<Human>& humans, Rng& narration) const {
    std::map<std::string, std::string> vars;
    HumanId a0 = e.actors.size() > 0 ? e.actors[0] : -1;
    HumanId a1 = e.actors.size() > 1 ? e.actors[1] : -1;
    HumanId a2 = e.actors.size() > 2 ? e.actors[2] : -1;
    vars["actor"] = humanName(humans, a0);
    vars["name"] = vars["actor"];
    vars["victim"] = humanName(humans, a1);
    vars["speaker"] = vars["actor"];
    vars["listener"] = vars["victim"];
    vars["subject"] = humanName(humans, a2);
    vars["leader"] = vars["actor"];
    vars["n"] = std::to_string(e.param);
    vars["votes"] = std::to_string(e.param);
    vars["accuracy"] = std::to_string(e.param) + "%";
    vars["tick"] = std::to_string(e.tick);
    vars["item"] = "food";
    vars["place"] = "(" + std::to_string(e.where.x) + "," + std::to_string(e.where.y) + ")";
    vars["cause"] = deathCause(e.param);
    int age = 0;
    if (a0 >= 0 && (size_t)a0 < humans.size()) age = humans[a0].ageDays / TICKS_PER_YEAR;
    vars["age"] = std::to_string(age);
    int alive = 0;
    for (size_t i = 0; i < humans.size(); ++i) if (humans[i].alive) ++alive;
    vars["survivors"] = std::to_string(alive);
    vars["grudges"] = std::to_string(countEventsOfType(history, a0, EventType::Feud, EventType::Theft));
    vars["names"] = "the rest";
    return grammar_.expand(eventTypeName(e.type), vars, narration);
}

std::string Narrator::obituary(HumanId id, const History& history,
                               const DynArray<Human>& humans, Rng& narration) const {
    std::map<std::string, std::string> vars;
    vars["name"] = humanName(humans, id);
    vars["occupation"] = "citizen";

    EventId birth = -1, death = -1, top = -1, rumor = -1;
    int children = 0, grudges = 0;
    for (EventId eid : history.eventsFor(id)) {
        const EventRecord& e = history.get(eid);
        switch (e.type) {
            case EventType::Birth:
                if (birth == -1) birth = eid; else ++children;
                break;
            case EventType::Death: death = eid; break;
            case EventType::Feud: case EventType::Theft: ++grudges; break;
            case EventType::Discovered: case EventType::Embezzle:
            case EventType::LeaderElected: case EventType::Rescue:
                if (top == -1) top = eid;
                break;
            case EventType::RumorSpread: case EventType::Gossip:
                if (rumor == -1) rumor = eid;
                break;
            default: break;
        }
    }
    int birthYear = birth >= 0 ? (int)(history.get(birth).tick / TICKS_PER_YEAR) : 0;
    int deathYear = death >= 0 ? (int)(history.get(death).tick / TICKS_PER_YEAR) : birthYear;
    vars["birthYear"] = std::to_string(birthYear);
    vars["deathYear"] = std::to_string(deathYear);
    vars["children"] = std::to_string(children);
    vars["grudges"] = std::to_string(grudges);
    vars["cause"] = death >= 0 ? deathCause(history.get(death).param) : "unknown";
    vars["topEvent"] = top >= 0 ? std::string(eventTypeName(history.get(top).type)) : "surviving";
    vars["rumorQuote"] = rumor >= 0 ? narrate(history.get(rumor), history, humans, narration)
                                    : "nothing worth repeating";

    return grammar_.expand("__obituary__", vars, narration);
}

} // namespace tge
