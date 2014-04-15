#include "boost_cfg.hpp"

#include "parser_x3/SpiritParser.hpp"

#include "GlobalContext.hpp"

using namespace eddic;

int main(int argc, char** argv){
    if(argc == 1){
        std::cout << "Not enough args" << std::endl;
        return 1;
    }

    std::string file(argv[1]);

    auto context = std::make_shared<GlobalContext>(Platform::INTEL_X86_64);
    parser_x3::SpiritParser parser;
    ast::SourceFile source;

    if(parser.parse(file, source, context)){
        std::cout << "succeeded" << std::endl;
    } else {
        std::cout << "failed" << std::endl;
    }

    return 0;
}
