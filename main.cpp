#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "lexer/lexer.h"
#include "lexer/token.h"
#include "symbolTable/symbolTable.h"
#include "errorHandler/errorHandler.h"
#include "parser/parser.h"

#include "tac/tacGen.h"
#include "tac/dce.h"

using namespace std;

// Converts enum TokenKind to readable string
string tokenKindToString(TokenKind kind) {
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

// Utility function to read file content
string readFile(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file '" << filename << "'\n";
        exit(1);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    cout << "=== SignalLang Compiler ===\n";
    cout << "(Lexer → Parser → TAC → Dead Code Elimination)\n\n";

    // ---- Step 1: Check command-line arguments ----
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <source_file.signal>\n";
        cerr << "Example: ./build/SensorLang examples/example.signal\n";
        return 1;
    }

    string filename = argv[1];
    string source = readFile(filename);

    cout << "Loaded program: " << filename << "\n";
    cout << string(60, '-') << "\n";
    cout << source << "\n";
    cout << string(60, '-') << "\n\n";

    // ---- Step 2: Initialize components ----
    ErrorHandler err;
    SymbolTable sym(&err);
    Lexer lexer(&sym, &err);

    // ---- Step 3: Lexical Analysis (display tokens) ----
    cout << "Lexing tokens for display...\n\n";
    auto tokens = lexer.tokenize(source);

    cout << left << setw(12) << "TOKEN"
         << setw(20) << "LEXEME"
         << setw(8)  << "LINE"
         << setw(8)  << "COL" << "\n";
    cout << string(48, '-') << "\n";

    for (const auto &t : tokens) {
        if (t.kind == TokenKind::END_OF_FILE) break;
        cout << left << setw(12) << tokenKindToString(t.kind)
             << setw(20) << t.lexeme
             << setw(8)  << t.line
             << setw(8)  << t.column
             << "\n";
    }

    cout << "\n(End of token listing)\n\n";

    // ---- Step 4: Parsing ----
    lexer.setSource(source);
    sym.insert(SymbolEntry("in", "builtin", "float()->float", sym.currentScope(), -1));
    sym.insert(SymbolEntry("out", "builtin", "void(float)", sym.currentScope(), -1));

    Parser parser(&lexer, &sym, &err);
    cout << "Parsing source...\n";
    bool parseOk = parser.parse();
    cout << "\nParsing " << (parseOk ? "succeeded" : "failed (syntax errors)") << ".\n\n";

    // ---- Step 5: TAC Generation ----
    lexer.setSource(source); // reset for TAC generation
    TACGenerator tacGen(&lexer, &sym, &err);
    vector<TacInst> tac;
    tacGen.generate(tac);

    cout << "=== Generated TAC (Before DCE) ===\n";
    TACGenerator::print(tac);
    cout << "\n";

    // ---- Step 6: Dead Code Elimination ----
    cout << "Running Dead Code Elimination...\n";
    DeadCodeEliminator::eliminate(tac, sym);

    cout << "\n=== TAC (After DCE) ===\n";
    TACGenerator::print(tac);
    cout << "\n";

    // ---- Step 7: Final Outputs ----
    cout << "=== Final Symbol Table ===\n";
    sym.dump();

    cout << "\n=== Compiler Messages ===\n";
    err.printSummary();

    cout << "\nCompilation complete.\n";
    return 0;
}
