#ifndef INSTRUCTION_GUARD
#define INSTRUCTION_GUARD

#include <Parser.hpp>
#include <optional>
#include <vector>
#include <unordered_map>
#include <string>
#include <any>
#include <stack>

enum OpCode {
    op_neg, op_add, op_sub, op_mul, op_div,
    op_cmp, op_adda, op_load, op_store, op_phi,
    op_end, op_bra, op_bne, op_beq, op_ble,
    op_blt, op_bge, op_bgt, op_read, op_write,
    op_writeNL, op_cnst, op_init
};

enum class RelOp {
    EQ, NEQ, LT, LTEQ, GT, GTEQ
};

// Data structure of each instruction
class Instruct {
public:
    OpCode opcode;
    std::optional<int> x;
    std::optional<int> y;
    int id;
    Instruct(OpCode opcode, std::optional<int> x = std::nullopt, std::optional<int> y = std::nullopt);
};

using DataType = std::vector<int>;      // Empty vector => var, otherwise => array
using BlockId = unsigned;

// The attribute of variable
class Variable {
public:
    DataType type;
    std::optional<int> exp_id;
    Variable(DataType type = {}, std::optional<int> exp_id = std::nullopt);
};

// Data structure of a basic block
class BasicBlock : public std::vector<Instruct> {
public:
    int id;
    std::unordered_map<std::string, Variable> findIdent;
    std::optional<BlockId> fall_through;
    std::optional<BlockId> branch;
    BasicBlock(std::unordered_map<std::string, Variable> findIdent, 
               std::optional<BlockId> fall_through = std::nullopt,
               std::optional<BlockId> branch = std::nullopt);
};

// Data structure of whole graph
class Graph : public std::vector<BasicBlock> {
public:
    // map ident -> class (type, optional expression id)
    std::unordered_map<std::string, Variable> findIdent;
};

// Singleton manager
class GraphManager {
public:
    static GraphManager &instance();
    Graph graph;
    // RC addBasicBlock();
    // RC putVariable(const std::string &identName, const Variable &variable);
protected:
    GraphManager() = default;
};

// Collect context when parsing
class ContextManager {
public:
    static ContextManager &instance();
    std::stack<std::any> consumed;      // Put items when parsing
    std::stack<BlockId> blocks;
protected:
    ContextManager() = default;
};

#endif