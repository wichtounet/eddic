//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <boost/variant.hpp>

#include "asm/IntelCodeGenerator.hpp"
#include "asm/CodeGeneratorFactory.hpp"

#include "tac/Printer.hpp"

#include "AssemblyFileWriter.hpp"
#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "StringPool.hpp"

#include "Labels.hpp"

using namespace eddic;

as::IntelCodeGenerator::IntelCodeGenerator(AssemblyFileWriter& w) : writer(w) {}

void as::IntelCodeGenerator::generate(tac::Program& program, StringPool& pool, FunctionTable& table, Platform platform){
    CodeGeneratorFactory factory;
    auto generator = factory.get(platform, writer);

    resetNumbering();

    generator->writeRuntimeSupport(table); 

    for(auto& function : program.functions){
        generator->compile(function);
    }

    generator->addStandardFunctions();

    generator->addGlobalVariables(program.context, pool);
}
