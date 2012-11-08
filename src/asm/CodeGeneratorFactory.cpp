//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "asm/CodeGeneratorFactory.hpp"
#include "asm/IntelX86CodeGenerator.hpp"
#include "asm/IntelX86_64CodeGenerator.hpp"

using namespace eddic;

std::shared_ptr<as::CodeGenerator> eddic::as::CodeGeneratorFactory::get(Platform platform, AssemblyFileWriter& writer, std::shared_ptr<GlobalContext> context){
    switch(platform){
        case Platform::INTEL_X86:
            return std::make_shared<as::IntelX86CodeGenerator>(writer, context);
        case Platform::INTEL_X86_64:
            return std::make_shared<as::IntelX86_64CodeGenerator>(writer, context);
    }

    return nullptr;
}
