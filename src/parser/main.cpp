#include "GlobalContext.hpp"
#include "ast/SourceFile.hpp"
#include "parser/SpiritParser.hpp"

int main(int argc, char* args[])
{
    const std::vector<std::string> argv(args+1, args+argc);
    using namespace eddic::parser;
    SpiritParser parser;

    for (int i=0; i<100; i++)
        for (auto& fname : argv)
    {
        eddic::ast::SourceFile program;
        std::cout << fname << ": " << std::boolalpha << parser.parse(fname, program, nullptr) << "\n";
    }
}
