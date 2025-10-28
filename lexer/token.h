#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenKind{
    IDENT,
    FLOAT_LIT,

    SEMICOLON, 

    // arithmetic
    PLUS, MINUS, STAR, SLASH,

    // assignment
    ASSIGN,

    END_OF_FILE,
    UNKNOWN
};

// Token structure
struct Token{
    TokenKind kind;
    int line; 
    int column;
    std::string lexeme;

    Token() : kind(TokenKind::UNKNOWN), lexeme(), line(-1), column(-1) {}
    Token(TokenKind k, const std::string &lx, int l, int c)
        : kind(k), lexeme(lx), line(l), column(c) {}


};


#endif // TOKEN_H