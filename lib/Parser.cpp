#include <Parser.hpp>
#include <filesystem>
#include <fstream>
#include <Emit.hpp>


int readFile(std::string filename, std::string &s){
    if(!std::filesystem::exists(filename)){
        return -1;
    }
    std::ifstream infile(filename);
    std::string line("");
    while(std::getline(infile, line)){
        s += line;
    }
    return 0;
}

bool isLetter(std::string_view &s){
    if(s.size() == 0 || !islower(s[0])){
        return false;
    }
    emitLetter(s[0]);
    s = s.substr(1);
    return true;
}

bool isDigit(std::string_view &s){
    if(s.size() == 0 || !isdigit(s[0])){
        return false;
    }
    emitDigit(s[0]);
    s = s.substr(1);
    return true;
}

bool isRelOp(std::string_view &s){
    std::string symbol;
    if(s.size() == 0 || 
    (s[0] != '=' && s[0] != '!' && s[0] != '<' && s[0] != '>')){
        return false;
    }
    std::string_view op;
    if(s.size() >= 2){
        op = s.substr(0, 2);
        if(op == "==" || op == "!=" || op == "<=" || op == ">="){
            symbol = s.substr(0, 2);
            s = s.substr(2);
            emitRelOp(symbol);
            return true;
        }
    }
    op = s.substr(0, 1);
    if(op == ">" || op == "<"){
        symbol += op;
        s = s.substr(1);
        emitRelOp(symbol);
        return true;
    }
    return false;
}

bool isIdent(std::string_view &s){
    if(s.size() == 0 || !isLetter(s)){
        return false;
    }
    while(isLetter(s) || isDigit(s)){
        continue;
    }
    emitIdent();
    return true;
}

bool isNumber(std::string_view &s){
    if(s.size() == 0 || !isDigit(s)){
        return false;
    }
    while(isDigit(s)){
        continue;
    }
    emitNumber();
    return true;
}

bool isDesignator(std::string_view &s){
    std::string_view s_copy(s);
    if(!isIdent(s_copy)){
        return false;
    }
    while(s_copy.size() > 0 && s_copy[0] == '['){
        s_copy = s_copy.substr(1);
        if(isExpression(s_copy) && s_copy.size() > 0 && s_copy[0] == ']'){
            s_copy = s_copy.substr(1);
        }else{
            return false;
        }
    }
    s = s_copy;
    return true;
}

bool isFactor(std::string_view &s){
    std::string_view s_copy(s);
    // Test function call first since "call" will be consumed by designator
    if(isFuncCall(s_copy) || isDesignator(s_copy) || isNumber(s_copy)){
        emitFactor();
        s = s_copy;
        return true;
    }
    if(s_copy.size() > 0 && s_copy[0] == '('){
        s_copy = s_copy.substr(1);
        if(isExpression(s_copy) && s_copy.size() > 0 && s_copy[0] == ')'){
            s_copy = s_copy.substr(1);
            s = s_copy;
            // No need to call emitFactor to pass expression id in stack
            return true;
        }
    }
    return false;
}

bool isTerm(std::string_view &s){
    std::string_view s_copy(s);
    if(!isFactor(s_copy)){
        return false;
    }
    std::string operators;
    while(s_copy.size() > 0 && (s_copy[0] == '*' || s_copy[0] == '/')){
        operators += s_copy[0];
        s_copy = s_copy.substr(1);
        if(!isFactor(s_copy)){
            return false;
        }
    }
    s = s_copy;
    emitTerm(operators);
    return true;
}

bool isExpression(std::string_view &s){
    std::string_view s_copy(s);
    if(!isTerm(s_copy)){
        return false;
    }
    std::string operators;
    while(s_copy.size() > 0 && (s_copy[0] == '+' || s_copy[0] == '-')){
        operators += s_copy[0];
        s_copy = s_copy.substr(1);
        if(!isTerm(s_copy)){
            return false;
        }
    }
    s = s_copy;
    emitExpression(operators);
    return true;
}

bool isRelation(std::string_view &s){
    std::string_view s_copy(s);
    if(isExpression(s_copy) && isRelOp(s_copy) && isExpression(s_copy)){
        s = s_copy;
        emitRelation();
        return true;
    }
    return false;
}

bool isAssignment(std::string_view &s){
    // We need to make a space between let and designator
    // However, there is no space in designator<-expression.
    if(s.size() < 4 || s.substr(0, 4) != std::string_view("let ")){
        return false;
    }
    // Consume "let "
    std::string_view s_copy = s.substr(4);
    if(isDesignator(s_copy) && s_copy.size() > 2 && s_copy.substr(0, 2) == std::string_view("<-")){
        // Consume assign symbol by s_copy
        s_copy = s_copy.substr(2);
        if(isExpression(s_copy)){
            s = s_copy;
            emitAssignment();
            return true;
        }
    }
    return false;
}

bool isFuncCall(std::string_view &s){
    // There is a space between call and ident
    // However, in the function, there is no space
    if(s.size() < 5 || s.substr(0, 5) != std::string_view("call ")){
        return false;
    }
    // Consume the "call " first by s_copy
    std::string_view s_copy = s.substr(5);
    if(isIdent(s_copy)){
        if(s_copy.size() > 0 && s_copy[0] == '('){
            // Consume left parantheses
            s_copy = s_copy.substr(1);
            if(isExpression(s_copy)){
                while(s_copy.size() > 0 && s_copy[0] == ','){
                    s_copy = s_copy.substr(1);
                    if(!isExpression(s_copy)){
                        return false;
                    }
                }
            }
            if(s_copy.size() > 0 && s_copy[0] == ')'){
                s_copy = s_copy.substr(1);
                s = s_copy;
                return true;
            }
        }else{
            // Function with no parameter and no parantheses
            s = s_copy;
            return true;
        }
    }
    return false;
}

bool isIfStatement(std::string_view &s){
    std::string_view s_copy(s);
    if(s_copy.size() < 3 || s_copy.substr(0, 3) != std::string_view("if ")){
        return false;
    }
    // Consume "if "
    s_copy = s_copy.substr(3);
    // Test and consume relation
    if(!isRelation(s_copy)){
        return false;
    }
    // Test " then "
    if(s_copy.size() < 6 || s_copy.substr(0, 6) != " then "){
        return false;
    }
    // Consume " then "
    s_copy = s_copy.substr(6);
    // Test and consume stateSequence
    if(!isStatSequence(s_copy)){
        return false;
    }
    // Test " else "
    if(s_copy.size() >= 6 && s_copy.substr(0, 6) == std::string_view(" else ")){
        // Consume " else "
        s_copy = s_copy.substr(6);
        if(!isStatSequence(s_copy)){
            return false;
        }
    }
    // Test " fi"
    if(s_copy.size() < 3 || s_copy.substr(0, 3) != std::string_view(" fi")){
        return false;
    }
    // Consume " fi"
    s_copy = s_copy.substr(3);
    s = s_copy;
    emitIfStatement();
    return true;
}

bool isWhileStatement(std::string_view &s){
    if(s.size() < 6 || s.substr(0, 6) != std::string_view("while ")){
        return false;
    }
    // Consume "while "
    std::string_view s_copy = s.substr(6);
    if(!isRelation(s_copy)){
        return false;
    }
    // Test " do "
    if(s_copy.size() < 4 || s_copy.substr(0, 4) != std::string_view(" do ")){
        return false;
    }
    // Consume " do "
    s_copy = s_copy.substr(4);
    // Test statSequence
    if(!isStatSequence(s_copy)){
        return false;
    }
    // Test " od"
    if(s_copy.size() < 3 || s_copy.substr(0, 3) != std::string_view(" od")){
        return false;
    }
    // Consume "od"
    s_copy = s_copy.substr(3);
    s = s_copy;
    return true;
}

bool isReturnStatement(std::string_view &s){
    if(s.size() < 6 || s.substr(0, 6) != std::string_view("return")){
        return false;
    }
    // Consume "return"
    s = s.substr(6);
    // Consume expression if contain " expression"
    if(s[0] == ' ' ) {
        std::string_view s_copy = s.substr(1);
        if(isExpression(s_copy)) {
            s = s_copy;
        }
    }
    return true;
}

bool isStatement(std::string_view &s){
    if(isAssignment(s) || isFuncCall(s) || isReturnStatement(s) || isIfStatement(s) || isWhileStatement(s)){
        return true;
    }
    return false;
}

bool isStatSequence(std::string_view &s){
    startStatSequence();
    if(!isStatement(s)){
        return false;
    }
    while(s.size() > 0 && s[0] == ';'){
        s = s.substr(1);
        if(!isStatement(s)){
            break;
        }
    }
    endStatSequence();
    return true;
}

bool isTypeDecl(std::string_view &s){
    int dimNumber = 0;
    std::string_view s_copy = s;
    // Check key word
    if(s_copy.size() >= 3 && s_copy.substr(0, 3) == std::string_view("var")){
        s_copy = s_copy.substr(3);
        s = s_copy;
        emitTypeDecl(dimNumber);
        return true;
    }else if(s_copy.size() >= 5 && s_copy.substr(0, 5) == std::string_view("array")){
        s_copy = s_copy.substr(5);
    }else{
        return false;
    }
    if(s_copy.size() == 0 || s_copy[0] != '[') {
        return false;
    }
    // Consume '['
    s_copy = s_copy.substr(1);
    // Test number and consume
    if(!isNumber(s_copy)){
        return false;
    }
    // Test right ]
    if(s_copy.size() == 0 || s_copy[0] != ']'){
        return false;
    }
    // Consume ]
    s_copy = s_copy.substr(1);
    dimNumber += 1;
    // Test multiple [number]
    while(s_copy.size() > 0 && s_copy[0] == '['){
        // Consume '['
        s_copy = s_copy.substr(1);
        // Test and consume number
        if(!isNumber(s_copy)){
            return false;
        }
        // Test ']'
        if(s_copy.size() == 0 || s_copy[0] != ']'){
            return false;
        }
        // Consume ']'
        s_copy = s_copy.substr(1);
        dimNumber += 1;
    }
    s = s_copy;
    emitTypeDecl(dimNumber);
    return true;
}

bool isVarDecl(std::string_view &s){
    std::string_view s_copy = s;
    // Test typeDecl and ident
    if(!isTypeDecl(s_copy) || !isIdent(s_copy)){
        return false;
    }
    while(s_copy.size() > 0 && s_copy[0] == ','){
        // Consume ','
        s_copy = s_copy.substr(1);
        // Test ident
        if(!isIdent(s_copy)){
            return false;
        }
    }
    if(s_copy.size() == 0 || s_copy[0] != ';'){
        return false;
    }
    // Consume ';'
    s_copy = s_copy.substr(1);
    s = s_copy;
    emitVarDecl();
    return true;
}

bool isFuncDecl(std::string_view &s){
    std::string_view s_copy = s;
    // Test and consume "void"
    if(s_copy.size() >= 4 && s_copy.substr(0, 4) == std::string_view("void")){
        s_copy = s_copy.substr(4);
    }
    // Test "function"
    if(s_copy.size() < 8 || s_copy.substr(0, 8) != std::string_view("function")){
        return false;
    }
    // Consume "function"
    s_copy = s_copy.substr(8);
    // Test ident and formalParam
    if(!isIdent(s_copy) || !isFormalParam(s_copy)){
        return false;
    }
    // Test ';'
    if(s_copy.size() == 0 || s_copy[0] != ';'){
        return false;
    }
    // Consume ';'
    s_copy = s_copy.substr(1);
    // Test funcBody
    if(!isFuncBody(s_copy)){
        return false;
    }
    // Test ';'
    if(s_copy.size() == 0 || s_copy[0] != ';'){
        return false;
    }
    // Consume ';'
    s_copy = s_copy.substr(1);
    s = s_copy;
    return true;
}

bool isFormalParam(std::string_view &s){
    std::string_view s_copy = s;
    if(s_copy.size() == 0 || s_copy[0] != '('){
        return false;
    }
    // Consume
    s_copy = s_copy.substr(1);
    if(isIdent(s_copy)){
        while(s_copy.size() > 0 && s_copy[0] == ','){
            // Consume ','
            s_copy = s_copy.substr(1);
            if(!isIdent(s_copy)){
                return false;
            }
        }
    }
    if(s_copy.size() == 0 || s_copy[0] != ')'){
        return false;
    }
    // Consume ')'
    s_copy = s_copy.substr(1);
    s = s_copy;
    return true;
}

bool isFuncBody(std::string_view &s){
    std::string_view s_copy = s;
    while(isVarDecl(s_copy)){
        continue;
    }
    if(s_copy.size() == 0 || s_copy[0] != '{'){
        return false;
    }
    // Consume '{'
    s_copy = s_copy.substr(1);
    // Consume statSequence
    isStatSequence(s_copy);
    if(s_copy.size() == 0 || s_copy[0] != '}'){
        return false;
    }
    // Consume '}'
    s_copy = s_copy.substr(1);
    s = s_copy;
    return true;
}

bool isComputation(std::string_view &s){
    std::string_view s_copy = s;
    // Test "main"
    if(s_copy.size() < 4 || s_copy.substr(0, 4) != std::string_view("main")){
        return false;
    }
    // Consume "main"
    s_copy = s_copy.substr(4);
    // Consume VarDecl
    while(isVarDecl(s_copy)){
        continue;
    }
    // Consume funcDecl
    while(isFuncDecl(s_copy)){
        continue;
    }
    // Test '{'
    if(s_copy.size() == 0 || s_copy[0] != '{'){
        return false;
    }
    // Consume '{'
    s_copy = s_copy.substr(1);
    // Test and consume statSequence
    if(!isStatSequence(s_copy)){
        return false;
    }
    // Test "}."
    if(s_copy.size() < 2 || s_copy.substr(0, 2) != std::string_view("}.")){
        return false;
    }
    // Consume "}."
    s_copy = s_copy.substr(2);
    s = s_copy;
    return true;
}