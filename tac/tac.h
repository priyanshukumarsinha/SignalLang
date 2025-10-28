#ifndef TAC_H
#define TAC_H

#include <string>
#include <vector>
#include <iostream>

enum class TACOp {
    LOAD_CONST, // dest = const (literal stored in arg1Literal)
    ASSIGN,     // dest = arg1
    ADD, SUB, MUL, DIV, // dest = arg1 op arg2
    NOP
};

struct TacInst {
    TACOp op;
    std::string dest;   // destination variable/temp
    std::string arg1;   // operand 1 (var/temp)
    std::string arg2;   // operand 2 (var/temp) if any
    std::string arg1Literal; // used when op==LOAD_CONST (literal text)

    TacInst() : op(TACOp::NOP) {}
};

static inline std::string opToString(TACOp op) {
    switch (op) {
        case TACOp::LOAD_CONST: return "LOAD_CONST";
        case TACOp::ASSIGN: return "ASSIGN";
        case TACOp::ADD: return "ADD";
        case TACOp::SUB: return "SUB";
        case TACOp::MUL: return "MUL";
        case TACOp::DIV: return "DIV";
        default: return "NOP";
    }
}

static inline void printTacLine(const TacInst &i, std::ostream &out = std::cout) {
    switch (i.op) {
        case TACOp::LOAD_CONST:
            out << i.dest << " = " << i.arg1Literal << "\n";
            break;
        case TACOp::ASSIGN:
            out << i.dest << " = " << i.arg1 << "\n";
            break;
        case TACOp::ADD:
        case TACOp::SUB:
        case TACOp::MUL:
        case TACOp::DIV: {
            std::string sym = (i.op==TACOp::ADD? "+" : i.op==TACOp::SUB ? "-" : i.op==TACOp::MUL ? "*" : "/");
            out << i.dest << " = " << i.arg1 << " " << sym << " " << i.arg2 << "\n";
            break;
        }
        default:
            out << "// NOP\n";
    }
}

#endif // TAC_H
