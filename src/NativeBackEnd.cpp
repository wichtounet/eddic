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

//Three Address Code
#include "tac/Program.hpp"
#include "tac/BasicBlockExtractor.hpp"
#include "tac/TemporaryAllocator.hpp"
#include "tac/LivenessAnalyzer.hpp"
#include "tac/Optimizer.hpp"
#include "tac/Printer.hpp"

//Code generation
#include "asm/CodeGeneratorFactory.hpp"

using namespace eddic;

void NativeBackEnd::generate(std::shared_ptr<tac::Program> tacProgram){
    std::string output = option_value("output");

    //Separate into basic blocks
    tac::BasicBlockExtractor extractor;
    extractor.extract(*tacProgram);

    //Allocate storage for the temporaries that need to be stored
    tac::TemporaryAllocator allocator;
    allocator.allocate(*tacProgram);

    tac::Optimizer optimizer;
    optimizer.optimize(*tacProgram, *get_string_pool());

    //If asked by the user, print the Three Address code representation
    if(option_defined("tac") || option_defined("tac-only")){
        tac::Printer printer;
        printer.print(*tacProgram);
    }

    //If necessary, continue the compilation process
    if(!option_defined("tac-only")){
        //Compute liveness of variables
        tac::LivenessAnalyzer liveness;
        liveness.compute(*tacProgram);

        //Generate assembly from TAC
        AssemblyFileWriter writer("output.asm");

        as::CodeGeneratorFactory factory;
        auto generator = factory.get(platform, writer);
        generator->generate(*tacProgram, *get_string_pool()); 
        writer.write(); 

        //If it's necessary, assemble and link the assembly
        if(!option_defined("assembly")){
            assemble(platform, output, option_defined("debug"), option_defined("verbose"));

            //Remove temporary files
            if(!option_defined("keep")){
                remove("output.asm");
            }

            remove("output.o");
        }
    }
}
