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

using ExpId = int;                      // id for instruction in basic block
using Number = unsigned long;
using DataType = std::vector<Number>;      // Empty vector => var, otherwise => array
using BlockId = unsigned;

// Data structure of each instruction
class Instruct {
public:
    OpCode opcode;
    std::optional<ExpId> x;
    std::optional<ExpId> y;
    ExpId id;
    Instruct(OpCode opcode = OpCode::op_init, std::optional<ExpId> x = std::nullopt, std::optional<ExpId> y = std::nullopt);
};

// The attribute of variable
class Variable {
public:
    DataType type;
    // std::optional<int> offset;
    std::optional<ExpId> exp_id;
    Variable(DataType type = {}, std::optional<ExpId> exp_id = std::nullopt);
};

class Designator {
public:
    std::string identName;
    std::vector<ExpId> dims;
    Designator(std::string identName = "", std::vector<ExpId> dims = {});
};

// Data structure of a basic block
class BasicBlock : public std::vector<Instruct> {
public:
    int id;
    std::unordered_map<std::string, Variable> findIdent;
    std::optional<BlockId> fall_through;
    std::optional<BlockId> branch;
    std::optional<BlockId> dom;
    BasicBlock(std::unordered_map<std::string, Variable> findIdent, 
               std::optional<BlockId> fall_through = std::nullopt,
               std::optional<BlockId> branch = std::nullopt,
               std::optional<BlockId> dom = std::nullopt);
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