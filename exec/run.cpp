#include <Emit.hpp>
#include <Graphiz.hpp>
#include <Parser.hpp>
#include <Instruction.hpp>
#include <iostream>
#include <string>

// #define USE_CSE

int main(int argc, char *argv[]) {
    if(argc != 3) {
        std::cout << "usage: ./run input.txt output.txt" << std::endl;
        return 1;
    }
    std::string inputFile(argv[1]);
    std::string outputFile(argv[2]);
    std::string code;
    if(readFile(inputFile, code) == -1) {
        std::cout << "Read file error" << std::endl;
        return 1;
    }
    std::string_view source(code);
    if(!isComputation(source)) {
        std::cout << "Parsing error" << std::endl;
        return 1;
    }

#ifdef USE_CSE
    common_subexpression_elimination();
#endif

    PrintGraph printGraph(outputFile);
    printGraph.print();
    GraphManager &graphObj = GraphManager::instance();
    for(auto p: graphObj.graph.back().findIdent) {
        std::cout << p.first << " <- " << p.second.exp_id.value() << std::endl; 
    }
    return 0;
}