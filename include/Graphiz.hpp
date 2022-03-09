#ifndef GRAPHIZ_GUARD
#define GRAPHIZ_GUARD

#include <string>
#include <iostream>
#include <fstream>
#include <Emit.hpp>

class PrintGraph {
public:
    std::string fileName;
    std::ofstream out;
    PrintGraph(const std::string fileName="graphiz.dot");
    void print();
};

#endif