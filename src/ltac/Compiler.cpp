//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "Labels.hpp"
#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "VisitorUtils.hpp"
#include "Platform.hpp"
#include "Type.hpp"
#include "PerfsTimer.hpp"
#include "Options.hpp"
#include "logging.hpp"

#include "ltac/Compiler.hpp"
#include "ltac/StatementCompiler.hpp"
#include "ltac/Utils.hpp"

#include "mtac/Quadruple.hpp"
#include "mtac/Utils.hpp"
#include "mtac/EscapeAnalysis.hpp"

using namespace eddic;

ltac::Compiler::Compiler(Platform platform, std::shared_ptr<Configuration> configuration) : platform(platform), configuration(configuration) {}

void ltac::Compiler::compile(mtac::Program& source, FloatPool& float_pool){
    timing_timer timer(source.context->timing(), "ltac_compilation");

    for(auto& function : source.functions){
        compile(function, float_pool);
    }
}

void ltac::Compiler::compile(mtac::Function& function, FloatPool& float_pool){
    log::emit<Trace>("Compiler") << "Compile LTAC for function " << function.get_name() << log::endl;
    
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
            mtac::Quadruple fake_label(block->label, mtac::Operator::LABEL);
            compiler.compile(fake_label);
        }

        for(auto& quadruple : block->statements){
           compiler.compile(quadruple); 
        }

        block->statements.clear();

        compiler.end_bb();
    }

    function.set_pseudo_registers(compiler.manager.last_pseudo_reg());
    function.set_pseudo_float_registers(compiler.manager.last_float_pseudo_reg());
}
