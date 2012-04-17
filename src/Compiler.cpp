//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cstdio>

#include "DebugStopWatch.hpp"
#include "Compiler.hpp"
#include "Target.hpp"
#include "Utils.hpp"
#include "Options.hpp"
#include "SymbolTable.hpp"
#include "SemanticalException.hpp"
#include "AssemblyFileWriter.hpp"
#include "Assembler.hpp"

#include "FrontEnds.hpp"

//Three Address Code
#include "tac/Program.hpp"
#include "tac/BasicBlockExtractor.hpp"
#include "tac/TemporaryAllocator.hpp"
#include "tac/LivenessAnalyzer.hpp"
#include "tac/Optimizer.hpp"
#include "tac/Printer.hpp"

//Code generation
#include "asm/CodeGeneratorFactory.hpp"

//32 bits by default
eddic::Platform eddic::platform = Platform::INTEL_X86;

using namespace eddic;

int Compiler::compile(const std::string& file) {
    if(!option_defined("quiet")){
        std::cout << "Compile " << file << std::endl;
    }

    if(TargetDetermined && Target64){
        platform = Platform::INTEL_X86_64;
    }

    if(option_defined("32")){
        platform = Platform::INTEL_X86;
    }
    
    if(option_defined("64")){
        platform = Platform::INTEL_X86_64;
    }

    StopWatch timer;
    
    int code = compileOnly(file, platform);

    if(!option_defined("quiet")){
        std::cout << "Compilation took " << timer.elapsed() << "ms" << std::endl;
    }

    return code;
}

int Compiler::compileOnly(const std::string& file, Platform platform) {
    //Reset the symbol table
    symbols.reset();

    //Make sure that the file exists 
    if(!file_exists(file)){
        std::cout << "The file \"" + file + "\" does not exists" << std::endl;

        return false;
    }

    auto front_end = get_front_end(file);

    if(!front_end){
        std::cout << "The file \"" + file + "\" cannot be compiled using eddic" << std::endl;

        return false;
    }

    int code = 0; 

    try {
        auto tacProgram = front_end->compile(file);

        //If program is null, it means that the user didn't wanted it
        if(tacProgram){
            std::string output = option_value("output");

            //Separate into basic blocks
            tac::BasicBlockExtractor extractor;
            extractor.extract(*tacProgram);

            //Allocate storage for the temporaries that need to be stored
            tac::TemporaryAllocator allocator;
            allocator.allocate(*tacProgram);

            tac::Optimizer optimizer;
            optimizer.optimize(*tacProgram, *front_end->get_string_pool());

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
                generator->generate(*tacProgram, *front_end->get_string_pool()); 
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
    } catch (const SemanticalException& e) {
        if(!option_defined("quiet")){
            if(e.position()){
                auto& position = *e.position();

                std::cout << position.file << ":" << position.line << ":" << " error: " << e.what() << std::endl;
            } else {
                std::cout << e.what() << std::endl;
            }
        }

        code = 1;
    }

    return code;
}
