#include <Graphiz.hpp>

PrintGraph::PrintGraph(std::string fileName) {
    out = std::ofstream(fileName, std::ios::out | std::ios::trunc);
}

void PrintGraph::print() {
    out << "digraph G {\n"; 
    GraphManager &graphObj = GraphManager::instance();
    unsigned blockNum = graphObj.graph.size();
    // Print basic block
    for(unsigned i=0; i<blockNum; ++i) {
        out << "\nbb" << (i+1) << " [shape=record,label=\"<b>BB" << (i+1) << "|";
        int instructNum = graphObj.graph[i].size();
        // Print instruction
        out << "{";
        for(unsigned j=0; j<instructNum; ++j) {
            out << graphObj.graph[i][j].id << ": " << opCode_to_string(graphObj.graph[i][j]);
            if(j < instructNum-1) {
                out << "|";
            }
        }
        out << "}\"];\n";
    }
	// Connect edge between blocks
	for(int i=0; i<blockNum; i++) {
		BasicBlock block = graphObj.graph[i];
		if(block.fall_through != std::nullopt) {
			out << "bb" << (i+1) << ":s -> " << "bb" << graphObj.graph[block.fall_through.value()].id << ":n ";
			out << "[label=\"fall-through\"];\n";
		}
		if(block.branch != std::nullopt) {
			out << "bb" << (i+1) << ":s -> " << "bb" << graphObj.graph[block.branch.value()].id << ":n ";
			out << "[label=\"branch\"];\n";
		}
		if(block.dom != std::nullopt) {
			out << "bb" << (block.dom.value() + 1) << ":b -> " << "bb" << (i+1) << ":b";
			out << "[color=blue, style=solid, label=\"dom\"]\n";
		}
	}
    out << "\n}";
}