//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <cstdio>

#include "StopWatch.hpp"
#include "Compiler.hpp"
#include "Target.hpp"
#include "Utils.hpp"
#include "Options.hpp"
#include "SemanticalException.hpp"
#include "TerminationException.hpp"
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
#include "mtac/RegisterAllocation.hpp"
#include "mtac/reference_resolver.hpp"
#include "mtac/WarningsEngine.hpp"

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
    int code = 0; 

    std::unique_ptr<mtac::Program> program;

    try {
        //Make sure that the file exists 
        if(!file_exists(file)){
            throw SemanticalException("The file \"" + file + "\" does not exists");
        }

        auto front_end = get_front_end(file);

        if(!front_end){
            throw SemanticalException("The file \"" + file + "\" cannot be compiled using eddic");
        }

        program = compile_mtac(file, platform, configuration, *front_end);

        //If program is null, it means that the user didn't wanted it
        if(program){
            mtac::collect_warnings(*program, configuration);

            if(!configuration->option_defined("mtac-only")){
                compile_ltac(*program, platform, configuration, *front_end);
            }
        }
    } catch (const SemanticalException& e) {
        if(!configuration->option_defined("quiet")){
            output_exception(e, program->context);
        }

        code = 1;
    } catch (const TerminationException&) {
        code = 1;
    }

    //Display stats if necessary
    if(program && configuration->option_defined("stats")){
        std::cout << "Statistics" << std::endl;

        for(auto& counter : program->context->stats()){
            std::cout << "\t" << counter.first << ":" << counter.second << std::endl;
        }
    }
    
    //Display timings if necessary
    if(program && configuration->option_defined("time")){
        program->context->timing().display();
    }

    return code;
}

std::unique_ptr<mtac::Program> Compiler::compile_mtac(const std::string& file, Platform platform, std::shared_ptr<Configuration> configuration, FrontEnd& front_end){
    front_end.set_configuration(configuration);

    auto program = front_end.compile(file, platform);

    //If program is null, it means that the user didn't wanted it
    if(program){
        mtac::resolve_references(*program);

        //Separate into basic blocks
        mtac::BasicBlockExtractor extractor;
        extractor.extract(*program);

        //If asked by the user, print the Three Address code representation before optimization
        if(configuration->option_defined("mtac-opt")){
            std::cout << *program << std::endl;
        }
            
        //Build the call graph (will be used for each optimization level)
        mtac::build_call_graph(*program);

        //Optimize MTAC
        mtac::Optimizer optimizer;
        optimizer.optimize(*program, front_end.get_string_pool(), platform, configuration);

        //Allocate parameters into registers
        if(configuration->option_defined("fparameter-allocation")){
            mtac::register_param_allocation(*program, platform);
        }

        //If asked by the user, print the Three Address code representation
        if(configuration->option_defined("mtac") || configuration->option_defined("mtac-only")){
            std::cout << *program << std::endl;
        }
    }

    return std::move(program);
}
    
void Compiler::compile_ltac(mtac::Program& program, Platform platform, std::shared_ptr<Configuration> configuration, FrontEnd& front_end){
    //Compute the definitive reachable flag for functions
    program.call_graph.compute_reachable();

    auto back_end = get_back_end(Output::NATIVE_EXECUTABLE);

    back_end->set_string_pool(front_end.get_string_pool());
    back_end->set_configuration(configuration);

    back_end->generate(program, platform);
}
