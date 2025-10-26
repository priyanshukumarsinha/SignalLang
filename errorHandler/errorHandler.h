#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include <vector>

// Error can arise in various phases
enum class ErrorPhase{
    LEXICAL, // Erros found during lexical analysis (tokenization)
    SYNTAX, // Error found during parsing (grammar issues)
    SEMANTIC, // Error found during semantic analysis (type errors, undeclared variables, etc)
    RUNTIME, // Errors that occur during program execution 
    GENERIC // General Error that do not fit any of the above phases
};

// Enum representing the severity level of an error
// the severity level represents how serious and error is 
enum class Severity{
    INFO, // Informational messages (not an actual error)
    WARNING, // Warning messages (not fatal)
    ERROR, // Regular issues that may prevent compilation
    FATAL // Critical issues that usually stop compilation
};

// so to represent all details about the error
// we need a encapsulated structure
struct CompilerError{
    ErrorPhase phase; //phase of compilation where the error occurred
    Severity severity; //how important the error is
    std::string message; // the error message
    int line; // line at which error occurred (-1 if unknownn)
    int column; // col at which error occurred (-1 if unknownn)
    bool recoverable; // indicates if the compiler can continue after this error

    // Default Constructor
    CompilerError() = default;

    // Parameterized Constructor to initialize all fields
    CompilerError(ErrorPhase p, Severity s, const std::string &msg, int l = -1, int c = -1, bool r = true) : phase(p), severity(s), message(msg), line(l), column(c), recoverable(r) {};
};

class ErrorHandler{
private:
    std::vector<CompilerError> errors;
    bool stopOnFatal; 

    // Helper functions
    static std::string phaseToString(ErrorPhase p);
    static std::string severityToString(Severity s);

    // prints an error in human readable format
    std::string printSingle(const CompilerError &e) const;

public:
    // default constructor
    ErrorHandler(); // Constructor
    ~ErrorHandler(); // Destructor

    // first lets set if the compiler stops on fatal errors
    // for that we need to change stopOnFatal
    void setStopOnFatal(bool v);

    // General reporting function (can report any severity)
    void report(ErrorPhase p, Severity s, const std::string &message, int line=-1, int col=-1, bool recoverable = true);

    // report Error as per severity
    // Error : Severity = ERROR => could be recoverable or non recoverable
    void reportError(ErrorPhase p, const std::string &message, int line=-1, int col=-1, bool recoverable = true);
    // Warning : Severity = WARNING => No need to recover
    void reportWarning(ErrorPhase p, const std::string &message, int line=-1, int col=-1);
    // Info : Severity = INFO => No need to recover
    void reportInfo(ErrorPhase p, const std::string &message, int line=-1, int col=-1);
    // Fatal : Severity = FATAL => Not recoverable
    void reportFatal(ErrorPhase p, const std::string &message, int line=-1, int col=-1);

    // Query Functions to get metadata, info about errors collected
    size_t errorCount() const; //returns the number of errors
    size_t warningCount() const; // number of warnings
    bool hasFatal() const; // returns true if atleast one error is fatal
    std::vector<CompilerError> getAll() const;

    // Functions to display or print errors
    void printSummary() const; // print all errors and warning to console
    void clear(); //clears all stored errors

    // [LOG]
    bool saveToFile(const std::string &filename) const; //saves errors to a file
};




#endif // ERROR_HANDLER_H