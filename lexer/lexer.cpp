#include "lexer.h"
#include <cctype>
#include <iostream>

using namespace std;

/* ---------------- Constructor ---------------- */
Lexer::Lexer(SymbolTable* sym, ErrorHandler* err)
    : src(nullptr), idx(0), length(0), line(1), col(1),
      symtab(sym), errHandler(err) {}

/* ---------------- State helpers ---------------- */
bool Lexer::eof() const {
    return idx >= length;
}

char Lexer::peek(int offset) const {
    size_t pos = idx + offset;
    if (pos >= length) return '\0';
    return (*src)[pos];
}

char Lexer::get() {
    if (eof()) return '\0';
    char c = (*src)[idx++];
    if (c == '\n') { ++line; col = 1; }
    else ++col;
    return c;
}

void Lexer::skipWhitespace() {
    while (!eof() && isspace(static_cast<unsigned char>(peek())))
        get();
}

/* ---------------- Small char classifiers ---------------- */
bool Lexer::isIdentStart(char c) const {
    return isalpha(static_cast<unsigned char>(c)) || c == '_';
}
bool Lexer::isIdentBody(char c) const {
    return isalnum(static_cast<unsigned char>(c)) || c == '_';
}

/* ---------------- Error reporting ---------------- */
void Lexer::reportError(const std::string& msg) {
    if (errHandler)
        errHandler->reportError(ErrorPhase::LEXICAL, msg, line, col);
    else
        cerr << "[Lexical Error] Line " << line << ", Col " << col << ": " << msg << "\n";
}

/* ---------------- Token lexers ---------------- */

// identifier (and placeholder insertion)
Token Lexer::lexIdentifier() {
    int startLine = line, startCol = col;
    string lex;
    // first char already ensured to be ident-start by caller
    while (!eof() && isIdentBody(peek())) lex.push_back(get());

    // insert placeholder in symbol table (if provided)
    if (symtab && !symtab->existsInCurrentScope(lex)) {
        symtab->insertTokenPlaceholder(lex, startLine);
    }
    return Token(TokenKind::IDENT, lex, startLine, startCol);
}

// number (simple float recognition: digits and optional single '.')
Token Lexer::lexNumber() {
    int startLine = line, startCol = col;
    string lex;
    bool seenDot = false;

    // Support forms: 123  3.14  .5  12.
    if (peek() == '.') {
        seenDot = true;
        lex.push_back(get()); // consume '.'
        if (!isdigit(static_cast<unsigned char>(peek()))) {
            reportError("Malformed number literal: '.' not followed by digits");
            return Token(TokenKind::UNKNOWN, lex, startLine, startCol);
        }
        while (!eof() && isdigit(static_cast<unsigned char>(peek()))) lex.push_back(get());
        return Token(TokenKind::FLOAT_LIT, lex, startLine, startCol);
    }

    // integer part
    while (!eof() && isdigit(static_cast<unsigned char>(peek()))) lex.push_back(get());

    // fractional part
    if (peek() == '.') {
        seenDot = true;
        lex.push_back(get()); // consume '.'
        while (!eof() && isdigit(static_cast<unsigned char>(peek()))) lex.push_back(get());
    }

    if (seenDot) return Token(TokenKind::FLOAT_LIT, lex, startLine, startCol);
    // In your minimal token set only FLOAT_LIT exists (integers aren't a separate kind).
    // We still return FLOAT_LIT for numbers without dot to keep parser simple (or you can adjust).
    return Token(TokenKind::FLOAT_LIT, lex, startLine, startCol);
}

// operators & semicolon
Token Lexer::lexOperatorOrSymbol() {
    int startLine = line, startCol = col;
    char c = get();
    switch (c) {
        case '+': return Token(TokenKind::PLUS, "+", startLine, startCol);
        case '-': return Token(TokenKind::MINUS, "-", startLine, startCol);
        case '*': return Token(TokenKind::STAR, "*", startLine, startCol);
        case '/': return Token(TokenKind::SLASH, "/", startLine, startCol);
        case '=': return Token(TokenKind::ASSIGN, "=", startLine, startCol);
        case ';': return Token(TokenKind::SEMICOLON, ";", startLine, startCol);
        default:
            reportError(string("Unrecognized symbol '") + c + "'");
            return Token(TokenKind::UNKNOWN, string(1, c), startLine, startCol);
    }
}

/* ---------------- Public streaming API ---------------- */

// Set the source to be lexed and reset internal state.
// Use this before calling getNextToken().
void Lexer::setSource(const std::string& source) {
    // keep owned copy so pointer remains valid
    ownedSource = source;
    src = &ownedSource;
    idx = 0;
    length = src->size();
    line = 1;
    col = 1;
}

// Return next token from input. Stateful: repeated calls return the stream.
// At EOF, returns TokenKind::END_OF_FILE (and does not advance further).
Token Lexer::getNextToken() {
    if (!src) {
        // no source set — return EOF immediately
        return Token(TokenKind::END_OF_FILE, "<EOF>", 0, 0);
    }

    // Skip whitespace and newlines
    skipWhitespace();

    if (eof()) {
        return Token(TokenKind::END_OF_FILE, "<EOF>", line, col);
    }

    char c = peek();

    // identifier
    if (isIdentStart(c)) {
        return lexIdentifier();
    }

    // number (digit or .digit)
    if (isdigit(static_cast<unsigned char>(c)) || (c == '.' && isdigit(static_cast<unsigned char>(peek(1))))) {
        return lexNumber();
    }

    // operators & semicolon
    return lexOperatorOrSymbol();
}

/* ---------------- One-shot tokenize convenience ---------------- */
std::vector<Token> Lexer::tokenize(const std::string& source) {
    // set the source and repeatedly call getNextToken
    setSource(source);
    std::vector<Token> toks;
    while (true) {
        Token t = getNextToken();
        toks.push_back(t);
        if (t.kind == TokenKind::END_OF_FILE) break;
    }
    return toks;
}
