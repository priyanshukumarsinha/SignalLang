#include "symbolTable.h"
#include <iostream>
#include <sstream>

using namespace std;

// Constructor
SymbolTable::SymbolTable(ErrorHandler *err){
    errHandler = err;
    nextMemoryIndex = 0;
    beginScope();
}

// Destructor
SymbolTable::~SymbolTable() = default;

void SymbolTable::beginScope(){
    // we push a new hash map onto the stack
    scopes.emplace_back();
}

void SymbolTable::endScope(){
    if(!scopes.empty()) scopes.pop_back();
}

int SymbolTable::currentScope() const{
    return (static_cast<int>(scopes.size()-1));
}

bool SymbolTable::insert(const SymbolEntry &entry){
    if(scopes.empty()) return false;

    auto &table = scopes.back();
    auto it = table.find(entry.name);

    if(it != table.end()){
        // it means the symbol is already declared
        reportDuplicate(it->second, entry);
        return false;
    }

    // Else copy entry and change its scope level
    SymbolEntry e = entry;
    e.scopeLevel = currentScope();

    if (e.memoryAddr.empty()) {
        if (e.kind == "variable" && e.is_state) {
            // State/global variable
            e.memoryAddr = "0x" + to_hex(0x1000 + nextMemoryIndex++);
        } else if (e.kind == "variable" && e.scopeLevel == 0) {
            // Global variable
            e.memoryAddr = "0x" + to_hex(0x1000 + nextMemoryIndex++);
        } else {
            // Local variable or non-state → stack-based address
            e.memoryAddr = "stk" + to_string(nextMemoryIndex++);
        }
    }

    // lastly add the symbol to the current scope
    table[e.name] = e;
    return true;
}

// FAKE ADDRESS GENERATOR
// Just for representation
string SymbolTable::to_hex(int x){
    std::ostringstream oss;
    oss << std::hex << std::nouppercase << x;
    return oss.str();
}

void SymbolTable::reportDuplicate(const SymbolEntry &existing, const SymbolEntry &attempt){
    if (errHandler) {
        // Use the ErrorHandler to report a semantic error
        errHandler->reportError(
            ErrorPhase::SEMANTIC,
            "Duplicate declaration of '" + attempt.name + "'; previously declared at line " +
            to_string(existing.decl_line),
            attempt.decl_line
        );
    } else {
        // Fallback to stderr if no ErrorHandler is provided
        cerr << "Error: Duplicate declaration of '" << attempt.name 
             << "'; previously declared at line " << existing.decl_line
             << " (current decl line " << attempt.decl_line << ")\n";
    }
}


// Insert a placeholder for a token (dummy symbol)
bool SymbolTable::insertTokenPlaceholder(const string &name, int token_line) {
    if (existsInCurrentScope(name)) return false;

    // Create a dummy SymbolEntry representing the token
    SymbolEntry e(name, "token", "unknown", currentScope(), token_line);
    e.is_dummy = true;
    e.decl_line = token_line;

    // Insert into current scope
    return insert(e);
}

SymbolEntry* SymbolTable::lookup(const string &name) {
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return &it->second; // return pointer to symbol entry if found
        }
    }
    return nullptr; // not found
}

// Lookup a symbol only in the current scope
SymbolEntry* SymbolTable::lookupLocal(const string &name) {
    if (scopes.empty()) return nullptr;
    auto it = scopes.back().find(name);
    if (it == scopes.back().end()) return nullptr;
    return &it->second;
}

bool SymbolTable::existsInCurrentScope(const std::string &name){
    SymbolEntry * symtab = lookupLocal(name);
    if(symtab){
        return true;
    }    
    return false;
}

void SymbolTable::markUsed(const std::string &name){
    SymbolEntry * entry = lookup(name);
    if(entry){
        // if the variable is found
        // mark it used
        entry->is_used = true;
    }
    else{
        // if not found
        // then reportError
        errHandler->reportError(ErrorPhase::SEMANTIC, "Undeclared Identifier '" + name + "' used");
    
        // if the symbol is undeclared
        // reportError and create a dummy
        SymbolEntry d(name, "variable", "unknown", 0, -1);
        d.is_dummy = true;
        d.is_used = true;
        if (!scopes.empty()) scopes.front()[name] = d; // insert dummy in global scope
    }
}

// Update a symbol entry with a lambda updater
bool SymbolTable::updateEntry(const string &name, const function<void(SymbolEntry&)> &updater) {
    SymbolEntry *e = lookupLocal(name);
    if (e == nullptr) {
        // Try inserting a default variable if not present
        SymbolEntry entry(name, "variable", "unknown", currentScope(), -1);
        bool ok = insert(entry);
        if (!ok) return false;
        e = lookupLocal(name);
        if (e == nullptr) return false;
    }
    updater(*e); // apply lambda to update symbol
    return true;
}


std::vector<SymbolEntry> SymbolTable::getUnusedEntries() const{
    vector<SymbolEntry> res;
    for(int i= static_cast<int>(scopes.size()-1); i>=0; i--){
        for(const auto &p:scopes[i]){
            if(!p.second.is_used) res.push_back(p.second);
        }
    }
    return res;
}

void SymbolTable::dump() const{
    cout << "=== Symbol Table Dump ===\n";
    for (size_t level = 0; level < scopes.size(); ++level) {
        cout << "Scope level " << level << ":\n";
        for (const auto &p : scopes[level]) {
            const SymbolEntry &e = p.second;
            cout << "  name='" << e.name << "' kind='" << e.kind << "' type='" << e.type
                 << "' addr='" << e.memoryAddr << "' scope=" << e.scopeLevel
                 << " decl_line=" << e.decl_line
                 << " is_state=" << (e.is_state ? "yes" : "no")
                 << " is_used=" << (e.is_used ? "yes" : "no")
                 << (e.is_dummy ? " [DUMMY]" : "");
            if (!e.value.empty()) cout << " value='" << e.value << "'";
            cout << "\n";
        }
    }
    cout << "=========================\n";
}

void SymbolTable::clear(){
    scopes.clear();
    nextMemoryIndex = 0;
    beginScope();
}