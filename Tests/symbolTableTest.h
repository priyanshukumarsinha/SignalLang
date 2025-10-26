#ifndef SYMBOLTABLETEST_H
#define SYMBOLTABLETEST_H

#include "../symbolTable/symbolTable.h"
#include <iostream>

class SymbolTableTest {
public:
    // Run all test cases for SymbolTable
    void runAll();

private:
    // Scope Management Tests
    void testBeginAndEndScope();

    // Insertion & Lookup Tests
    void testInsertAndLookup();
    void testInsertTokenPlaceholder();
    void testExistsInCurrentScope();
    void testLookupLocal();

    // Updates and Flags Tests
    void testMarkUsed();
    void testUpdateEntry();
    void testGetUnusedEntries();

    // Debug / Utilities Tests
    void testDumpAndClear();

    // Helper functions to show test results
    void assertTrue(bool condition, const std::string& testName);
    void assertFalse(bool condition, const std::string& testName);
    void assertEqual(const std::string& a, const std::string& b, const std::string& testName);
};

#endif // SYMBOLTABLETEST_H
