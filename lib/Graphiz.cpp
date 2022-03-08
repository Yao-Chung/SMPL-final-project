#include <Graphiz.hpp>
#include <sstream>

std::string PrintGraph::opCode_to_string(Instruct instruct) {
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