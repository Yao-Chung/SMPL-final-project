#include <Instruction.hpp>


Instruct::Instruct(OpCode opcode, std::optional<int> x, std::optional<int> y) : 
opcode(opcode), x(x), y(y) {
    static int id = 0;
    id += 1;
    this->id = id;
}

BasicBlock::BasicBlock(std::unordered_map<std::string, Variable> findIdent) :
findIdent(findIdent) {
    Instruct instruct(OpCode::op_init);
    this->push_back(instruct);
    static int id = 0;
    id += 1;
    this->id = id;
}

Variable::Variable(DataType type, std::optional<int> exp_id) : type(type), exp_id(exp_id) {};

GraphManager &GraphManager::instance() {
    static GraphManager single_instance;
    return single_instance;
}

// RC GraphManager::addBasicBlock() {
//     BasicBlock newBlock;
//     graph.push_back(newBlock);
//     return 0;
// }

// RC GraphManager::putVariable(const std::string &identName, const Variable &variable) {
//     // Ident already exists
//     if(graph.findIdent.contains(identName)) {
//         return -1;
//     }
//     graph.findIdent[identName] = variable;
//     return 0;
// }

ContextManager &ContextManager::instance() {
    // Using default constructor
    static ContextManager single_instance;
    return single_instance;
}
