#include <iostream>

#include "parser_x3/SpiritParser.hpp"

//#include "GlobalContext.hpp"

using namespace eddic;

int main(int argc, char** argv){
    if(argc == 1){
        std::cout << "Not enough args" << std::endl;
        return 1;
    }

    std::string file(argv[1]);

    //auto context = std::make_shared<GlobalContext>(Platform::INTEL_X86_64);
    parser_x3::SpiritParser parser;
    //ast::SourceFile source;

    parser.parse(file/*, source, context*/);

    return 0;
}
