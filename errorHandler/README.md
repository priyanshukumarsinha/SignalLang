# **Error Handler Documentation**

## **Table of Contents**

* [Overview](#overview)
* [Error Handler Structure](#error-handler-structure)
* [Enums & Structs](#enums--structs)
* [Class & API](#class--api)
* [Integration with Symbol Table](#integration-with-symbol-table)
* [Usage Examples](#usage-examples)
* [Testing](#testing)
* [Notes & Best Practices](#notes--best-practices)

---

## **Overview**

The **Error Handler** is a utility module to manage errors during compilation or runtime. It supports different **phases of compilation**, **severity levels**, and can be configured to **stop on fatal errors** or continue after recoverable ones.

Key features:

* Handles **lexical, syntax, semantic, runtime, and generic errors**.
* Differentiates **INFO, WARNING, ERROR, and FATAL** severities.
* Records error **metadata**: message, line, column, recoverable status.
* Can print, summarize, and save errors.
* Easily integrates with **symbol tables** or other compiler modules.

---

## **Error Handler Structure**

| Component       | Type                         | Description                                             |
| --------------- | ---------------------------- | ------------------------------------------------------- |
| `ErrorHandler`  | Class                        | Main class for managing compiler errors.                |
| `CompilerError` | Struct                       | Encapsulates error metadata.                            |
| `ErrorPhase`    | Enum                         | Compilation phase where the error occurred.             |
| `Severity`      | Enum                         | How serious the error is (INFO, WARNING, ERROR, FATAL). |
| `errors`        | `std::vector<CompilerError>` | Stores all collected errors.                            |
| `stopOnFatal`   | `bool`                       | Determines if compilation stops on fatal errors.        |

---

## **Enums & Structs**

### **ErrorPhase**

| Enum Value | Description                                            |
| ---------- | ------------------------------------------------------ |
| `LEXICAL`  | Errors found during tokenization (lexical analysis).   |
| `SYNTAX`   | Errors during parsing (grammar issues).                |
| `SEMANTIC` | Semantic errors (type mismatch, undeclared variables). |
| `RUNTIME`  | Errors that occur during program execution.            |
| `GENERIC`  | General errors not categorized elsewhere.              |

### **Severity**

| Enum Value | Description                                |
| ---------- | ------------------------------------------ |
| `INFO`     | Informational messages (not errors).       |
| `WARNING`  | Warning messages (non-fatal).              |
| `ERROR`    | Regular errors, may be recoverable.        |
| `FATAL`    | Critical issues, usually stop compilation. |

### **CompilerError**

| Field         | Type          | Description                                        |
| ------------- | ------------- | -------------------------------------------------- |
| `phase`       | `ErrorPhase`  | Compilation phase of the error.                    |
| `severity`    | `Severity`    | Severity of the error.                             |
| `message`     | `std::string` | Error description.                                 |
| `line`        | `int`         | Line number (-1 if unknown).                       |
| `column`      | `int`         | Column number (-1 if unknown).                     |
| `recoverable` | `bool`        | True if compilation can continue after this error. |

---

## **Class & API**

### **ErrorHandler Class**

```cpp
class ErrorHandler {
private:
    std::vector<CompilerError> errors;
    bool stopOnFatal;

    static std::string phaseToString(ErrorPhase p);
    static std::string severityToString(Severity s);
    std::string printSingle(const CompilerError &e) const;

public:
    ErrorHandler(); 
    ~ErrorHandler();

    void setStopOnFatal(bool v);
    void report(ErrorPhase p, Severity s, const std::string &message, int line=-1, int col=-1, bool recoverable=true);

    void reportError(ErrorPhase p, const std::string &message, int line=-1, int col=-1, bool recoverable=true);
    void reportWarning(ErrorPhase p, const std::string &message, int line=-1, int col=-1);
    void reportInfo(ErrorPhase p, const std::string &message, int line=-1, int col=-1);
    void reportFatal(ErrorPhase p, const std::string &message, int line=-1, int col=-1);

    size_t errorCount() const;
    size_t warningCount() const;
    bool hasFatal() const;
    std::vector<CompilerError> getAll() const;

    void printSummary() const;
    void clear();
    bool saveToFile(const std::string &filename) const;
};
```

### **API Functions Table**

| Function          | Parameters                                                        | Return Type                  | Description                                |
| ----------------- | ----------------------------------------------------------------- | ---------------------------- | ------------------------------------------ |
| `report()`        | `ErrorPhase`, `Severity`, `message`, `line`, `col`, `recoverable` | void                         | General reporting function.                |
| `reportError()`   | `ErrorPhase`, `message`, `line`, `col`, `recoverable`             | void                         | Reports an error.                          |
| `reportWarning()` | `ErrorPhase`, `message`, `line`, `col`                            | void                         | Reports a warning.                         |
| `reportInfo()`    | `ErrorPhase`, `message`, `line`, `col`                            | void                         | Reports info.                              |
| `reportFatal()`   | `ErrorPhase`, `message`, `line`, `col`                            | void                         | Reports a fatal error.                     |
| `errorCount()`    | None                                                              | size_t                       | Returns number of errors.                  |
| `warningCount()`  | None                                                              | size_t                       | Returns number of warnings.                |
| `hasFatal()`      | None                                                              | bool                         | Checks if at least one fatal error exists. |
| `getAll()`        | None                                                              | `std::vector<CompilerError>` | Returns all errors.                        |
| `printSummary()`  | None                                                              | void                         | Prints all errors and warnings.            |
| `clear()`         | None                                                              | void                         | Clears stored errors.                      |
| `saveToFile()`    | `filename`                                                        | bool                         | Saves errors to a file.                    |

---

## **Integration with Symbol Table**

The Error Handler is often used alongside a **Symbol Table** to validate identifiers, types, or scopes:

```cpp
if (!symbolTable.exists("x")) {
    errorHandler.reportError(ErrorPhase::SEMANTIC, "Variable 'x' not declared", line, col);
}
```

---

## **Usage Examples**

### Example 1: Basic Error Reporting

```cpp
ErrorHandler err;
err.reportError(ErrorPhase::SYNTAX, "Unexpected token ';'", 10, 5);
err.reportWarning(ErrorPhase::SEMANTIC, "Unused variable 'y'", 12, 8);
err.printSummary();
```

### Example 2: Fatal Error Handling

```cpp
err.reportFatal(ErrorPhase::RUNTIME, "Division by zero", 20, 15);
if (err.hasFatal()) {
    std::cout << "Compilation stopped due to fatal error.\n";
}
```

---

## **Testing**

### Sample Test Case

```cpp
#include "ErrorHandler.h"
#include <cassert>

int main() {
    ErrorHandler eh;
    eh.reportError(ErrorPhase::LEXICAL, "Unknown character", 1, 2);
    eh.reportWarning(ErrorPhase::SYNTAX, "Deprecated syntax", 3, 5);
    
    assert(eh.errorCount() == 1);
    assert(eh.warningCount() == 1);

    eh.clear();
    assert(eh.errorCount() == 0);
    assert(eh.warningCount() == 0);

    std::cout << "All tests passed!\n";
}
```

This ensures:

* Errors and warnings are counted correctly.
* `clear()` works.
* Fatal errors can be detected.
* Report messages are correctly stored.

---

## **Notes & Best Practices**

* Always provide **line and column numbers** for easier debugging.
* Use **`setStopOnFatal(true)`** to halt compilation on fatal errors.
* Call `clear()` after handling errors to avoid stale data.
* Save logs to a file for post-mortem debugging using `saveToFile()`.

---




