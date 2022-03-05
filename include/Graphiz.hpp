#ifndef GRAPHIZ_GUARD
#define GRAPHIZ_GUARD

#include <string>
#include <iostream>
#include <fstream>
#include <Instruction.hpp>

class PrintGraph {
public:
    std::string fileName;
    std::ofstream out;
    PrintGraph(const std::string fileName="graphiz.dot");
    std::string opCode_to_string(Instruct instruct);
    void print();
};

#endif