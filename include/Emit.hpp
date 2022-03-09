#ifndef EMIT_GUARD
#define EMIT_GUARD

#include <Instruction.hpp>
#include <string>

void emitDigit(const char &c);       //Put terminal character to stack
void emitLetter(const char &c);      //Put terminal character to stack
void emitRelOp(const std::string &symbol);
void emitIdent();
void emitNumber();
void emitDesignator();
void emitFactor();
void emitTerm(const std::string &operators);
void emitExpression(const std::string &operators);
void emitRelation();
void emitAssignment();
void emitIfStatement();
void emitWhileStatement();
void startStatSequence();
void endStatSequence();
void emitTypeDecl(const int &dimNumber);
void emitVarDecl();     // Put variable into map
void emitComputation();
void rewrite_after_loop();
void common_subexpression_elimination();
void copy_propagation();
std::string opCode_to_string(Instruct instruct);

#endif