#ifndef PARSER_H
#define PARSER_H

#include <string>
// #include "parser/ast_stub.h" // small internal stub (optional, not used heavily)
#include "../lexer/token.h"
#include "../lexer/lexer.h"
#include "../symbolTable/symbolTable.h"
#include "../errorHandler/errorHandler.h"

/*
 * Simple recursive-descent parser for the minimal SignalLang.
 *
 * Grammar (informal):
 *
 *   program     := statement* EOF
 *   statement   := IDENT ASSIGN expression SEMICOLON
 *   expression  := term ( (PLUS|MINUS) term )*
 *   term        := factor ( (STAR|SLASH) factor )*
 *   factor      := IDENT | FLOAT_LIT
 *
 * The parser uses a streaming lexer (getNextToken) and interacts with the symbol table:
 *  - Right-hand IDENT usage: sym->markUsed(name)
 *  - Left-hand IDENT (assignment target): sym->insert(...) or updateEntry(...) to declare variable
 *
 * Error handling: syntax & semantic errors reported to ErrorHandler.
 */

class Parser {
public:
    // Construct with pointers to lexer, symbol table, and error handler.
    Parser(Lexer *lexer, SymbolTable *sym, ErrorHandler *err);

    // Parse the entire source (lexer should already have source set).
    // Returns true if parse succeeded without syntax errors (note: semantic errors may still be present).
    bool parse();

private:
    Lexer *lexer;
    SymbolTable *sym;
    ErrorHandler *err;

    // current lookahead token
    Token cur;

    // helpers
    Token nextToken();              // adv -> returns next token (consumes)
    void advance();                 // helper to advance cur
    bool accept(TokenKind k);       // if cur.kind == k then advance and return true
    bool expect(TokenKind k);       // if cur.kind == k then advance else report and return false

    // parsing functions following grammar
    bool parseStatement();
    bool parseExpression();   // returns true if parsed an expression (we don't create AST here)
    bool parseTerm();
    bool parseFactor();

    // reporting helpers
    void syntaxError(const std::string &msg, const Token &t);
};

#endif // PARSER_H
