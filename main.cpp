#include <iostream>
#include <iomanip>
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "symbolTable/symbolTable.h"
#include "errorHandler/errorHandler.h"

using namespace std;

// Converts enum TokenKind to readable string
std::string tokenKindToString(TokenKind kind) {
    switch (kind) {
        case TokenKind::IDENT:       return "IDENT";
        case TokenKind::FLOAT_LIT:   return "FLOAT_LIT";
        case TokenKind::SEMICOLON:   return "SEMICOLON";
        case TokenKind::PLUS:        return "PLUS";
        case TokenKind::MINUS:       return "MINUS";
        case TokenKind::STAR:        return "STAR";
        case TokenKind::SLASH:       return "SLASH";
        case TokenKind::ASSIGN:      return "ASSIGN";
        case TokenKind::END_OF_FILE: return "EOF";
        default:                     return "UNKNOWN";
    }
}


int main() {
    cout << "=== SignalLang Lexer Test ===\n\n";

    // Example source code for SignalLang
    string source = R"(
result = signal1 * 3.14 + temp;
temp = result / 2.0 - signal1;
output = temp * temp;
)";

    // Create ErrorHandler and SymbolTable
    ErrorHandler err;
    SymbolTable sym(&err);

    // Create Lexer (linked to symbol table & error handler)
    Lexer lexer(&sym, &err);
    lexer.setSource(source);

    cout << "Lexing input...\n\n";

    // Print header
    cout << left << setw(12) << "TOKEN" 
         << setw(15) << "LEXEME"
         << setw(8)  << "LINE"
         << setw(8)  << "COL" << "\n";
    cout << string(45, '-') << "\n";

    // Lex tokens one by one using getNextToken()
    while (true) {
        Token t = lexer.getNextToken();
        cout << left << setw(12) << tokenKindToString(t.kind)
             << setw(15) << t.lexeme
             << setw(8)  << t.line
             << setw(8)  << t.column
             << "\n";

        if (t.kind == TokenKind::END_OF_FILE) break;
    }

    cout << "\n=== Symbol Table ===\n";
    sym.dump();

    cout << "\n=== Error Summary ===\n";
    err.printSummary();

    cout << "\nLexing complete.\n";
    return 0;
}
