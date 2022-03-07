#include <Emit.hpp>
#include <Instruction.hpp>
#include <stdexcept>

void emitDigit(const char &c) {
    ContextManager &contextObj = ContextManager::instance();
    contextObj.consumed.push(c);
}
void emitLetter(const char &c) {
    ContextManager &contextObj = ContextManager::instance();
    contextObj.consumed.push(c);
}
void emitRelOp(const std::string &symbol) {
    ContextManager &contextObj = ContextManager::instance();
    if(symbol == "==") {
        contextObj.consumed.push(RelOp::EQ);
    }else if(symbol == "!=") {
        contextObj.consumed.push(RelOp::NEQ);
    }else if(symbol == "<") {
        contextObj.consumed.push(RelOp::LT);
    }else if(symbol == "<=") {
        contextObj.consumed.push(RelOp::LTEQ);
    }else if(symbol == ">") {
        contextObj.consumed.push(RelOp::GT);
    }else {
        contextObj.consumed.push(RelOp::GTEQ);
    }
}
void emitIdent() {
    ContextManager &contextObj = ContextManager::instance();
    std::string identName;
    // Pop all character
    while(!contextObj.consumed.empty() && contextObj.consumed.top().type() == typeid(char)) {
        identName = std::any_cast<char>(contextObj.consumed.top()) + identName;
        contextObj.consumed.pop();
    }
    // Put back into stack
    contextObj.consumed.push(identName);
}
void emitNumber() {
    // std::cout << "emit number" << std::endl;
    ContextManager &contextObj = ContextManager::instance();
    std::string number;
    while(!contextObj.consumed.empty() && contextObj.consumed.top().type() == typeid(char)
          && isdigit(std::any_cast<char>(contextObj.consumed.top()))) {
        number = std::any_cast<char>(contextObj.consumed.top()) + number;
        contextObj.consumed.pop();
    }
    // std::cout << "Put " << number << " back into stack" << std::endl;
    // Put number back into stack
    contextObj.consumed.push(stoi(number));
}
void emitDesignator() {
    //TODO: handle array
}
// Read value from stack
void emitFactor() {
    ContextManager &contextObj = ContextManager::instance();
    // Factor is Number
    if(contextObj.consumed.top().type() == typeid(int)) {
        GraphManager &graphObj = GraphManager::instance();
        // Get number from stack
        int number = std::any_cast<int>(contextObj.consumed.top());
        contextObj.consumed.pop();
        // Put const instruction into basic block
        Instruct instruct(OpCode::op_cnst, number);
        graphObj.graph.back().push_back(instruct);
        contextObj.consumed.push(instruct.id);
    }else if(contextObj.consumed.top().type() == typeid(std::string)) {
        // Factor is designator
        GraphManager &graphObj = GraphManager::instance();
        std::string identName = std::any_cast<std::string>(contextObj.consumed.top());
        contextObj.consumed.pop();
        if(!graphObj.graph.back().findIdent.contains(identName)) {
            throw std::invalid_argument( "identifier is not in current graph." );
        }else if(graphObj.graph.back().findIdent[identName].exp_id == std::nullopt) {
            throw std::invalid_argument("The value of identifier has not been assigned.");
        }else {
            int exp_id = graphObj.graph.back().findIdent[identName].exp_id.value();
            contextObj.consumed.push(exp_id);
        }
    }
    //TODO: expression and function call
}
void emitTerm(const std::string &operators) {
    ContextManager &contextObj = ContextManager::instance();
    int factorNum = operators.size() + 1;
    // Get ids from stack
    std::stack<int> factors;
    for(int i=0; i<factorNum; i++) {
        factors.push(std::any_cast<int>(contextObj.consumed.top()));
        contextObj.consumed.pop();
    }
    // Put instructions into basic block
    GraphManager &graphObj = GraphManager::instance();
    for(int i=0; i<operators.size(); i++) {
        int x = factors.top();
        factors.pop();
        int y = factors.top();
        factors.pop();
        if(operators[i] == '*') {
            Instruct instruct(OpCode::op_mul, x, y);
            graphObj.graph.back().push_back(instruct);
            factors.push(instruct.id);
        }else {
            Instruct instruct(OpCode::op_div, x, y);
            graphObj.graph.back().push_back(instruct);
            factors.push(instruct.id);
        }
    }
    // Put last id into context
    contextObj.consumed.push(factors.top());
}
void emitExpression(const std::string &operators) {
    ContextManager &contextObj = ContextManager::instance();
    int termNum = operators.size() + 1;
    std::stack<int> terms;
    // Get term ids from stack
    for(int i=0; i<termNum; i++) {
        terms.push(std::any_cast<int>(contextObj.consumed.top()));
        contextObj.consumed.pop();
    }
    GraphManager &graphObj = GraphManager::instance();
    for(int i=0; i<operators.size(); i++) {
        int x = terms.top();
        terms.pop();
        int y = terms.top();
        terms.pop();
        if(operators[i] == '+') {
            Instruct instruct(OpCode::op_add, x, y);
            graphObj.graph.back().push_back(instruct);
            terms.push(instruct.id);
        }else {
            Instruct instruct(OpCode::op_sub, x, y);
            graphObj.graph.back().push_back(instruct);
            terms.push(instruct.id);
        }
    }
    // Put last id into context
    contextObj.consumed.push(terms.top());
}
void emitRelation() {
    ContextManager &contextObj = ContextManager::instance();
    int y = std::any_cast<int>(contextObj.consumed.top());
    contextObj.consumed.pop();
    RelOp symbol = std::any_cast<RelOp>(contextObj.consumed.top());
    contextObj.consumed.pop();
    int x = std::any_cast<int>(contextObj.consumed.top());
    contextObj.consumed.pop();
    Instruct instruct(OpCode::op_cmp, x, y);
    // Put instruction into basic block
    GraphManager &graphObj = GraphManager::instance();
    graphObj.graph.back().push_back(instruct);
    // Put cmp id into stack
    contextObj.consumed.push(instruct.id);
    // Put compare op into stack
    contextObj.consumed.push(symbol);
}
void emitAssignment() {
    ContextManager &contextObj = ContextManager::instance();
    // Get expression from stack
    int exp_id = std::any_cast<int>(contextObj.consumed.top());
    contextObj.consumed.pop();
    // Get designator from stack
    std::string identName = std::any_cast<std::string>(contextObj.consumed.top());
    contextObj.consumed.pop();
    // Test ident in the block map or not
    GraphManager &graphObj = GraphManager::instance();
    if(!graphObj.graph.back().findIdent.contains(identName)) {
        std::cout << identName << std::endl;
        throw std::invalid_argument("No this identifier.");
    }
    // Assign exp_id to identifier
    graphObj.graph.back().findIdent[identName].exp_id = exp_id;
}
void emitIfStatement() {
    ContextManager &contextObj = ContextManager::instance();
    GraphManager &graphObj = GraphManager::instance();
    
    // Create a Basicblock and put it into graph
    std::unordered_map<std::string, Variable> tmpMap;
    BasicBlock jointBlock(tmpMap);
    graphObj.graph.push_back(jointBlock);
    
    // Build link when poping consumed
    std::unordered_map<std::string, Variable> mpFall, mpBranch;
    int linkCnt = 0;
    BlockId branchIndex;
    while(contextObj.consumed.top().type() != typeid(RelOp)) {
        // Encounter a block
        if(contextObj.consumed.top().type() == typeid(BlockId)) {
            BlockId blockIndex = std::any_cast<BlockId>(contextObj.consumed.top());
            contextObj.consumed.pop();
            if(linkCnt == 0) {
                graphObj.graph[blockIndex].fall_through = (BlockId)graphObj.graph.size() - 1;
                mpFall = graphObj.graph[blockIndex].findIdent;
                linkCnt += 1;
            }else {
                graphObj.graph[blockIndex].branch = (BlockId)graphObj.graph.size() - 1;
                mpBranch = graphObj.graph[blockIndex].findIdent;
                branchIndex = blockIndex;
                linkCnt += 1;
            }
        }else {
            contextObj.consumed.pop();
        }
    }
    RelOp cmpOp = std::any_cast<RelOp>(contextObj.consumed.top());
    contextObj.consumed.pop();
    int cmp_id = std::any_cast<int>(contextObj.consumed.top());
    contextObj.consumed.pop();
    BlockId headIndex = std::any_cast<BlockId>(contextObj.consumed.top());
    contextObj.consumed.pop();
    // Push current block into consumed
    contextObj.consumed.push((BlockId)graphObj.graph.size()-1);
    // Test link is all set or not
    if(linkCnt < 2) {
        graphObj.graph[headIndex].branch = (BlockId)graphObj.graph.size() - 1;
        mpBranch = graphObj.graph[headIndex].findIdent;
        branchIndex = headIndex;
        linkCnt += 1;
    }

    // Update its own map
    graphObj.graph.back().findIdent = graphObj.graph[headIndex].findIdent;
    // Compare map and add phi instruction into block (end of graph)
    for(auto [ident, variable]: graphObj.graph[headIndex].findIdent) {
        if(mpFall[ident].exp_id != mpBranch[ident].exp_id) {
            // Push phi instruction into block
            Instruct instruct(OpCode::op_phi, mpFall[ident].exp_id, mpBranch[ident].exp_id);
            graphObj.graph.back().push_back(instruct);
            // Use id of phi instruction as new exp_id
            graphObj.graph.back().findIdent[ident].exp_id = instruct.id;
        }
    }
    
    // Add branch instruction into block (blockIndex)
    // Head branch
    BlockId branchToId = graphObj.graph[headIndex].branch.value();
    switch (cmpOp) {
        case RelOp::EQ:
            graphObj.graph[headIndex].emplace_back(OpCode::op_bne, cmp_id, graphObj.graph[branchToId].front().id);
            break;
        case RelOp::NEQ:
            graphObj.graph[headIndex].emplace_back(OpCode::op_beq, cmp_id, graphObj.graph[branchToId].front().id);
            break;
        case RelOp::LT:
            graphObj.graph[headIndex].emplace_back(OpCode::op_bge, cmp_id, graphObj.graph[branchToId].front().id);
            break;
        case RelOp::LTEQ:
            graphObj.graph[headIndex].emplace_back(OpCode::op_bgt, cmp_id, graphObj.graph[branchToId].front().id);
            break;
        case RelOp::GT:
            graphObj.graph[headIndex].emplace_back(OpCode::op_ble, cmp_id, graphObj.graph[branchToId].front().id);
            break;
        case RelOp::GTEQ:
            graphObj.graph[headIndex].emplace_back(OpCode::op_blt, cmp_id, graphObj.graph[branchToId].front().id);
            break;
    }

    // branch to joint
    if(headIndex != branchIndex) {
        graphObj.graph[branchIndex].emplace_back(OpCode::op_bra, graphObj.graph.back().front().id);
    }

    // Pop block from blocks stack and push itself
    contextObj.blocks.pop();
    contextObj.blocks.push((BlockId)graphObj.graph.size()-1);
}
void startStatSequence() {
    GraphManager &graphObj = GraphManager::instance();
    ContextManager &contextObj = ContextManager::instance();
    if(contextObj.blocks.empty()) {
        graphObj.graph.push_back(BasicBlock(graphObj.graph.findIdent));
    }else {
        unsigned lastIndex = contextObj.blocks.top();
        // In state sequence nest
        graphObj.graph.push_back(BasicBlock(graphObj.graph[lastIndex].findIdent));

        // Connect new block with last block
        if(graphObj.graph[lastIndex].fall_through == std::nullopt) {
            graphObj.graph[lastIndex].fall_through = (BlockId)graphObj.graph.size() - 1;
        }else {
            graphObj.graph[lastIndex].branch = (BlockId)graphObj.graph.size() - 1;
        }
    }
    // Push index of basic block into stack
    // The type of index is unsigned
    contextObj.consumed.push((BlockId)graphObj.graph.size() - 1);
    contextObj.blocks.push((BlockId)graphObj.graph.size() - 1);
}
void endStatSequence() {
    ContextManager &contextObj = ContextManager::instance();
    contextObj.blocks.pop();
}
void emitTypeDecl(const int &dimNumber) {
    // std::cout << "emit emitTypeDecl: " << std::endl;
    ContextManager &contextObj = ContextManager::instance();
    DataType type;
    for(int i=0; i<dimNumber; i++) {
        type.push_back(std::any_cast<int>(contextObj.consumed.top()));
        contextObj.consumed.pop();
    }
    // Correct the order
    reverse(type.begin(), type.end());
    // std::cout << "Put vector of int back to stack" << std::endl;
    contextObj.consumed.push(type);
}
void emitVarDecl() {
    ContextManager &contextObj = ContextManager::instance();
    std::vector<std::string> identNames;
    while(!contextObj.consumed.empty() && contextObj.consumed.top().type() != typeid(DataType)) {
        identNames.push_back(std::any_cast<std::string>(contextObj.consumed.top()));
        contextObj.consumed.pop();
    }
    DataType type = std::any_cast<DataType>(contextObj.consumed.top());
    contextObj.consumed.pop();
    // Put ident into map in the graph
    GraphManager &graphObj = GraphManager::instance();
    for(auto identName: identNames) {
        graphObj.graph.findIdent[identName] = Variable(type);   
    }
}
void emitComputation() {

}

