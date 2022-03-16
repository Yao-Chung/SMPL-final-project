#include <Instruction.hpp>


Instruct::Instruct(OpCode opcode, std::optional<ExpId> x, std::optional<ExpId> y) : 
opcode(opcode), x(x), y(y) {
    static ExpId id = 0;
    id += 1;
    this->id = id;
}

BasicBlock::BasicBlock(std::unordered_map<std::string, Variable> findIdent, std::optional<BlockId> fall_through, std::optional<BlockId> branch, std::optional<BlockId> dom) :
findIdent(findIdent), fall_through(fall_through), branch(branch), dom(dom) {
    Instruct instruct(OpCode::op_init);
    this->push_back(instruct);
    static int id = 0;
    id += 1;
    this->id = id;
}

Variable::Variable(DataType type, std::optional<Number> arrSize, std::optional<ExpId> exp_id) {
    this->type = type;
    this->exp_id = exp_id;
    static Number cur = 0;
    if(arrSize != std::nullopt) {
        // This variable is array
        this->offset = cur;
        cur += arrSize.value();
    }
};

Designator::Designator(std::string identName, std::optional<ExpId> address) : identName(identName), address(address) {};

FuncReturnExpId::FuncReturnExpId(ExpId exp_id) : exp_id(exp_id) {};

FuncArrow::FuncArrow(std::string destination, ExpId sourceId, BlockId blockIndex, std::vector<std::pair<ExpId, ExpId>> paraExpIds, bool expectVoid) : destination(destination), sourceId(sourceId), blockIndex(blockIndex), paraExpIds(paraExpIds), expectVoid(expectVoid) {};

GraphManager &GraphManager::instance() {
    static GraphManager single_instance;
    return single_instance;
}

ContextManager &ContextManager::instance() {
    // Using default constructor
    static ContextManager single_instance;
    return single_instance;
}
