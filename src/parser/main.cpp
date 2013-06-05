#include "GlobalContext.hpp"
#include "ast/SourceFile.hpp"
#include "parser/SpiritParser.hpp"

int main(int argc, char* args[])
{
    const std::vector<std::string> argv(args+1, args+argc);

    using namespace eddic;
    using namespace eddic::parser;

    for (int i=0; i<5; i++)
        for (auto& fname : argv)
        {
            SpiritParser parser;

            eddic::ast::SourceFile program;
            parser.parse(fname, program, std::make_shared<GlobalContext>(Platform::INTEL_X86_64));
        }
}
