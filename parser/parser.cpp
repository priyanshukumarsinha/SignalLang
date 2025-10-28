#include "parser.h"
#include <iostream>

using namespace std;

Parser::Parser(Lexer *lexer_, SymbolTable *sym_, ErrorHandler *err_)
    : lexer(lexer_), sym(sym_), err(err_) {
    // initialize lookahead
    cur = lexer->getNextToken();
}

// consume one token and return the newly consumed token (old cur)
Token Parser::nextToken() {
    Token old = cur;
    cur = lexer->getNextToken();
    return old;
}

void Parser::advance() {
    cur = lexer->getNextToken();
}

bool Parser::accept(TokenKind k) {
    if (cur.kind == k) {
        advance();
        return true;
    }
    return false;
}

bool Parser::expect(TokenKind k) {
    if (cur.kind == k) {
        advance();
        return true;
    }
    // report syntax error
    syntaxError(string("Expected token '") + to_string((int)k) + "' but found '" + cur.lexeme + "'", cur);
    return false;
}

void Parser::syntaxError(const std::string &msg, const Token &t) {
    if (err) {
        // report with token position if available
        err->reportError(ErrorPhase::SYNTAX, msg, t.line, t.column);
    } else {
        cerr << "[Syntax] (line " << t.line << "," << t.column << "): " << msg << "\n";
    }
}

/* ---------------- parse entry ---------------- */
bool Parser::parse() {
    bool ok = true;
    // program := statement* EOF
    while (cur.kind != TokenKind::END_OF_FILE) {
        bool s = parseStatement();
        if (!s) {
            // on syntax error, attempt simple recovery: skip tokens until semicolon or EOF
            ok = false;
            while (cur.kind != TokenKind::SEMICOLON && cur.kind != TokenKind::END_OF_FILE) {
                advance();
            }
            if (cur.kind == TokenKind::SEMICOLON) advance(); // skip sentinel
            // continue parsing next statement
        }
    }
    return ok;
}

/* ---------------- statement ----------------
   statement := IDENT ASSIGN expression SEMICOLON
*/
bool Parser::parseStatement() {
    // require IDENT
    if (cur.kind != TokenKind::IDENT) {
        syntaxError("Statement must start with identifier (assignment).", cur);
        return false;
    }

    // store LHS name and position
    string lhsName = cur.lexeme;
    int lhsLine = cur.line;
    nextToken(); // consume IDENT

    // expect '='
    if (!expect(TokenKind::ASSIGN)) return false;

    // parse expression
    if (!parseExpression()) {
        syntaxError("Invalid expression on right side of assignment.", cur);
        return false;
    }

    // expect ';'
    if (!expect(TokenKind::SEMICOLON)) {
        // If semicolon missing, try to continue
        return false;
    }

    // Semantic handling: declare/define LHS variable if needed.
    // If symbol exists and was dummy -> update; else insert as variable (type float).
    SymbolEntry *entry = sym->lookup(lhsName);
    if (entry) {
        if (entry->is_dummy) {
            // update placeholder to concrete variable
            sym->updateEntry(lhsName, [lhsLine](SymbolEntry &e){
                e.kind = "variable";
                e.type = "float";
                e.is_dummy = false;
                e.decl_line = lhsLine;
            });
        } else {
            // already declared — okay
        }
    } else {
        // Insert new variable in current scope
        SymbolEntry e(lhsName, "variable", "float", sym->currentScope(), lhsLine);
        sym->insert(e);
    }

    // Mark LHS as used (assignment counts as usage)
    sym->markUsed(lhsName);

    return true;
}

/* ---------------- expression parsing ----------------
   expression := term ( (PLUS|MINUS) term )*
*/
bool Parser::parseExpression() {
    if (!parseTerm()) return false;
    while (cur.kind == TokenKind::PLUS || cur.kind == TokenKind::MINUS) {
        // consume operator
        advance();
        if (!parseTerm()) {
            syntaxError("Missing term after operator", cur);
            return false;
        }
    }
    return true;
}

/* ---------------- term parsing ----------------
   term := factor ( (STAR|SLASH) factor )*
*/
bool Parser::parseTerm() {
    if (!parseFactor()) return false;
    while (cur.kind == TokenKind::STAR || cur.kind == TokenKind::SLASH) {
        advance();
        if (!parseFactor()) {
            syntaxError("Missing factor after operator", cur);
            return false;
        }
    }
    return true;
}

/* ---------------- factor parsing ----------------
   factor := IDENT | FLOAT_LIT
*/
bool Parser::parseFactor() {
    if (cur.kind == TokenKind::IDENT) {
        // IDENT in expression -> mark used
        string name = cur.lexeme;
        int useLine = cur.line;
        sym->markUsed(name);
        // consume
        advance();
        return true;
    } else if (cur.kind == TokenKind::FLOAT_LIT) {
        // numeric literal
        advance();
        return true;
    } else {
        syntaxError("Expected identifier or numeric literal in expression", cur);
        return false;
    }
}
