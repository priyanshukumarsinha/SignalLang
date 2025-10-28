#include "tacGen.h"
#include <iostream>

using namespace std;

TACGenerator::TACGenerator(Lexer *lexer_, SymbolTable *sym_, ErrorHandler *err_)
    : lexer(lexer_), sym(sym_), err(err_), tempCounter(0) {
    // initialize lookahead
    cur = lexer->getNextToken();
}

Token TACGenerator::nextToken() {
    Token old = cur;
    cur = lexer->getNextToken();
    return old;
}
void TACGenerator::advance() { cur = lexer->getNextToken(); }

string TACGenerator::newTemp() {
    if (!freeTemps.empty()) {
        string t = freeTemps.top(); freeTemps.pop();
        return t;
    }
    string t = "t" + to_string(tempCounter++);
    return t;
}

/* emit helpers */
void TACGenerator::emitLoadConst(vector<TacInst> &out, const string &dest, const string &literal) {
    TacInst i; i.op = TACOp::LOAD_CONST; i.dest = dest; i.arg1Literal = literal;
    out.push_back(i);
}
void TACGenerator::emitBinary(vector<TacInst> &out, TACOp op, const string &dest, const string &a, const string &b) {
    TacInst i; i.op = op; i.dest = dest; i.arg1 = a; i.arg2 = b;
    out.push_back(i);
}
void TACGenerator::emitAssign(vector<TacInst> &out, const string &dest, const string &src) {
    TacInst i; i.op = TACOp::ASSIGN; i.dest = dest; i.arg1 = src;
    out.push_back(i);
}

/* parse program */
bool TACGenerator::generate(vector<TacInst> &out) {
    return parseProgram(out);
}

bool TACGenerator::parseProgram(vector<TacInst> &out) {
    while (cur.kind != TokenKind::END_OF_FILE) {
        if (!parseStatement(out)) {
            // on syntax error, try to recover to next semicolon
            if (err) err->reportError(ErrorPhase::SYNTAX, "Skipping to next ';' on parse error", cur.line, cur.column);
            while (cur.kind != TokenKind::SEMICOLON && cur.kind != TokenKind::END_OF_FILE) advance();
            if (cur.kind == TokenKind::SEMICOLON) advance();
        }
    }
    return true; // we don't fail hard; errors are reported to ErrorHandler
}

bool TACGenerator::parseStatement(vector<TacInst> &out) {
    // statement := IDENT ASSIGN expression SEMICOLON
    if (cur.kind != TokenKind::IDENT) {
        if (err) err->reportError(ErrorPhase::SYNTAX, "Expected identifier at start of statement", cur.line, cur.column);
        return false;
    }
    string lhs = cur.lexeme;
    int lhsLine = cur.line;
    nextToken(); // consume IDENT

    if (cur.kind != TokenKind::ASSIGN) {
        if (err) err->reportError(ErrorPhase::SYNTAX, "Expected '=' after identifier", cur.line, cur.column);
        return false;
    }
    nextToken(); // consume ASSIGN

    string rhsName;
    if (!parseExpression(out, rhsName)) {
        if (err) err->reportError(ErrorPhase::SYNTAX, "Invalid expression in assignment", cur.line, cur.column);
        return false;
    }

    // expect semicolon
    if (cur.kind != TokenKind::SEMICOLON) {
        if (err) err->reportError(ErrorPhase::SYNTAX, "Missing semicolon at end of statement", cur.line, cur.column);
        return false;
    }
    nextToken(); // consume semicolon

    // semantic: ensure lhs exists (or insert)
    SymbolEntry *entry = sym->lookup(lhs);
    if (entry) {
        if (entry->is_dummy) {
            sym->updateEntry(lhs, [lhsLine](SymbolEntry &e){
                e.kind = "variable";
                e.type = "float";
                e.is_dummy = false;
                e.decl_line = lhsLine;
            });
        }
    } else {
        SymbolEntry e(lhs, "variable", "float", sym->currentScope(), lhsLine);
        sym->insert(e);
    }

    // emit assign: if rhsName is a temp or variable or literal-holder
    // if rhsName looks like a literal indicator we should have produced a temp with LOAD_CONST.
    emitAssign(out, lhs, rhsName);

    // mark lhs used (assignment counts)
    sym->markUsed(lhs);
    return true;
}

/* parseExpression (handles precedence): expression := term ((+|-) term)* */
bool TACGenerator::parseExpression(vector<TacInst> &out, string &result) {
    string left;
    if (!parseTerm(out, left)) return false;

    while (cur.kind == TokenKind::PLUS || cur.kind == TokenKind::MINUS) {
        TokenKind op = cur.kind;
        nextToken();
        string right;
        if (!parseTerm(out, right)) {
            if (err) err->reportError(ErrorPhase::SYNTAX, "Missing term after operator", cur.line, cur.column);
            return false;
        }
        // produce temp dest
        string dest = newTemp();
        // emit op
        if (op == TokenKind::PLUS) emitBinary(out, TACOp::ADD, dest, left, right);
        else emitBinary(out, TACOp::SUB, dest, left, right);
        // release temps (if left/right were temps we can push them back)
        if (left.size()>0 && left[0]=='t') freeTemps.push(left);
        if (right.size()>0 && right[0]=='t') freeTemps.push(right);
        left = dest;
    }
    result = left;
    return true;
}

/* parseTerm := factor ((*|/) factor)* */
bool TACGenerator::parseTerm(vector<TacInst> &out, string &result) {
    string left;
    if (!parseFactor(out, left)) return false;

    while (cur.kind == TokenKind::STAR || cur.kind == TokenKind::SLASH) {
        TokenKind op = cur.kind;
        nextToken();
        string right;
        if (!parseFactor(out, right)) {
            if (err) err->reportError(ErrorPhase::SYNTAX, "Missing factor after operator", cur.line, cur.column);
            return false;
        }
        string dest = newTemp();
        if (op == TokenKind::STAR) emitBinary(out, TACOp::MUL, dest, left, right);
        else emitBinary(out, TACOp::DIV, dest, left, right);
        if (left.size()>0 && left[0]=='t') freeTemps.push(left);
        if (right.size()>0 && right[0]=='t') freeTemps.push(right);
        left = dest;
    }
    result = left;
    return true;
}

/* factor := IDENT | FLOAT_LIT
   - For FLOAT_LIT: create a temp and LOAD_CONST literal into it and return temp name
   - For IDENT: just return the identifier name (and markUsed)
*/
bool TACGenerator::parseFactor(vector<TacInst> &out, string &result) {
    if (cur.kind == TokenKind::IDENT) {
        string name = cur.lexeme;
        sym->markUsed(name); // mark usage
        nextToken();
        result = name;
        return true;
    } else if (cur.kind == TokenKind::FLOAT_LIT) {
        string lit = cur.lexeme;
        // create a temp to hold literal
        string dest = newTemp();
        emitLoadConst(out, dest, lit);
        nextToken();
        result = dest;
        return true;
    } else {
        if (err) err->reportError(ErrorPhase::SYNTAX, "Expected identifier or float literal", cur.line, cur.column);
        return false;
    }
}

void TACGenerator::print(const vector<TacInst> &tac, ostream &out) {
    for (size_t i = 0; i < tac.size(); ++i) {
        out << i << ":\t";
        printTacLine(tac[i], out);
    }
}
