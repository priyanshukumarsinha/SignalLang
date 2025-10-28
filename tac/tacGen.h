#ifndef TACGEN_H
#define TACGEN_H

#include "../lexer/lexer.h"
#include "../symbolTable/symbolTable.h"
#include "../errorHandler/errorHandler.h"
#include "tac.h"
#include <vector>
#include <stack>
#include <string>
#include <memory>

/*
 * TACGenerator
 *  - Consumes tokens via a Lexer (streaming API: setSource + getNextToken)
 *  - Parses statements of the form: IDENT = expression ;
 *  - Expressions: term ((+|-) term)*
 *  - Term: factor ((*|/) factor)*
 *  - Factor: IDENT | FLOAT_LIT
 *
 *  Emits three-address code into vector<TacInst>.
 *
 *  Minimizes temporaries by reusing freed temps (basic).
 */
class TACGenerator {
public:
    TACGenerator(Lexer *lexer, SymbolTable *sym, ErrorHandler *err);

    // Generate TAC from the lexer's current source (lexer must be set via setSource)
    // Returns true on successful parse & generation (syntax errors may be reported).
    bool generate(std::vector<TacInst> &out);

    // Utility: pretty print TAC
    static void print(const std::vector<TacInst> &tac, std::ostream &out = std::cout);

private:
    Lexer *lexer;
    SymbolTable *sym;
    ErrorHandler *err;

    Token cur; // lookahead

    // temp management
    int tempCounter;
    std::stack<std::string> freeTemps;
    std::string newTemp();

    // helpers
    Token nextToken();
    void advance();

    // parsing & emission
    bool parseProgram(std::vector<TacInst> &out);
    bool parseStatement(std::vector<TacInst> &out);
    // parseExpr returns the name (variable or temp) which holds the value
    bool parseExpression(std::vector<TacInst> &out, std::string &result);
    bool parseTerm(std::vector<TacInst> &out, std::string &result);
    bool parseFactor(std::vector<TacInst> &out, std::string &result);

    // emit helpers
    void emitLoadConst(std::vector<TacInst> &out, const std::string &dest, const std::string &literal);
    void emitBinary(std::vector<TacInst> &out, TACOp op, const std::string &dest,
                    const std::string &a, const std::string &b);
    void emitAssign(std::vector<TacInst> &out, const std::string &dest, const std::string &src);
};

#endif // TACGEN_H
