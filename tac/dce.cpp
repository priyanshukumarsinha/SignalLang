#include "dce.h"
#include <unordered_set>
#include <string>
#include <iostream>

using namespace std;

static inline vector<string> usesOf(const TacInst &i) {
    vector<string> res;
    switch (i.op) {
        case TACOp::LOAD_CONST:
            // no uses
            break;
        case TACOp::ASSIGN:
            if (!i.arg1.empty()) res.push_back(i.arg1);
            break;
        case TACOp::ADD:
        case TACOp::SUB:
        case TACOp::MUL:
        case TACOp::DIV:
            if (!i.arg1.empty()) res.push_back(i.arg1);
            if (!i.arg2.empty()) res.push_back(i.arg2);
            break;
        default:
            break;
    }
    return res;
}

void DeadCodeEliminator::eliminate(vector<TacInst> &tac, const SymbolTable &sym) {
    unordered_set<string> live;
    // initialize live set with variables that are externally used (sym.is_used)
    auto unused = sym.getUnusedEntries(); // but we need used ones
    // The SymbolTable doesn't expose getUsedEntries; we will iterate through scopes directly by duplicating minimal logic.
    // But because SymbolTable is not const-accessible for internals we instead rely on getUnusedEntries only to derive used set:
    // We'll mark everything referenced in the symbol table as used except returned unused. Simpler approach:
    // For safety, initialize live with all symbol names that are marked used via lookup of scopes (we can't due to interface).
    // So we'll instead trust symbol table's getUnusedEntries and treat everything NOT in that list as used.
    // Build set of declared names from union of defs in TAC too.
    unordered_set<string> declared;
    for (const auto &inst : tac) {
        if (!inst.dest.empty() && inst.dest[0] != 't') declared.insert(inst.dest);
    }
    // Build set of unused from symbol table
    unordered_set<string> unusedSet;
    for (auto &e : sym.getUnusedEntries()) {
        unusedSet.insert(e.name);
    }
    // Consider declared variables that are not in unusedSet as live
    for (const string &name : declared) {
        if (unusedSet.find(name) == unusedSet.end()) {
            live.insert(name);
        }
    }

    // Backward traversal
    vector<char> keep(tac.size(), 0);
    for (int i = (int)tac.size()-1; i >= 0; --i) {
        const TacInst &inst = tac[i];
        bool hasSideEffect = false; // our instructions are pure
        const string &def = inst.dest;
        if (!def.empty() && live.find(def) != live.end()) {
            // definition is needed -> keep and add uses
            keep[i] = 1;
            for (auto &u : usesOf(inst)) {
                // if this use is a temp like tX, add it; or variable
                if (!u.empty()) live.insert(u);
            }
        } else {
            // def not live -> remove (keep[i]=0)
            // But if instruction has no dest (shouldn't), or is LOAD_CONST assigned to temp unused -> drop
            // No action
        }
    }

    // Reconstruct new tac vector
    vector<TacInst> out;
    out.reserve(tac.size());
    for (size_t i = 0; i < tac.size(); ++i) if (keep[i]) out.push_back(tac[i]);
    tac.swap(out);
}
