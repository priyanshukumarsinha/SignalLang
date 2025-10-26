#include <iostream>
#include <cassert>
#include "errorHandlerTest.h"

using namespace std;

void ErrorHandlerTest::runAll(){
    testInfoMessages();
    testWarningMessages();
    testErrorMessages();
    testFatalMessages();
    testCountsAndQueries();
    testClearFunctionality();
    testSaveToFile();
    testEdgeCases();
    cout << "All ErrorHandler tests passed successfully!\n";
}

void ErrorHandlerTest::testInfoMessages()
{
    ErrorHandler h;
    h.reportInfo(ErrorPhase::LEXICAL, "Lexer initialized.");
    h.reportInfo(ErrorPhase::SYNTAX, "Parsing started.", 0, 0); // edge: line/col zero
    auto all = h.getAll();
    assert(all.size() == 2);
    for (auto &e : all)
        assert(e.severity == Severity::INFO);
}

// -----------------------
// Test WARNING messages
// -----------------------
void ErrorHandlerTest::testWarningMessages()
{
    ErrorHandler h;
    h.reportWarning(ErrorPhase::SYNTAX, "Unused variable.", 12, 5);
    h.reportWarning(ErrorPhase::SEMANTIC, "Deprecated function used.", -1, -1); // unknown line/col
    auto all = h.getAll();
    assert(all.size() == 2);
    assert(all[0].severity == Severity::WARNING);
    assert(all[1].severity == Severity::WARNING);
    assert(all[0].line == 12 && all[0].column == 5);
    assert(all[1].line == -1 && all[1].column == -1);
}

// -----------------------
// Test ERROR messages
// -----------------------
void ErrorHandlerTest::testErrorMessages()
{
    ErrorHandler h;
    h.reportError(ErrorPhase::SEMANTIC, "Type mismatch.", 20, 10);
    h.reportError(ErrorPhase::SEMANTIC, "", 30, 0); // empty message
    assert(h.errorCount() == 2);
    auto all = h.getAll();
    assert(all[0].severity == Severity::ERROR);
    assert(all[1].message.empty());
}

// -----------------------
// Test FATAL messages
// -----------------------
void ErrorHandlerTest::testFatalMessages()
{
    ErrorHandler h;
    h.setStopOnFatal(false); // prevent stopping test
    h.reportFatal(ErrorPhase::RUNTIME, "Segmentation fault.", 42, 15);
    auto all = h.getAll();
    assert(all[0].severity == Severity::FATAL);
    assert(h.hasFatal() == true);
}

// -----------------------
// Test counts and query functions
// -----------------------
void ErrorHandlerTest::testCountsAndQueries()
{
    ErrorHandler h;
    h.reportError(ErrorPhase::SEMANTIC, "E1");
    h.reportWarning(ErrorPhase::SYNTAX, "W1");
    h.reportInfo(ErrorPhase::LEXICAL, "I1");
    h.setStopOnFatal(false);
    h.reportFatal(ErrorPhase::RUNTIME, "F1", 1, 1);
    assert(h.errorCount() == 2); // ERROR + FATAL
    assert(h.warningCount() == 1);
    assert(h.hasFatal() == true);
    auto all = h.getAll();
    assert(all.size() == 4);
}

// -----------------------
// Test clear function
// -----------------------
void ErrorHandlerTest::testClearFunctionality()
{
    ErrorHandler h;
    h.reportError(ErrorPhase::SEMANTIC, "E1");
    assert(!h.getAll().empty());
    h.clear();
    assert(h.getAll().empty());
    assert(h.errorCount() == 0);
    assert(h.warningCount() == 0);
}

// -----------------------
// Test saveToFile function
// -----------------------
void ErrorHandlerTest::testSaveToFile()
{
    ErrorHandler h;
    h.reportError(ErrorPhase::SEMANTIC, "E1");
    h.reportWarning(ErrorPhase::SYNTAX, "W1");
    bool saved = h.saveToFile("test_errors.txt");
    assert(saved == true);
}

// -----------------------
// Edge cases
// -----------------------
void ErrorHandlerTest::testEdgeCases()
{
    ErrorHandler h;

    // Very long message
    string longMsg(1000, 'x');
    h.reportError(ErrorPhase::SEMANTIC, longMsg);
    assert(h.getAll().back().message.size() == 1000);

    // Negative line/column
    h.reportWarning(ErrorPhase::LEXICAL, "Negative line/col", -5, -10);
    auto last = h.getAll().back();
    assert(last.line == -5 && last.column == -10);

    // Multiple non-recoverable fatal errors
    h.setStopOnFatal(false);
    h.reportFatal(ErrorPhase::RUNTIME, "Fatal1");
    h.reportFatal(ErrorPhase::RUNTIME, "Fatal2");
    auto all = h.getAll();
    size_t fatalCount = 0;
    for (auto &e : all)
        if (e.severity == Severity::FATAL)
            ++fatalCount;
    assert(fatalCount == 2);

    // Empty message
    h.reportInfo(ErrorPhase::LEXICAL, "");
    assert(h.getAll().back().message.empty());
}
