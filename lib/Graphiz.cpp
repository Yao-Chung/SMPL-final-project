#include <Graphiz.hpp>

PrintGraph::PrintGraph(std::string fileName) {
    out = std::ofstream(fileName, std::ios::out | std::ios::trunc);
}

void PrintGraph::print() {
	GraphManager &graphObj = GraphManager::instance();
	
    out << "digraph G {\n";
	for(auto [funcName, curGraph]: graphObj.funcToGraph) {
		graphObj.graph = curGraph;
		unsigned blockNum = graphObj.graph.size();
		// Print basic block
		for(unsigned i=0; i<blockNum; ++i) {
			if(i > 0) {
				out << "\n" << funcName << "bb" << (i+1) << " [shape=record,label=\"<b>BB" << (i+1) << "|";
			}else {
				out << "\n" << funcName << "bb" << (i+1) << " [shape=record,label=\"<b>" << graphObj.graph.funcName << "\\n" << "BB" << (i+1) << "|";
			}
			int instructNum = graphObj.graph[i].size();
			// Print instruction
			out << "{";
			for(unsigned j=0; j<instructNum; ++j) {
				out << "<" << graphObj.graph[i][j].id << "> " << graphObj.graph[i][j].id << ": " << opCode_to_string(graphObj.graph[i][j]);
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
				out << funcName << "bb" << (i+1) << ":s -> " << funcName << "bb" << graphObj.graph[block.fall_through.value()].id << ":n ";
				out << "[label=\"fall-through\"];\n";
			}
			if(block.branch != std::nullopt) {
				out << funcName << "bb" << (i+1) << ":s -> " << funcName << "bb" << graphObj.graph[block.branch.value()].id << ":n ";
				out << "[label=\"branch\"];\n";
			}
			if(block.dom != std::nullopt) {
				out << funcName << "bb" << (block.dom.value() + 1) << ":b -> " << funcName << "bb" << (i+1) << ":b";
				out << "[color=blue, style=solid, label=\"dom\"]\n";
			}
		}
	}
	// Build function branch arrow
	for(auto [funcName, curGraph]: graphObj.funcToGraph) {
		for(auto arrow: curGraph.arrows) {
			// Check function void expectation on arrow match with real situation
			if(arrow.expectVoid != graphObj.funcToGraph[arrow.destination].isVoid) {
				throw std::invalid_argument("expectVoid conflicts with isVoid");
			}
			out << funcName << "bb" << (arrow.blockIndex + 1) << ":" << arrow.sourceId << " -> " << arrow.destination << "bb1" << ":n ";
			// Print parameters on arrow
			std::string all_parameters;
			all_parameters += "(";
			for(auto exp_id: arrow.paraExpIds) {
				all_parameters += (std::to_string(exp_id.first) + " => " + std::to_string(exp_id.second) +",");
			}
			if(all_parameters != "(") {
				all_parameters.pop_back();
			}
			all_parameters += ")";
			out << "[color=green, style=solid, label=\"call " << all_parameters << "\"]\n";
		}
	}
	out << "\n}";
}