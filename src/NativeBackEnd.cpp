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
#include "mtac/Program.hpp"
#include "mtac/BasicBlockExtractor.hpp"
#include "mtac/TemporaryAllocator.hpp"
#include "mtac/LivenessAnalyzer.hpp"
#include "mtac/Optimizer.hpp"
#include "mtac/Printer.hpp"

//Code generation
#include "asm/CodeGeneratorFactory.hpp"

using namespace eddic;

void NativeBackEnd::generate(std::shared_ptr<mtac::Program> tacProgram){
    std::string output = option_value("output");

    //Separate into basic blocks
    mtac::BasicBlockExtractor extractor;
    extractor.extract(*tacProgram);

    //Allocate storage for the temporaries that need to be stored
    mtac::TemporaryAllocator allocator;
    allocator.allocate(*tacProgram);

    mtac::Optimizer optimizer;
    optimizer.optimize(*tacProgram, *get_string_pool());

    //If asked by the user, print the Three Address code representation
    if(option_defined("tac") || option_defined("tac-only")){
        mtac::Printer printer;
        printer.print(*tacProgram);
    }

    //If necessary, continue the compilation process
    if(!option_defined("tac-only")){
        //Compute liveness of variables
        mtac::LivenessAnalyzer liveness;
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
