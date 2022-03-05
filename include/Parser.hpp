#ifndef PARSER_GUARD
#define PARSER_GUARD
#include <iostream>
#include <string>
#include <string_view>

typedef int RC;     //Return code from parser

int readFile(std::string filename, std::string &s);
bool isLetter(std::string_view &s);
bool isDigit(std::string_view &s);
bool isRelOp(std::string_view &s);
bool isIdent(std::string_view &s);
bool isNumber(std::string_view &s);
bool isDesignator(std::string_view &s);
bool isFactor(std::string_view &s);
bool isTerm(std::string_view &s);
bool isExpression(std::string_view &s);
bool isRelation(std::string_view &s);
bool isAssignment(std::string_view &s);
bool isFuncCall(std::string_view &s);
bool isIfStatement(std::string_view &s);
bool isWhileStatement(std::string_view &s);
bool isReturnStatement(std::string_view &s);
bool isStatement(std::string_view &s);
bool isStatSequence(std::string_view &s);
bool isTypeDecl(std::string_view &s);
bool isVarDecl(std::string_view &s);
bool isFuncDecl(std::string_view &s);
bool isFormalParam(std::string_view &s);
bool isFuncBody(std::string_view &s);
bool isComputation(std::string_view &s);

#endif