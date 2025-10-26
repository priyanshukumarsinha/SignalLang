#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <functional>
#include "../errorHandler/errorHandler.h"
#include <unordered_map>


// this is the structure of a single symbol
// i.e how a single symbol is represented
struct SymbolEntry{
    std::string name; // Name of the identifier (i.e variable name, function name, constant, etc)

    // TODO: change this to TokenKind when defining Lexer
    std::string kind; // Type of symbol: "Variable", "Constant", "Function", "Builtin", "Token"

    // TODO: change this to DataType when defining Parser
    std::string type; // Data type : "int", "float", "bool", "unknown", or function signature for functions

    int scopeLevel = 0; // Scope in which the symbol is declared (0 = global)

    std::string memoryAddr; // Placeholder for memory location or register

    std::string value; // initial value assigned to the variable (if any)

    bool is_state = false; // true if the symbol represents a state variable (eg global state)
    // a state variable can be used in that scope until the scope ends
    // but a var variable can only be used until out()

    bool is_used = false; // true if symbol has been referenced in the code
    // Used to find out unused variable to give warnings

    int decl_line = -1; // Line number where the symbol was declared
    // Helps with error messages

    bool is_dummy = false; // True for placeholder entries (like tokens or temporary symbols)

    // Default Constructor
    SymbolEntry() = default;

    // Parameterized Constructor
    SymbolEntry(const std::string &n, const std::string &k, const std::string &t, int s = 0, int line = -1) : name(n), kind(k), type(t), scopeLevel(s), decl_line(line){}
};



// Now the Symbol Table
// The symbol table contains all the symbols
// i.e the SymbolEntry struct we used earlier
class SymbolTable{
private:
    // Vector of hash maps representing nested scopes
    // Each unordered_map stores symbols of the corresponding scope
    std::vector<std::unordered_map<std::string, SymbolEntry>> scopes;

    int nextMemoryIndex;  // Counter for generating unique memory addresses
    ErrorHandler *errHandler; // Pointer to error handler for reporting semantic errors

    // Helper Function: convert integer to hexadecimal string
    static std::string to_hex(int x);
    void reportDuplicate(const SymbolEntry &existing, const SymbolEntry &attempt);
    

public:
    // optionally receives an ErrorHandler pointer for reporting issues
    SymbolTable(ErrorHandler *err = nullptr);

    // Destructor : clears symbol table memory
    ~SymbolTable();

    // Scope Management
    // 1. begin a scope (push a new map of symbols)
    void beginScope();
    
    // 2. End the current scope (pop the top map)
    void endScope();

    // 3. get the current scope level
    int currentScope() const;

    // Now when declaring varibles
    // 1. we need to lookup the symbol table in all the scopes
    // 2. we need to insert the symbol in the currentscope

    bool insert(const SymbolEntry &entry); // returns false if symbol already exists in current scope

    // Insert a placeholder for a token with its declaration line
    bool insertTokenPlaceholder(const std::string &name, int token_line);

    // Lookup a symbol from the innermost to outermost scope
    // Returns pointer to SymbolEntry or nullptr if not found
    SymbolEntry* lookup(const std::string &name);

    // Lookup a symbol only in top (current) scope
    SymbolEntry* lookupLocal(const std::string &name);

    // check if a symbol exists in the current scope
    // checks using lookupLocal
    bool existsInCurrentScope(const std::string &name);

    // UPDATES AND FLAGS
    
    // Mark a symbol as used 
    // i.e during declaration or other operations
    void markUsed(const std::string &name);

    // Update a symbol's entry
    // Returns true if the symbol exists and is updated
    bool updateEntry(const std::string &name, const std::function<void(SymbolEntry&)> &updater);

    // To retrieve all symbols in all scopes that we declared but never used
    std::vector<SymbolEntry> getUnusedEntries() const;

    // Utility Functions

    // print the entire symbol table
    void dump() const;

    // Clear all scopes and reset memory index
    void clear();

};

#endif // SYMBOL_TABLE_H