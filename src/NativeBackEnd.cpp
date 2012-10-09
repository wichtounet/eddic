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

//Low-level Three Address Code
#include "ltac/Compiler.hpp"
#include "ltac/PeepholeOptimizer.hpp"
#include "ltac/Printer.hpp"

//Code generation
#include "asm/CodeGeneratorFactory.hpp"

using namespace eddic;

void NativeBackEnd::generate(std::shared_ptr<mtac::Program> mtacProgram, Platform platform){
    std::string output = configuration->option_value("output");

    //Prepare the float pool
    auto float_pool = std::make_shared<FloatPool>();

    //Create a new LTAC program
    auto ltac_program = std::make_shared<ltac::Program>();

    //Generate LTAC Code
    ltac::Compiler ltacCompiler(platform, configuration);
    ltacCompiler.compile(mtacProgram, ltac_program, float_pool);

    if(configuration->option_defined("fpeephole-optimization")){
        optimize(ltac_program, platform);
    }

    //If asked by the user, print the Three Address code representation
    if(configuration->option_defined("ltac") || configuration->option_defined("ltac-only")){
        ltac::Printer printer;
        printer.print(ltac_program);
    }

    if(!configuration->option_defined("ltac-only")){
        auto input_file_name = configuration->option_value("input");
        auto asm_file_name = input_file_name + ".s";
        auto object_file_name = input_file_name + ".o";

        //Generate assembly from TAC
        AssemblyFileWriter writer(asm_file_name);

        as::CodeGeneratorFactory factory;
        auto generator = factory.get(platform, writer, mtacProgram->context);

        //Generate the code from the LTAC Program
        generator->generate(ltac_program, get_string_pool(), float_pool); 

        //Write the output
        writer.write(); 

        //If it's necessary, assemble and link the assembly
        if(!configuration->option_defined("assembly")){
            assemble(platform, asm_file_name, object_file_name, output, configuration->option_defined("debug"), configuration->option_defined("verbose"));

            //Remove temporary files
            if(!configuration->option_defined("keep")){
                remove(asm_file_name.c_str());
            }

            remove(object_file_name.c_str());
        }
    }
}
