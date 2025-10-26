#include "symbolTableTest.h"

// Run all test cases
void SymbolTableTest::runAll() {
    testBeginAndEndScope();
    testInsertAndLookup();
    testInsertTokenPlaceholder();
    testExistsInCurrentScope();
    testLookupLocal();
    testMarkUsed();
    testUpdateEntry();
    testGetUnusedEntries();
    testDumpAndClear();
    std::cout << "All SymbolTable tests completed.\n";
}

// -------------------------
// Scope Management Tests
// -------------------------
void SymbolTableTest::testBeginAndEndScope() {
    SymbolTable st;

    int initialLevel = st.currentScope();
    st.beginScope();
    assertTrue(st.currentScope() == initialLevel + 1, "beginScope increases level");

    st.endScope();
    assertTrue(st.currentScope() == initialLevel, "endScope decreases level");
}

// -------------------------
// Insertion & Lookup Tests
// -------------------------
void SymbolTableTest::testInsertAndLookup() {
    SymbolTable st;

    SymbolEntry var("x", "variable", "int");
    bool inserted = st.insert(var);
    assertTrue(inserted, "insert variable x succeeds");

    SymbolEntry* found = st.lookup("x");
    assertTrue(found != nullptr, "lookup finds x");
    assertEqual(found->type, "int", "lookup type of x is int");

    // Duplicate insertion should fail
    inserted = st.insert(var);
    assertFalse(inserted, "duplicate insert fails");
}

void SymbolTableTest::testInsertTokenPlaceholder() {
    SymbolTable st;
    bool inserted = st.insertTokenPlaceholder("TOKEN_A", 5);
    assertTrue(inserted, "insertTokenPlaceholder succeeds");

    SymbolEntry* token = st.lookup("TOKEN_A");
    assertTrue(token != nullptr && token->is_dummy, "lookup finds dummy token");
    assertEqual(token->kind, "token", "token kind is 'token'");
    assertTrue(token->decl_line == 5, "token decl_line is correct");
}

void SymbolTableTest::testExistsInCurrentScope() {
    SymbolTable st;
    st.insert(SymbolEntry("y", "variable", "float"));
    assertTrue(st.existsInCurrentScope("y"), "existsInCurrentScope finds y");
    assertFalse(st.existsInCurrentScope("z"), "existsInCurrentScope false for z");
}

void SymbolTableTest::testLookupLocal() {
    SymbolTable st;
    st.insert(SymbolEntry("a", "variable", "int"));
    st.beginScope();
    st.insert(SymbolEntry("b", "variable", "float"));

    SymbolEntry* localA = st.lookupLocal("a");
    SymbolEntry* localB = st.lookupLocal("b");

    assertTrue(localA == nullptr, "lookupLocal a not found in inner scope");
    assertTrue(localB != nullptr, "lookupLocal b found in current scope");
}

// -------------------------
// Updates and Flags Tests
// -------------------------
void SymbolTableTest::testMarkUsed() {
    SymbolTable st;
    st.insert(SymbolEntry("usedVar", "variable", "int"));
    st.markUsed("usedVar");

    SymbolEntry* e = st.lookup("usedVar");
    assertTrue(e->is_used, "markUsed sets is_used");

    // Mark undeclared variable → creates dummy
    st.markUsed("undeclaredVar");
    SymbolEntry* dummy = st.lookup("undeclaredVar");
    assertTrue(dummy != nullptr && dummy->is_dummy, "markUsed creates dummy for undeclared");
}

void SymbolTableTest::testUpdateEntry() {
    SymbolTable st;
    st.insert(SymbolEntry("num", "variable", "int"));

    bool updated = st.updateEntry("num", [](SymbolEntry &e){ e.type = "float"; });
    assertTrue(updated, "updateEntry updates existing symbol");

    SymbolEntry* e = st.lookup("num");
    assertEqual(e->type, "float", "updateEntry changes type");

    // Update undeclared variable → inserts dummy first
    updated = st.updateEntry("newVar", [](SymbolEntry &e){ e.value = "42"; });
    assertTrue(updated, "updateEntry inserts undeclared variable");
    SymbolEntry* newVar = st.lookup("newVar");
    assertTrue(newVar != nullptr && newVar->value == "42", "updateEntry sets value on newVar");
}

void SymbolTableTest::testGetUnusedEntries() {
    SymbolTable st;
    st.insert(SymbolEntry("usedVar", "variable", "int"));
    st.insert(SymbolEntry("unusedVar", "variable", "int"));

    st.markUsed("usedVar");

    std::vector<SymbolEntry> unused = st.getUnusedEntries();
    assertTrue(unused.size() == 1 && unused[0].name == "unusedVar", "getUnusedEntries finds unusedVar");
}

// -------------------------
// Debug / Utilities Tests
// -------------------------
void SymbolTableTest::testDumpAndClear() {
    SymbolTable st;
    st.insert(SymbolEntry("x", "variable", "int"));
    st.insertTokenPlaceholder("TOKEN_X", 1);

    // Just call dump() to ensure no crash (manual check)
    st.dump();

    st.clear();
    assertTrue(st.lookup("x") == nullptr && st.lookup("TOKEN_X") == nullptr, "clear removes all symbols");
}

// -------------------------
// Helper Assertion Functions
// -------------------------
void SymbolTableTest::assertTrue(bool condition, const std::string& testName) {
    if (condition)
        std::cout << "[PASS] " << testName << "\n";
    else
        std::cout << "[FAIL] " << testName << "\n";
}

void SymbolTableTest::assertFalse(bool condition, const std::string& testName) {
    assertTrue(!condition, testName);
}

void SymbolTableTest::assertEqual(const std::string& a, const std::string& b, const std::string& testName) {
    assertTrue(a == b, testName + " (expected '" + b + "', got '" + a + "')");
}
