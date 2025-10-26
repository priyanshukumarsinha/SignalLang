#ifndef ERRORHANDLERTEST_H
#define ERRORHANDLERTEST_H

#include "../errorHandler/errorHandler.h"

class ErrorHandlerTest {
public:
    // Run all test cases
    void runAll();

private:
    void testInfoMessages();
    void testWarningMessages();
    void testErrorMessages();
    void testFatalMessages();
    void testCountsAndQueries();
    void testClearFunctionality();
    void testSaveToFile();
    void testEdgeCases();
};

#endif // ERRORHANDLERTEST_H
