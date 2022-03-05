#include <Instruction.hpp>

void clear_stack() {
    ContextManager &contextObj = ContextManager::instance();
    while(!contextObj.consumed.empty()) {
        contextObj.consumed.pop();
    }
}

void clear_map() {
    GraphManager &graphObj = GraphManager::instance();
    graphObj.graph.back().findIdent.clear();
}

void clear_graph() {
    GraphManager &graphObj = GraphManager::instance();
    graphObj.graph.clear();
}

void initialize_map(const std::string &identName) {
    GraphManager &graphObj = GraphManager::instance();
    graphObj.graph.back().findIdent[identName] = Variable({}, 18);
}