#include <iostream>
#include <vector>

#include "parser_x3/SpiritParser.hpp"

#include "GlobalContext.hpp"

using namespace eddic;

int main(int argc, char** args){
    const std::vector<std::string> argv(args+1, args+argc);

    for (int i=0; i<100; i++){
        for (auto& file : argv){
            auto context = std::make_shared<GlobalContext>(Platform::INTEL_X86_64);
            parser_x3::SpiritParser parser;
            ast::SourceFile source;

            parser.parse(file, source, context);
        }
    }

    return 0;
}
