#include <Emit.hpp>
#include <stdexcept>
#include <sstream>
#include <unordered_map>

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
    contextObj.consumed.push((Number)stoi(number));
}
void emitDesignator() {
    ContextManager &contextObj = ContextManager::instance();
    GraphManager &graphObj = GraphManager::instance();
    // Get array dimensions
    std::vector<ExpId> indexes;
    while(contextObj.consumed.top().type() != typeid(std::string)) {
        indexes.push_back(std::any_cast<ExpId>(contextObj.consumed.top()));
        contextObj.consumed.pop();
    }
    reverse(indexes.begin(), indexes.end());
    // Get identName
    std::string identName = std::any_cast<std::string>(contextObj.consumed.top());
    contextObj.consumed.pop();

    // Check identName is valid or not
    if(!graphObj.graph.back().findIdent.contains(identName)) {
        throw std::invalid_argument( "identifier is not in current graph." );
    }

    if(indexes.empty()) {
        // This is normal variable
        Designator designator(identName);
        contextObj.consumed.push(designator);
    }else {
        // Calculate the memory address of array (emit instructions to basic block, add offset)
        DataType type = graphObj.graph.back().findIdent[identName].type;
        int curPtr = type.size();
        
        // The accumulate offset
        Instruct instructAccOffset(OpCode::op_cnst, 0);
        graphObj.graph.back().push_back(instructAccOffset);
        ExpId accOffset = instructAccOffset.id;
        
        // The accumulate multiply result
        Number accMul = 1;
        
        // Emit instructions and get the real offset of array
        while(curPtr > 0) {
            // Get the exp_id of current index
            ExpId curIndex = indexes[curPtr-1];
            // Put accMul into basic block
            Instruct putAccMul(OpCode::op_cnst, accMul);
            graphObj.graph.back().push_back(putAccMul);
            ExpId accMulId = putAccMul.id;
            // Multiply with accMul with curIndex and get the result
            Instruct curMulAcc(OpCode::op_mul, curIndex, accMulId);
            graphObj.graph.back().push_back(curMulAcc);
            ExpId mulResult = curMulAcc.id;
            // Update accMul
            accMul *= type[curPtr-1];
            // Update accOffset
            Instruct addWithAccOffset(OpCode::op_add, mulResult, accOffset);
            graphObj.graph.back().push_back(addWithAccOffset);
            accOffset = addWithAccOffset.id;
            // Update curPtr
            curPtr -= 1;
        }
        // Add accOffset with original offset in designator
        Instruct putOriginOffset(OpCode::op_cnst, (ExpId)graphObj.graph.back().findIdent[identName].offset.value());
        graphObj.graph.back().push_back(putOriginOffset);
        ExpId originOffset = putOriginOffset.id;
        Instruct getFinalAddress(OpCode::op_add, originOffset, accOffset);
        graphObj.graph.back().push_back(getFinalAddress);
        ExpId address = getFinalAddress.id;
        // Create designator object
        Designator designator(identName, address);
        // Push designator into consumed stack
        contextObj.consumed.push(designator);
    }
}
// Read value from stack
void emitFactor() {
    ContextManager &contextObj = ContextManager::instance();
    // Factor is Number
    if(contextObj.consumed.top().type() == typeid(Number)) {
        GraphManager &graphObj = GraphManager::instance();
        // Get number from stack
        Number number = std::any_cast<Number>(contextObj.consumed.top());
        contextObj.consumed.pop();
        // Put const instruction into basic block
        Instruct instruct(OpCode::op_cnst, (ExpId)number);
        graphObj.graph.back().push_back(instruct);
        contextObj.consumed.push(instruct.id);
    }else if(contextObj.consumed.top().type() == typeid(Designator)) {
        // Factor is designator
        GraphManager &graphObj = GraphManager::instance();
        Designator designator = std::any_cast<Designator>(contextObj.consumed.top());
        contextObj.consumed.pop();
        
        if(designator.address == std::nullopt) {
            // This is normal variable
            if(graphObj.graph.back().findIdent[designator.identName].exp_id == std::nullopt) {
                throw std::invalid_argument("The value of identifier has not been assigned.");
            }else {
                ExpId exp_id = graphObj.graph.back().findIdent[designator.identName].exp_id.value();
                contextObj.consumed.push(exp_id);
            }
        }else {
            // This is array
            Instruct getArrayValue(OpCode::op_load, designator.address);
            graphObj.graph.back().push_back(getArrayValue);
            contextObj.consumed.push(getArrayValue.id);
        }
    }
    // Expression: do nothing since the expId already in consumed stack
    //TODO:  function call
}
void emitTerm(const std::string &operators) {
    ContextManager &contextObj = ContextManager::instance();
    int factorNum = operators.size() + 1;
    // Get ids from stack
    std::stack<ExpId> factors;
    for(int i=0; i<factorNum; i++) {
        factors.push(std::any_cast<ExpId>(contextObj.consumed.top()));
        contextObj.consumed.pop();
    }
    // Put instructions into basic block
    GraphManager &graphObj = GraphManager::instance();
    for(int i=0; i<operators.size(); i++) {
        ExpId x = factors.top();
        factors.pop();
        ExpId y = factors.top();
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
    std::stack<ExpId> terms;
    // Get term ids from stack
    for(int i=0; i<termNum; i++) {
        terms.push(std::any_cast<ExpId>(contextObj.consumed.top()));
        contextObj.consumed.pop();
    }
    GraphManager &graphObj = GraphManager::instance();
    for(int i=0; i<operators.size(); i++) {
        ExpId x = terms.top();
        terms.pop();
        ExpId y = terms.top();
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
    ExpId y = std::any_cast<ExpId>(contextObj.consumed.top());
    contextObj.consumed.pop();
    RelOp symbol = std::any_cast<RelOp>(contextObj.consumed.top());
    contextObj.consumed.pop();
    ExpId x = std::any_cast<ExpId>(contextObj.consumed.top());
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
    ExpId exp_id = std::any_cast<ExpId>(contextObj.consumed.top());
    contextObj.consumed.pop();
    // Get designator from stack
    Designator designator = std::any_cast<Designator>(contextObj.consumed.top());
    contextObj.consumed.pop();
    // Test ident in the block map or not
    GraphManager &graphObj = GraphManager::instance();
    if(!graphObj.graph.back().findIdent.contains(designator.identName)) {
        std::cout << designator.identName << " does not exist!" << std::endl;
        throw std::invalid_argument("No this identifier.");
    }
    if(designator.address == std::nullopt) {
        // This is normal variable, then assign exp_id to identifier
        graphObj.graph.back().findIdent[designator.identName].exp_id = exp_id;
    }else {
        // This is array, then we need to store value represented by exp_id into memory
        Instruct storeToAddress(OpCode::op_store, exp_id, designator.address);
        graphObj.graph.back().push_back(storeToAddress);
    }
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
    ExpId cmp_id = std::any_cast<ExpId>(contextObj.consumed.top());
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
    graphObj.graph.back().dom = contextObj.blocks.top();
    contextObj.blocks.pop();
    contextObj.blocks.push((BlockId)graphObj.graph.size()-1);
}
void emitWhileStatement() {
    // Get instance of singleton class
    ContextManager &contextObj = ContextManager::instance();
    GraphManager &graphObj = GraphManager::instance();
    
    // Get two block index from consumed
    int received = 0;
    BlockId upIndex, downIndex;
    RelOp cmpOp;
    while(received < 2)
    {
        if(contextObj.consumed.top().type() == typeid(BlockId)) {
            if(received == 0) {
                downIndex = std::any_cast<BlockId>(contextObj.consumed.top());
            }else {
                upIndex = std::any_cast<BlockId>(contextObj.consumed.top());
            }
            received += 1;
        }
        else if(contextObj.consumed.top().type() == typeid(RelOp) && received == 1) {
            cmpOp = std::any_cast<RelOp>(contextObj.consumed.top());
        }
        contextObj.consumed.pop();
    }
    
    // Create new block
    std::unordered_map<std::string, Variable> tmp;
    graphObj.graph.push_back(BasicBlock(tmp));
    
    // Get two ident map of block
    std::unordered_map<std::string, Variable> upMap, downMap;
    upMap = graphObj.graph[upIndex].findIdent;
    downMap = graphObj.graph[downIndex].findIdent;

    // Get the original cmp instruction
    Instruct oldCmpInstruct = graphObj.graph[upIndex].back();
    graphObj.graph[upIndex].pop_back();

    // Compare map and add phi instructions
    std::optional<ExpId> downBranchId = std::nullopt;
    for(auto [ident, variable]: upMap) {
        if(upMap[ident].exp_id != downMap[ident].exp_id) {
            Instruct instruct(OpCode::op_phi, upMap[ident].exp_id, downMap[ident].exp_id);
            graphObj.graph[upIndex].push_back(instruct);
            graphObj.graph[upIndex].findIdent[ident].exp_id = instruct.id;
            if(downBranchId == std::nullopt) {
                downBranchId = instruct.id;
            }
            if(upMap[ident].exp_id == oldCmpInstruct.x.value()) {
                oldCmpInstruct.x = instruct.id;
            }else if(upMap[ident].exp_id == oldCmpInstruct.y.value()) {
                oldCmpInstruct.y = instruct.id;
            }
        }
    }

    // Put cmp instruct back to basic block
    graphObj.graph[upIndex].push_back(oldCmpInstruct);
    if(downBranchId == std::nullopt) {
        // No phi function
        downBranchId = oldCmpInstruct.id;
    }

    // Set the branch link
    graphObj.graph[upIndex].branch = (BlockId)graphObj.graph.size() - 1;
    graphObj.graph[downIndex].branch = upIndex;

    // Add branch instruction
    graphObj.graph[downIndex].emplace_back(OpCode::op_bra, downBranchId.value());
    ExpId branchToInstructId = graphObj.graph.back().front().id;
    ExpId cmp_id = oldCmpInstruct.id;
    switch (cmpOp) {
        case RelOp::EQ:
            graphObj.graph[upIndex].emplace_back(OpCode::op_bne, cmp_id, branchToInstructId);
            break;
        case RelOp::NEQ:
            graphObj.graph[upIndex].emplace_back(OpCode::op_beq, cmp_id, branchToInstructId);
            break;
        case RelOp::LT:
            graphObj.graph[upIndex].emplace_back(OpCode::op_bge, cmp_id, branchToInstructId);
            break;
        case RelOp::LTEQ:
            graphObj.graph[upIndex].emplace_back(OpCode::op_bgt, cmp_id, branchToInstructId);
            break;
        case RelOp::GT:
            graphObj.graph[upIndex].emplace_back(OpCode::op_ble, cmp_id, branchToInstructId);
            break;
        case RelOp::GTEQ:
            graphObj.graph[upIndex].emplace_back(OpCode::op_blt, cmp_id, branchToInstructId);
            break;
    }
    
    // Update the map of head block and new block
    graphObj.graph.back().findIdent = graphObj.graph[upIndex].findIdent;

    // Pop block from blocks stack and push new block into stack
    graphObj.graph.back().dom =  contextObj.blocks.top();
    contextObj.blocks.pop();
    contextObj.blocks.push((BlockId)graphObj.graph.size() - 1);

    // Push new block id into consumed stack
    contextObj.consumed.push((BlockId)graphObj.graph.size() - 1);
}
void startStatSequence() {
    GraphManager &graphObj = GraphManager::instance();
    ContextManager &contextObj = ContextManager::instance();
    if(contextObj.blocks.empty()) {
        graphObj.graph.push_back(BasicBlock(graphObj.graph.findIdent));
    }else {
        BlockId lastIndex = contextObj.blocks.top();
        // In state sequence nest
        graphObj.graph.push_back(BasicBlock(graphObj.graph[lastIndex].findIdent));

        // Connect new block with last block
        if(graphObj.graph[lastIndex].fall_through == std::nullopt) {
            graphObj.graph[lastIndex].fall_through = (BlockId)graphObj.graph.size() - 1;
        }else {
            graphObj.graph[lastIndex].branch = (BlockId)graphObj.graph.size() - 1;
        }
        // Build dom link
        graphObj.graph.back().dom = lastIndex;
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
        type.push_back(std::any_cast<Number>(contextObj.consumed.top()));
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
    reverse(identNames.begin(), identNames.end());
    DataType type = std::any_cast<DataType>(contextObj.consumed.top());
    contextObj.consumed.pop();
    // Put ident into map in the graph
    GraphManager &graphObj = GraphManager::instance();
    if(!type.empty()) {
        // This is array, then count size
        Number arrSize = 1;
        for(Number number: type) {
            arrSize *= number;
        }
        for(auto identName: identNames) {
            graphObj.graph.findIdent[identName] = Variable(type, arrSize);   
        }
    }else {
        // This is normal variable
        for(auto identName: identNames) {
            graphObj.graph.findIdent[identName] = Variable(type);   
        }
    }
}
void emitComputation() {
    rewrite_after_loop();
}
void rewrite_after_loop() {
    // Get instance from singleton class
    GraphManager &graphObj = GraphManager::instance();
    ContextManager &contextObj = ContextManager::instance();

    // Rewrite from root
    std::optional<BlockId> curIndex = 0;
    std::unordered_map<ExpId, ExpId> lastMap;
    while(curIndex != std::nullopt) {
        // Build curMap before rewriting
        std::unordered_map<ExpId, ExpId> curMap;
        for(auto &instruct: graphObj.graph[curIndex.value()]) {
            if(instruct.opcode == OpCode::op_phi) {
                curMap[instruct.x.value()] = instruct.id;
                curMap[instruct.y.value()] = instruct.id;
            }
            if(instruct.x != std::nullopt && lastMap.count(instruct.x.value())) {
                instruct.x = lastMap[instruct.x.value()];
            }
            if(instruct.y != std::nullopt && lastMap.count(instruct.y.value())) {
                instruct.y = lastMap[instruct.y.value()];
            }
        }
        lastMap = curMap;
        curIndex = graphObj.graph[curIndex.value()].fall_through;
    }
}
void common_subexpression_elimination() {
    // Get instance from singleton class
    GraphManager &graphObj = GraphManager::instance();
    ContextManager &contextObj = ContextManager::instance();

    std::unordered_map<ExpId, ExpId> replace;
    while(true) {
        unsigned oidSize = replace.size();
        // For each basic block, do common subexpression elimination
        for(int i=0; i<graphObj.graph.size(); i++) {
            // Build the set of dominated blocks' instruction
            std::unordered_map<std::string, Instruct> domInstruct;
            BlockId curIndex = i;
            while(graphObj.graph[curIndex].dom != std::nullopt) {
                curIndex = graphObj.graph[curIndex].dom.value();
                for(auto instruct: graphObj.graph[curIndex]) {
                    domInstruct[opCode_to_string(instruct)] = instruct;
                }
            }
            // Compare current block's instruction with domInstruct
            BasicBlock block = graphObj.graph[i];
            block.clear();
            for(auto instruct: graphObj.graph[i]) {
                // Test and replace the deleted id
                if(instruct.x != std::nullopt && replace.count(instruct.x.value())) {
                    instruct.x = replace[instruct.x.value()];
                }
                if(instruct.y != std::nullopt && replace.count(instruct.y.value())) {
                    instruct.y = replace[instruct.y.value()];
                }
                std::string instructStr = opCode_to_string(instruct);
                if((domInstruct.count(instructStr) && instruct.opcode != OpCode::op_init)) {
                    // Repeated, then we can not push into block
                    ExpId deleted_id = instruct.id, dom_id = domInstruct[instructStr].id;
                    replace[deleted_id] = dom_id;
                }else if(instruct.opcode == OpCode::op_phi && instruct.x == instruct.y) {
                    ExpId deleted_id = instruct.id, replace_id = instruct.x.value();
                    replace[deleted_id] = replace_id;
                }else {
                    // No need to eliminate, then we can push into block
                    block.push_back(instruct);
                    domInstruct[opCode_to_string(instruct)] = instruct;
                }
            }
            // Replace graph[i] with block
            graphObj.graph[i] = block;
        }
        if(oidSize == replace.size()) {
            break;
        }
    }
}
std::string opCode_to_string(Instruct instruct) {
    std::stringstream ss;
    switch (instruct.opcode) {
        case OpCode::op_neg:
			ss << "neg " << "(" << instruct.x.value() << ")";
			break;
		case OpCode::op_add:
			ss << "add " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_sub:
			ss << "sub " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_mul:
			ss << "mul " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_div:
			ss << "div " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_cmp:
			ss << "cmp " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_adda:
			ss << "adda " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_load:
			ss << "load " << "(" << instruct.x.value() << ")";
			break;
		case OpCode::op_store:
			ss << "store " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_phi:
			ss << "phi " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_end:
			ss << "end ";
			break;
		case OpCode::op_bra:
			ss << "bra " << "(" << instruct.x.value() << ")";
			break;
		case OpCode::op_bne:
			ss << "bne " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_beq:
			ss << "beq " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_ble:
			ss << "ble " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_blt:
			ss << "blt " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_bge:
			ss << "bge " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_bgt:
			ss << "bgt " << "(" << instruct.x.value() << ")" << " " << "(" << instruct.y.value() << ")";
			break;
		case OpCode::op_read:
			ss << "read ";
			break;
		case OpCode::op_write:
			ss << "write " << "(" << instruct.x.value() << ")";
			break;
		case OpCode::op_writeNL:
			ss << "writeNL ";
			break;
		case OpCode::op_cnst:
			ss << "const " << "#" << instruct.x.value();
			break;
		case OpCode::op_init:
			ss << "\\<init\\>";
        default:
            ss << "";
    }
    return ss.str();
}
