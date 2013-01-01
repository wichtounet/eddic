//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Labels.hpp"
#include "FunctionContext.hpp"
#include "VisitorUtils.hpp"
#include "Platform.hpp"
#include "Type.hpp"
#include "PerfsTimer.hpp"
#include "Options.hpp"

#include "ltac/Statement.hpp"
#include "ltac/Compiler.hpp"
#include "ltac/StatementCompiler.hpp"
#include "ltac/Utils.hpp"

#include "mtac/Statement.hpp"
#include "mtac/Utils.hpp"
#include "mtac/EscapeAnalysis.hpp"

using namespace eddic;

ltac::Compiler::Compiler(Platform platform, std::shared_ptr<Configuration> configuration) : platform(platform), configuration(configuration) {}

void ltac::Compiler::compile(mtac::Program& source, std::shared_ptr<FloatPool> float_pool){
    for(auto& function : source.functions){
        compile(function, float_pool);
    }
}

void ltac::Compiler::compile(mtac::Function& function, std::shared_ptr<FloatPool> float_pool){
    PerfsTimer timer("LTAC Compilation");
    
    //Compute the block usage (in order to know if we have to output the label)
    mtac::computeBlockUsage(function, block_usage);

    resetNumbering();

    //First we computes a label for each basic block
    for(auto& block : function){
        block->label = newLabel();
    }
    
    StatementCompiler compiler(float_pool);
    compiler.descriptor = getPlatformDescriptor(platform);
    compiler.platform = platform;
    compiler.configuration = configuration;
    compiler.manager.pointer_escaped = mtac::escape_analysis(function);;
    
    //Handle parameters and register-allocated variables
    compiler.collect_parameters(function.definition());

    //Then we compile each of them
    for(auto& block : function){
        compiler.ended = false;
        compiler.bb = block;
        compiler.manager.bb = block;

        //If necessary add a label for the block
        if(block_usage.find(block) != block_usage.end()){
            compiler(std::make_shared<mtac::Quadruple>(mtac::Operator::LABEL, block->label));
        }

        visit_each(compiler, block->statements);

        compiler.end_bb();
    }

    function.set_pseudo_registers(compiler.manager.last_pseudo_reg());
    function.set_pseudo_float_registers(compiler.manager.last_float_pseudo_reg());
}
