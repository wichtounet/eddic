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
#include "mtac/Optimizer.hpp"
#include "mtac/Printer.hpp"
#include "mtac/RegisterAllocation.hpp"
#include "mtac/LoopAnalysis.hpp"

//Low-level Three Address Code
#include "ltac/Compiler.hpp"
#include "ltac/PeepholeOptimizer.hpp"
#include "ltac/Printer.hpp"

//Code generation
#include "asm/CodeGeneratorFactory.hpp"

using namespace eddic;

void NativeBackEnd::generate(std::shared_ptr<mtac::Program> mtacProgram, Platform platform){
    std::string output = configuration->option_value("output");

    //Separate into basic blocks
    mtac::BasicBlockExtractor extractor;
    extractor.extract(mtacProgram);
    
    //If asked by the user, print the Three Address code representation before optimization
    if(configuration->option_defined("mtac-opt")){
        mtac::Printer printer;
        printer.print(mtacProgram);
    }

    mtac::loop_analysis(mtacProgram); 
    
    //Optimize MTAC
    mtac::Optimizer optimizer;
    optimizer.optimize(mtacProgram, get_string_pool(), platform, configuration);

    //Allocate parameters into registers
    if(configuration->option_defined("fparameter-allocation")){
        mtac::register_param_allocation(mtacProgram, platform);
    }

    //Allocate variables into registers
    if(configuration->option_defined("fvariable-allocation")){
        mtac::register_variable_allocation(mtacProgram, platform);
    }
    
    //If asked by the user, print the Three Address code representation
    if(configuration->option_defined("mtac") || configuration->option_defined("mtac-only")){
        mtac::Printer printer;
        printer.print(mtacProgram);
    }

    //If necessary, continue the compilation process
    if(!configuration->option_defined("mtac-only")){
        //Prepare the float pool
        auto float_pool = std::make_shared<FloatPool>();

        //Create a new LTAC program
        auto ltac_program = std::make_shared<ltac::Program>();

        //Generate LTAC Code
        ltac::Compiler ltacCompiler(platform, configuration);
        ltacCompiler.compile(mtacProgram, ltac_program, float_pool);

        if(configuration->option_defined("fpeephole-optimization")){
            optimize(ltac_program, platform, configuration);
        }

        //If asked by the user, print the Three Address code representation
        if(configuration->option_defined("ltac") || configuration->option_defined("ltac-only")){
            ltac::Printer printer;
            printer.print(ltac_program);
        }

        if(!configuration->option_defined("ltac-only")){
            //Generate assembly from TAC
            AssemblyFileWriter writer("output.asm");

            as::CodeGeneratorFactory factory;
            auto generator = factory.get(platform, writer, mtacProgram->context);

            //Generate the code from the LTAC Program
            generator->generate(ltac_program, get_string_pool(), float_pool); 

            //Write the output
            writer.write(); 

            //If it's necessary, assemble and link the assembly
            if(!configuration->option_defined("assembly")){
                assemble(platform, output, configuration->option_defined("debug"), configuration->option_defined("verbose"));

                //Remove temporary files
                if(!configuration->option_defined("keep")){
                    remove("output.asm");
                }

                remove("output.o");
            }
        }
    }
}
