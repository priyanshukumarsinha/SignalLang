#include "errorHandler.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// Default Constructor
// error vector is already initialized as a vector
// stopOnFatal = true by default
ErrorHandler::ErrorHandler(): stopOnFatal(true){}

ErrorHandler::~ErrorHandler() = default;

// stop on Fatal setter
void ErrorHandler::setStopOnFatal(bool v){
    stopOnFatal = v ;
}

void ErrorHandler::report(ErrorPhase p, Severity s, const std::string &message, int line, int col, bool recoverable){
    // this is the general method used by all report functions 
    
    // first step is to create a CompilerError
    CompilerError err(p, s, message, line, col, recoverable);

    errors.push_back(err);

    // print the error immediately
    printSingle(err);
    
    // if the error is not recoverable exit
    // for this to execute i.e for exiting
    // stopOnFatal should be ON
    // it should be a FATAL error and !recoverable
    if(!recoverable && s == Severity::FATAL && stopOnFatal){
        throw runtime_error("Fatal compilation error encountered. Stopping");
    }
}

void ErrorHandler::reportError(ErrorPhase p, const std::string &message, int line, int col, bool recoverable){
    
    // Severity is given directly here
    // CompilerError err(p, Severity::ERROR, message, line, col, recoverable);

    report(p, Severity::ERROR, message, line, col, recoverable);
}

void ErrorHandler::reportWarning(ErrorPhase p, const std::string &message, int line, int col){
    
    // Severity is given directly here
    // CompilerError err(p, Severity::ERROR, message, line, col, recoverable);

    report(p, Severity::WARNING, message, line, col, true);
}

void ErrorHandler::reportInfo(ErrorPhase p, const std::string &message, int line, int col){
    
    // Severity is given directly here
    // CompilerError err(p, Severity::ERROR, message, line, col, recoverable);

    report(p, Severity::INFO, message, line, col, true);
}

void ErrorHandler::reportFatal(ErrorPhase p, const std::string &message, int line, int col){
    
    // Severity is given directly here
    // CompilerError err(p, Severity::ERROR, message, line, col, recoverable);

    report(p, Severity::FATAL, message, line, col, false);
}

size_t ErrorHandler::errorCount() const {
    size_t c=0;
    for(auto &e:errors){
        if(e.severity==Severity::FATAL || e.severity==Severity::ERROR){
            ++c;
        }
    }
    return c;
}

size_t ErrorHandler::warningCount() const {
    size_t c=0;
    for(auto &e:errors){
        if(e.severity==Severity::WARNING){
            ++c;
        }
    }
    return c;
}

bool ErrorHandler::hasFatal() const {
    for(auto &e:errors){
        if(e.severity==Severity::FATAL){
            return true;
        }
    }
    return false;
}

std::vector<CompilerError> ErrorHandler::getAll() const{
    return errors;
}

void ErrorHandler::printSummary() const{
    if(errors.empty()){
        cout << "No errors or warning \n";
    }

    cout << "=== Compiler Messages (" << errors.size() << ") ===\n";

    for(const auto& e:errors){
        cout << printSingle(e);
    }

    cout << "=== END OF MESSAGES ===\n";
}

void ErrorHandler::clear(){
    errors.clear();
}

bool ErrorHandler::saveToFile(const std::string &filename) const{
    ofstream out(filename);
    if(!out) return false; //could not open file

    if(errors.empty()){
        out << "No errors or warning \n";
        return true;
    }

    out << "=== Compiler Messages (" << errors.size() << ") ===\n";

    for(const auto& e:errors){
        out << printSingle(e);
    }

    out << "=== END OF MESSAGES ===\n";
    return true;

}

string ErrorHandler::phaseToString(ErrorPhase p){
    switch(p){
        case ErrorPhase::SYNTAX: return "Syntax Error";
        case ErrorPhase::LEXICAL: return "Lexical Error";
        case ErrorPhase::SEMANTIC: return "Semantic Error";
        case ErrorPhase::RUNTIME: return "Runtime Error";           
    }
    return "Generic Error";
}

string ErrorHandler::severityToString(Severity s){
    switch(s){
        case Severity::INFO: return "INFO";
        case Severity::WARNING: return "WARNING";
        case Severity::ERROR: return "ERROR";
        case Severity::FATAL: return "FATAL";
    }

    // Its unknown (unreachable mostly)
    return "UNKNOWN";
}

std::string ErrorHandler::printSingle(const CompilerError &e) const{
    std::ostringstream oss;
    oss << "[" << phaseToString(e.phase) << "]" << severityToString(e.severity);
    if(e.line >= 0){
        oss << "(line " << e.line;
        if(e.column >= 0){
            oss << ", col = " << e.column;
        }
        oss << ")";
    }

    oss << ": "<< e.message;

    if(!e.recoverable) oss << "[NON-RECOVERABLE]";
    oss << "\n";

    return oss.str();
}

