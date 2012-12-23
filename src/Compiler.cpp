//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cstdio>

#include "StopWatch.hpp"
#include "Compiler.hpp"
#include "Target.hpp"
#include "Utils.hpp"
#include "Options.hpp"
#include "SemanticalException.hpp"
#include "GlobalContext.hpp"

#include "FrontEnd.hpp"
#include "FrontEnds.hpp"
#include "BackEnds.hpp"
#include "BackEnd.hpp"
#include "Platform.hpp"

//Medium-level Three Address Code
#include "mtac/Program.hpp"
#include "mtac/BasicBlockExtractor.hpp"
#include "mtac/Optimizer.hpp"
#include "mtac/Printer.hpp"
#include "mtac/RegisterAllocation.hpp"
#include "mtac/reference_resolver.hpp"

using namespace eddic;

int Compiler::compile(const std::string& file, std::shared_ptr<Configuration> configuration) {
    if(!configuration->option_defined("quiet")){
        std::cout << "Compile " << file << std::endl;
    }

    //32 bits by default
    Platform platform = Platform::INTEL_X86;

    if(TargetDetermined && Target64){
        platform = Platform::INTEL_X86_64;
    }

    if(configuration->option_defined("32")){
        platform = Platform::INTEL_X86;
    }
    
    if(configuration->option_defined("64")){
        platform = Platform::INTEL_X86_64;
    }

    StopWatch timer;
    
    int code = compile_only(file, platform, configuration);

    if(!configuration->option_defined("quiet")){
        std::cout << "Compilation took " << timer.elapsed() << "ms" << std::endl;
    }

    return code;
}

int Compiler::compile_only(const std::string& file, Platform platform, std::shared_ptr<Configuration> configuration) {
    //Make sure that the file exists 
    if(!file_exists(file)){
        std::cout << "The file \"" + file + "\" does not exists" << std::endl;

        return false;
    }

    auto front_end = get_front_end(file);
    front_end->set_configuration(configuration);

    if(!front_end){
        std::cout << "The file \"" + file + "\" cannot be compiled using eddic" << std::endl;

        return false;
    }

    int code = 0; 

    std::unique_ptr<mtac::Program> program;

    try {
        program = front_end->compile(file, platform);

        //If program is null, it means that the user didn't wanted it
        if(program){
            mtac::resolve_references(*program);

            //Separate into basic blocks
            mtac::BasicBlockExtractor extractor;
            extractor.extract(*program);

            //If asked by the user, print the Three Address code representation before optimization
            if(configuration->option_defined("mtac-opt")){
                mtac::Printer printer;
                printer.print(*program);
            }

            //Optimize MTAC
            mtac::Optimizer optimizer;
            optimizer.optimize(*program, front_end->get_string_pool(), platform, configuration);

            //Allocate parameters into registers
            if(configuration->option_defined("fparameter-allocation")){
                mtac::register_param_allocation(*program, platform);
            }

            //If asked by the user, print the Three Address code representation
            if(configuration->option_defined("mtac") || configuration->option_defined("mtac-only")){
                mtac::Printer printer;
                printer.print(*program);
            }

            if(!configuration->option_defined("mtac-only")){
                auto back_end = get_back_end(Output::NATIVE_EXECUTABLE);

                back_end->set_string_pool(front_end->get_string_pool());
                back_end->set_configuration(configuration);

                back_end->generate(*program, platform);
            }
        }
    } catch (const SemanticalException& e) {
        if(!configuration->option_defined("quiet")){
            if(e.position()){
                auto& position = *e.position();

                std::cout << position.file << ":" << position.line << ":" << " error: " << e.what() << std::endl;
            } else {
                std::cout << e.what() << std::endl;
            }
        }

        code = 1;
    }

    //Display stats if necessary
    if(configuration->option_defined("stats")){
        std::cout << "Statistics" << std::endl;

        for(auto& counter : program->context->stats()){
            std::cout << "\t" << counter.first << ":" << counter.second << std::endl;
        }
    }

    return code;
}
