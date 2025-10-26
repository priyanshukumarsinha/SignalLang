# Symbol Table Documentation

## Table of Contents

# Table of Contents

1. [Overview](#overview)
2. [Structure of Symbol Table](#structure-of-symbol-table)
    - [SymbolEntry](#symbolentry)
    - [SymbolTable](#symboltable)
3. [API Reference](#api-reference)
    - [Constructor & Destructor](#constructor--destructor)
    - [Scope Management](#scope-management)
    - [Insertion & Lookup](#insertion--lookup)
    - [Updates and Flags](#updates-and-flags)
    - [Debug / Utilities](#debug--utilities)
4. [Integration with Error Handling](#integration-with-error-handling)
5. [Usage Examples](#usage-examples)
    - [Example 1: Variable Insertion](#example-1-variable-insertion)
    - [Example 2: Token Placeholder and Lookup](#example-2-token-placeholder-and-lookup)
    - [Example 3: Scope and Unused Variables](#example-3-scope-and-unused-variables)
6. [Testing](#testing)
7. [Directory Structure](#directory-structure)


---

## 1. Overview

The **Symbol Table** is a core data structure used in compilers, interpreters, and DSL parsers to **store information about identifiers** (variables, constants, functions, tokens, etc.).

It supports:

* **Nested scopes** (local and global)
* **Insertion and lookup** of symbols
* **Marking symbols as used** and tracking unused ones
* **Error reporting** for duplicates and undeclared identifiers

The implementation uses **C++ STL containers** (`unordered_map` + `vector`) for efficient symbol management.

---

## 2. Structure of Symbol Table

### 2.1 SymbolEntry

Represents a single symbol (variable, constant, function, or token).
**Fields:**

| Field        | Type          | Description                                                                      |
| ------------ | ------------- | -------------------------------------------------------------------------------- |
| `name`       | `std::string` | Identifier name                                                                  |
| `kind`       | `std::string` | Type of symbol: `"variable"`, `"constant"`, `"function"`, `"builtin"`, `"token"` |
| `type`       | `std::string` | Data type: `"int"`, `"float"`, `"bool"`, `"unknown"`, or function signature      |
| `scopeLevel` | `int`         | Scope in which the symbol is declared (`0` = global)                             |
| `memoryAddr` | `std::string` | Memory or stack address assigned to the symbol                                   |
| `value`      | `std::string` | Initial value (if any)                                                           |
| `is_state`   | `bool`        | True if global or state variable                                                 |
| `is_used`    | `bool`        | True if symbol has been referenced                                               |
| `decl_line`  | `int`         | Line number of declaration                                                       |
| `is_dummy`   | `bool`        | True for placeholders (tokens or undeclared dummy symbols)                       |

---

### 2.2 SymbolTable

The main class managing all symbols across **nested scopes**.

**Private Members:**

* `scopes`: `vector<unordered_map<string, SymbolEntry>>` — stack of scopes
* `nextMemoryIndex`: `int` — for generating addresses
* `errHandler`: `ErrorHandler*` — for reporting semantic errors

**Responsibilities:**

* Insert, lookup, and update symbols
* Manage scope stack (`beginScope`, `endScope`)
* Track unused symbols
* Generate memory addresses for variables

---

## 3. API Reference

### 3.1 Constructor & Destructor

```cpp
SymbolTable(ErrorHandler *err = nullptr);
~SymbolTable();
```

* Creates a symbol table and initializes the **global scope**.
* Optional `ErrorHandler` pointer for reporting semantic errors.

---

### 3.2 Scope Management

```cpp
void beginScope();
void endScope();
int currentScopeLevel() const;
```

* `beginScope()`: Push a new scope.
* `endScope()`: Pop the top scope.
* `currentScopeLevel()`: Return the current nesting level.

---

### 3.3 Insertion & Lookup

```cpp
bool insert(const SymbolEntry &entry);
bool insertTokenPlaceholder(const std::string &name, int token_line);
SymbolEntry* lookup(const std::string &name);
SymbolEntry* lookupLocal(const std::string &name);
bool existsInCurrentScope(const std::string &name);
```

* `insert`: Adds a symbol to the current scope. Reports duplicates.
* `insertTokenPlaceholder`: Inserts a dummy token placeholder.
* `lookup`: Search from **innermost to outermost scope**.
* `lookupLocal`: Search **only current scope**.
* `existsInCurrentScope`: Check existence in top scope.

---

### 3.4 Updates and Flags

```cpp
void markUsed(const std::string &name);
bool updateEntry(const std::string &name, const std::function<void(SymbolEntry&)> &updater);
std::vector<SymbolEntry> getUnusedEntries() const;
```

* `markUsed`: Marks a symbol as used; creates a dummy if undeclared.
* `updateEntry`: Modify a symbol with a lambda function.
* `getUnusedEntries`: Returns a list of symbols that were declared but never used.

---

### 3.5 Debug / Utilities

```cpp
void dump() const;
void clear();
```

* `dump()`: Print the contents of all scopes.
* `clear()`: Remove all symbols and reset memory indexing.

---

## 4. Integration with Error Handling

The `SymbolTable` works with an optional `ErrorHandler` to report:

* **Duplicate declarations**: `insert` triggers `reportDuplicate`.
* **Undeclared variables**: `markUsed` triggers `reportError`.
* **Fallback**: If no `ErrorHandler`, errors/warnings are printed to `stderr`.

---

## 5. Usage Examples

### 5.1 Example 1: Variable Insertion

```cpp
SymbolTable st;
st.insert(SymbolEntry("x", "variable", "int", 0, 3));
SymbolEntry* x = st.lookup("x");
std::cout << x->name << " has type " << x->type << "\n";
```

**Explanation:**

* Inserts `x` into global scope.
* Retrieves `x` and prints its type.

---

### 5.2 Example 2: Token Placeholder and Lookup

```cpp
st.insertTokenPlaceholder("TOKEN_A", 5);
SymbolEntry* token = st.lookup("TOKEN_A");
std::cout << token->kind << " at line " << token->decl_line << "\n";
```

**Explanation:**

* Creates a **dummy token**.
* Lookup confirms existence with correct type and line.

---

### 5.3 Example 3: Scope and Unused Variables

```cpp
st.beginScope();
st.insert(SymbolEntry("y", "variable", "float"));
st.markUsed("y");

auto unused = st.getUnusedEntries();
for (auto &e : unused) {
    std::cout << "Unused: " << e.name << "\n";
}
st.endScope();
```

**Explanation:**

* `y` is inserted in a local scope.
* Marked as used → will **not appear in unused list**.
* `endScope()` removes the local scope.

---

## 6. Testing

* Use **SymbolTableTest** (provided earlier) to validate all API functions.
* Tests include:

  * Scope management
  * Insertions & lookups
  * Token placeholders
  * Updates and marking used
  * Unused variable tracking
  * Dump and clear functionality

---

## 7. Directory Structure

```
AutoLang/
├── symbolTable/
│   ├── symbolTable.h
│   ├── symbolTable.cpp
├── errorHandler/
│   ├── errorHandler.h
│   ├── errorHandler.cpp
├── Tests/
│   ├── symbolTableTest.h
│   ├── symbolTableTest.cpp
│   ├── errorHandlerTest.h
│   ├── errorHandlerTest.cpp
│── main.cpp      # to run tests
```



