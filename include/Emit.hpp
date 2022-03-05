#ifndef EMIT_GUARD
#define EMIT_GUARD

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
// void startIfStatement();
void emitIfStatement();
void startStatSequence();
void endStatSequence();
void emitTypeDecl(const int &dimNumber);
void emitVarDecl();     // Put variable into map
void emitComputation();

#endif