//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <string>
#include <iostream>
#include <vector>
#include <atomic>

#define CXXOPTS_NO_RTTI
#include "cxxopts.hpp"

#include "Options.hpp"
#include "Utils.hpp"

using namespace eddic;

namespace {

void init_options(cxxopts::Options& options){
    options.add_options("General")
        ("h,help", "Generate this help message")
        ("S,assembly", "Generate only the assembly")
        ("k,keep", "Keep the assembly file")
        ("version", "Print the version of eddic")
        ("o,output", "Set the name of the executable", cxxopts::value<std::string>()->default_value("a.out"))
        ("g,debug", "Add debugging symbols")
        ("template-depth", "Define the maximum template depth", cxxopts::value<std::string>()->default_value("100"))
        ("32", "Force the compilation for 32 bits platform")
        ("64", "Force the compilation for 64 bits platform")
        ("warning-all", "Enable all the warning messages")
        ("warning-unused", "Warn about unused variables, parameters and functions")
        ("warning-cast", "Warn about useless casts")
        ("warning-effects", "Warn about statements without effect")
        ("warning-includes", "Warn about useless includes")
        ;

    options.add_options("Display")
        ("ast", "Print the Abstract Syntax Tree representation of the source")
        ("ast-raw", "Print the Abstract Syntax Tree representation of the source coming directly from the parser before any pass is run on the AST. ")
        ("ast-only", "Only print the Abstract Syntax Tree representation of the source (do not continue compilation after printing)")
        ("mtac", "Print the medium-level Three Address Code representation of the source")
        ("mtac-opt", "Print the medium-level Three Address Code representation of the source before any optimization has been performed")
        ("mtac-only", "Only print the medium-level Three Address Code representation of the source (do not continue compilation after printing)")
        ("ltac-pre", "Print the low-level Three Address Code representation of the source before allocation of registers")
        ("ltac-alloc", "Print the low-level Three Address Code representation of the source before optimization")
        ("ltac", "Print the final low-level Three Address Code representation of the source")
        ("ltac-only", "Only print the low-level Three Address Code representation of the source (do not continue compilation after printing)")
        ;

    options.add_options("Optimization")
        ("O,Opt", "Define the optimization level", cxxopts::value<std::string>()->implicit_value("0")->default_value("2"))
        ("O0", "Disable all optimizations")
        ("O1", "Enable low-level optimizations")
        ("O2", "Enable all optimizations improving the speed but do imply a space tradeoff.")
        ("O3", "Enable all optimizations improving the speed but can increase the size of the program.")
        ("fglobal-optimization", "Enable optimizer engine")
        ("fparameter-allocation", "Enable parameter allocation in register")
        ("fpeephole-optimization", "Enable peephole optimizer")
        ("fomit-frame-pointer", "Omit frame pointer from functions")
        ("finline-functions", "Enable inlining")
        ("fno-inline-functions", "Disable inlining")
        ("funroll-loops", "Enable Loop Unrolling")
        ("fcomplete-peel-loops", "Enable Complete Loop Peeling")
        ;

    options.add_options("Backend")
        ("log", "Define the logging", cxxopts::value<std::string>()->default_value("0"))
        ("q,quiet", "Do not print anything")
        ("v,verbose", "Make the compiler verbose")
        ("single-threaded", "Disable the multi-threaded optimization")
        ("time", "Activate the timing system")
        ("stats", "Activate the statistics system")
        ("input", "Input file", cxxopts::value<std::string>())
        ;
}

using triggers_t = std::unordered_map<std::string, std::vector<std::string>>;

void add_trigger(triggers_t& triggers, const std::string& option, const std::vector<std::string>& childs){
   triggers[option] = childs; 
}

inline void trigger_childs(std::shared_ptr<Configuration> configuration, const std::vector<std::string>& childs){
    for(auto& child : childs){
        configuration->values[child].defined = true;
        configuration->values[child].value = std::string("true");
    }
}

} //end of anonymous namespace

std::shared_ptr<Configuration> eddic::parseOptions(int argc, const char* argv[]) {
    //Create a new configuration
    auto configuration = std::make_shared<Configuration>();

    triggers_t triggers;

    try {
        add_trigger(triggers, "warning-all", {"warning-unused", "warning-cast", "warning-effects", "warning-includes"});

        //Special triggers for optimization levels
        add_trigger(triggers, "__1", {"fpeephole-optimization"});
        add_trigger(triggers, "__2", {"fglobal-optimization", "fomit-frame-pointer", "fparameter-allocation", "finline-functions"});
        add_trigger(triggers, "__3", {"funroll-loops", "fcomplete-peel-loops", "funswitch-loops"});

        cxxopts::Options options("eddic", "  source.eddi");

        init_options(options);

        options.parse_positional("input");

        auto& v = const_cast<char**&>(argv);
        options.parse(argc, v);

        for(auto& group : options.groups()){
            for(auto& option : options.group_help(group).options){
                auto name = option.l.empty() ? option.s : option.l;

                ConfigValue value;

                if(options.count(name)){
                    value.defined = true;

                    if(option.has_arg){
                        value.value = options[name].as<std::string>();
                    } else {
                        value.value = "true";
                    }
                } else {
                    value.defined = false;
                    value.value = "false";
                }

                configuration->values[name] = value;
            }
        }

        if(options.count("O0") + options.count("O1") + options.count("O2") > 1){
            std::cout << "Invalid command line options : only one optimization level should be set" << std::endl;

            return nullptr;
        }

        if(options.count("64") && options.count("32")){
            std::cout << "Invalid command line options : a compilation cannot be both 32 and 64 bits" << std::endl;

            return nullptr;
        }

        //Update optimization level based on special switches
        if(options.count("O0")){
            configuration->values["Opt"].value = "0";
        } else if(options.count("O1")){
            configuration->values["Opt"].value = "1";
        } else if(options.count("O2")){
            configuration->values["Opt"].value = "2";
        } else if(options.count("O3")){
            configuration->values["Opt"].value = "3";
        }

        //Triggers dependent options
        for(auto& trigger : triggers){
            if(configuration->option_defined(trigger.first)){
                trigger_childs(configuration, trigger.second);
            }
        }

        if(configuration->option_int_value("Opt") >= 1){
            trigger_childs(configuration, triggers["__1"]);
        } 

        if(configuration->option_int_value("Opt") >= 2){
            trigger_childs(configuration, triggers["__2"]);
        }

        if(configuration->option_int_value("Opt") >= 3){
            trigger_childs(configuration, triggers["__3"]);
        }
    } catch (const cxxopts::OptionException& e) {
        std::cout << "Invalid command line options: " << e.what() << std::endl;

        return nullptr;
    }

    return configuration;
}

bool Configuration::option_defined(const std::string& option_name){
    return values[option_name].defined;
}

std::string Configuration::option_value(const std::string& option_name){
    return values[option_name].value;
}

int Configuration::option_int_value(const std::string& option_name){
    return toNumber<int>(values[option_name].value);
}

void eddic::print_help(){
    cxxopts::Options options("eddic", "  source.eddi");

    init_options(options);

    std::cout << options.help({"General", "Display", "Optimization"}) << std::endl;
}

void eddic::print_version(){
    std::cout << "eddic version 1.2.3" << std::endl;
}
