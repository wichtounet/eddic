//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "NativeBackEnd.hpp"
#include "Utils.hpp"
#include "Options.hpp"
#include "AssemblyFileWriter.hpp"
#include "Assembler.hpp"
#include "FloatPool.hpp"

//Medium-level Three Address Code
#include "mtac/Program.hpp"
#include "mtac/BasicBlockExtractor.hpp"
#include "mtac/TemporaryAllocator.hpp"
#include "mtac/Optimizer.hpp"
#include "mtac/Printer.hpp"
#include "mtac/RegisterAllocation.hpp"

//Low-level Three Address Code
#include "ltac/Compiler.hpp"
#include "ltac/PeepholeOptimizer.hpp"
#include "ltac/Printer.hpp"

//Code generation
#include "asm/CodeGeneratorFactory.hpp"

using namespace eddic;

void NativeBackEnd::generate(std::shared_ptr<mtac::Program> mtacProgram){
    std::string output = option_value("output");

    //Separate into basic blocks
    mtac::BasicBlockExtractor extractor;
    extractor.extract(mtacProgram);
    
    //If asked by the user, print the Three Address code representation before optimization
    if(option_defined("mtac-opt")){
        mtac::Printer printer;
        printer.print(mtacProgram);
    }

    //Allocate storage for the temporaries that need to be stored
    mtac::TemporaryAllocator allocator;
    allocator.allocate(mtacProgram);
    
    mtac::Optimizer optimizer;

    if(OLevel >= 2){
        optimizer.optimize(mtacProgram, get_string_pool());
    
        //Allocate storage for the temporaries that need to be stored
        allocator.allocate(mtacProgram);
    } else {
        optimizer.basic_optimize(mtacProgram, get_string_pool());
    }

    //Allocate parameters into registers
    mtac::register_param_allocation();

    //Allocate variables into registers
    if(OLevel >= 2){
        mtac::register_variable_allocation(mtacProgram);
    }
    
    //If asked by the user, print the Three Address code representation
    if(option_defined("mtac") || option_defined("mtac-only")){
        mtac::Printer printer;
        printer.print(mtacProgram);
    }

    //If necessary, continue the compilation process
    if(!option_defined("mtac-only")){
        auto float_pool = std::make_shared<FloatPool>();

        auto ltac_program = std::make_shared<ltac::Program>();
        ltac::Compiler ltacCompiler;
        ltacCompiler.compile(mtacProgram, ltac_program, float_pool);

        if(OLevel >= 1){
            optimize(ltac_program);
        }

        //If asked by the user, print the Three Address code representation
        if(option_defined("ltac") || option_defined("ltac-only")){
            ltac::Printer printer;
            printer.print(ltac_program);
        }

        if(!option_defined("ltac-only")){
            //Generate assembly from TAC
            AssemblyFileWriter writer("output.asm");

            as::CodeGeneratorFactory factory;
            auto generator = factory.get(platform, writer);

            //Generate the code from the LTAC Program
            generator->generate(ltac_program, get_string_pool(), float_pool); 

            //Write the output
            writer.write(); 

            //If it's necessary, assemble and link the assembly
            if(!option_defined("assembly")){
                assemble(platform, output, option_defined("debug"), option_defined("verbose"));

                //Remove temporary files
                if(!option_defined("keep")){
//                    remove("output.asm");
                }

                remove("output.o");
            }
        }
    }
}
