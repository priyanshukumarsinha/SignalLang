#ifndef DCE_H
#define DCE_H

#include "tac.h"
#include "../symbolTable/symbolTable.h"
#include <vector>

/*
 * DeadCodeEliminator
 *  - Performs a backward liveness pass on linear TAC (no control flow).
 *  - Preserves instructions that define symbols which are live (either
 *    used later in program, or marked used externally via symbol table).
 *  - Side-effect free assumption: LOAD_CONST, ASSIGN, ADD/SUB/MUL/DIV are pure.
 */
class DeadCodeEliminator {
public:
    // Run DCE in-place on tac. Uses symbol table to initialize live set for named variables.
    static void eliminate(std::vector<TacInst> &tac, const SymbolTable &sym);
};

#endif // DCE_H
