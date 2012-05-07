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

#include "FrontEnds.hpp"
#include "BackEnds.hpp"

//Three Address Code
#include "mtac/Program.hpp"

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
        auto mtacProgram = front_end->compile(file);

        //If program is null, it means that the user didn't wanted it
        if(mtacProgram){
            auto back_end = get_back_end(Output::NATIVE_EXECUTABLE);

            back_end->set_string_pool(front_end->get_string_pool());

            back_end->generate(mtacProgram);
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
