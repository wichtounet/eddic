//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <unordered_map>

#include "tac/IntelX86CodeGenerator.hpp"
#include "tac/Program.hpp"

#include "AssemblyFileWriter.hpp"

#include "il/Labels.hpp"

using namespace eddic;

void tac::IntelX86CodeGenerator::generate(tac::Program& program, AssemblyFileWriter& writer) const {
    resetNumbering();

    for(auto& function : program.functions){
        std::unordered_map<std::shared_ptr<BasicBlock>, std::string> labels;

        for(auto& block : function->getBasicBlocks()){
            std::string label = newLabel();

            labels[block] = label;
            
            writer.stream() << label << ":" << std::endl;

            for(auto& statement : block->statements){

            }
        }
    }
}
