#ifndef LEXER_H
#define LEXER_H

#include <string>
#include "token.h"
#include "../symbolTable/symbolTable.h"
#include "../errorHandler/errorHandler.h"
#include <vector>

/*
 * Simple stateful Lexer for SignalLang (minimal token set)
 *
 * Usage patterns:
 * 1) One-shot: call tokenize(source) -> vector<Token>
 * 2) Streaming: call setSource(source) then repeatedly call getNextToken()
 *
 * The lexer integrates with SymbolTable to insert placeholders for identifiers,
 * and with ErrorHandler to report lexical errors.
 */
class Lexer {
public:
    // Construct with optional pointers to symbol table and error handler.
    Lexer(SymbolTable* sym = nullptr, ErrorHandler* err = nullptr);

    // One-shot tokenization (keeps the tokenizer stateless externally).
    std::vector<Token> tokenize(const std::string& source);

    // Streaming API:
    //  - setSource initializes the internal buffer and cursor
    //  - getNextToken returns the next token each time it's called
    void setSource(const std::string& source);
    Token getNextToken();

    // Helper: peek next token without consuming (consumes internally, so you can push it back if needed)
    // (Not implemented pushback here to keep lexer minimal — parser may store last token if needed.)

private:
    // Source buffer + cursor state
    const std::string* src;
    std::string ownedSource; // used when setSource is called with a temporary
    size_t idx;
    size_t length;
    int line; // 1-based
    int col;  // 1-based

    SymbolTable* symtab;
    ErrorHandler* errHandler;

    // Low-level helpers
    bool eof() const;
    char peek(int offset = 0) const;
    char get();
    void skipWhitespace();

    // Token factories (internal, used by both tokenize and getNextToken)
    Token lexIdentifier();
    Token lexNumber();
    Token lexOperatorOrSymbol();

    // Small char helpers
    bool isIdentStart(char c) const;
    bool isIdentBody(char c) const;

    // Error reporting helper
    void reportError(const std::string& msg);
};

#endif // LEXER_H
