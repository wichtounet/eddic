//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "asm/CodeGeneratorFactory.hpp"
#include "asm/IntelX86CodeGenerator.hpp"
#include "asm/IntelX86_64CodeGenerator.hpp"

using namespace eddic;

std::unique_ptr<as::CodeGenerator> eddic::as::CodeGeneratorFactory::get(Platform platform, AssemblyFileWriter& writer, mtac::Program& program, std::shared_ptr<GlobalContext> context){
    switch(platform){
        case Platform::INTEL_X86:
            return std::make_unique<as::IntelX86CodeGenerator>(writer, program, context);
        case Platform::INTEL_X86_64:
            return std::make_unique<as::IntelX86_64CodeGenerator>(writer, program, context);
    }

    return nullptr;
}
