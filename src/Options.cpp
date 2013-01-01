//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>
#include <vector>
#include <atomic>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "assert.hpp"
#include "Options.hpp"

using namespace eddic;

namespace po = boost::program_options;

namespace {

std::pair<std::string, std::string> numeric_parser(const std::string& s){
    if (s.find("-32") == 0) {
        return make_pair("32", std::string("true"));
    } else if (s.find("-64") == 0) {
        return make_pair("64", std::string("true"));
    } else {
        return make_pair(std::string(), std::string());
    }
}

std::atomic<bool> description_flag;

po::options_description visible("Usage : eddic [options] source.eddi");
po::options_description all("Usage : eddic [options] source.eddi");

std::unordered_map<std::string, std::vector<std::string>> triggers;

void add_trigger(const std::string& option, const std::vector<std::string>& childs){
   triggers[option] = childs; 
}

void init_descriptions(){
    po::options_description general("General options");
    general.add_options()
        ("help,h", "Generate this help message")
        ("assembly,S", "Generate only the assembly")
        ("keep,k", "Keep the assembly file")
        ("version", "Print the version of eddic")
        ("output,o", po::value<std::string>()->default_value("a.out"), "Set the name of the executable")
       
        ("debug,g", "Add debugging symbols")
        
        ("template-depth", po::value<int>()->default_value(100), "Define the maximum template depth")
        
        ("32", "Force the compilation for 32 bits platform")
        ("64", "Force the compilation for 64 bits platform")

        ("warning-all", "Enable all the warning messages")
        ("warning-unused", "Warn about unused variables, parameters and functions")
        ("warning-cast", "Warn about useless casts")
        ("warning-effects", "Warn about statements without effect")
        ("warning-includes", "Warn about useless includes")
        ;
    
    po::options_description display("Display options");
    display.add_options()
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

    po::options_description optimization("Optimization options");
    optimization.add_options()
        ("Opt,O", po::value<int>()->implicit_value(0)->default_value(2), "Define the optimization level")
        ("O0", "Disable all optimizations")
        ("O1", "Enable low-level optimizations")
        ("O2", "Enable all optimizations. This can be slow for big programs.")
        
        ("fglobal-optimization", "Enable optimizer engine")
        ("fparameter-allocation", "Enable parameter allocation in register")
        ("fpeephole-optimization", "Enable peephole optimizer")
        ("fomit-frame-pointer", "Omit frame pointer from functions")
        ("finline-functions", "Enable inlining")
        ("fno-inline-functions", "Disable inlining")
        ;
    
    po::options_description backend("Backend options");
    backend.add_options()
        ("log", po::value<int>()->default_value(0), "Define the logging")
        ("quiet,q", "Do not print anything")
        ("verbose,v", "Make the compiler verbose")
        ("single-threaded", "Disable the multi-threaded optimization")
        ("time", "Activate the timing system")
        ("stats", "Activate the statistics system")
        ("input", po::value<std::string>(), "Input file")
        ;

    all.add(general).add(display).add(optimization).add(backend);
    visible.add(general).add(display).add(optimization);

    add_trigger("warning-all", {"warning-unused", "warning-cast", "warning-effects", "warning-includes"});
    
    //Special triggers for optimization levels
    add_trigger("__1", {"fpeephole-optimization"});
    add_trigger("__2", {"fglobal-optimization", "fomit-frame-pointer", "fparameter-allocation", "finline-functions"});
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

    try {
        //Only if the description has not been already defined
        if(!description_flag.load()){
            bool old_value = description_flag.load();
            if(description_flag.compare_exchange_strong(old_value, true)){
                init_descriptions();
            }
        }

        //Add the option of the input file
        po::positional_options_description p;
        p.add("input", -1);

        //Create a new set of options
        po::variables_map options;

        //Parse the command line options
        po::store(po::command_line_parser(argc, argv).options(all).extra_parser(numeric_parser).positional(p).run(), options);
        po::notify(options);

        //Transfer the options in the eddic configuration
        for(auto& option : all.options()){
            ConfigValue value;

            if(options.count(option->long_name())){
                value.defined = true;
                value.value = options[option->long_name()].value();
            } else {
                value.defined = false;
                value.value = std::string("false");
            }

            configuration->values[option->long_name()] = value;
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
            configuration->values["Opt"].value = 0;
        } else if(options.count("O1")){
            configuration->values["Opt"].value = 1;
        } else if(options.count("O2")){
            configuration->values["Opt"].value = 2;
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
    } catch (const po::ambiguous_option& e) {
        std::cout << "Invalid command line options : " << e.what() << std::endl;

        return nullptr;
    } catch (const po::unknown_option& e) {
        std::cout << "Invalid command line options : " << e.what() << std::endl;

        return nullptr;
    } catch (const po::multiple_occurrences& e) {
        std::cout << "Only one file can be compiled" << std::endl;

        return nullptr;
    }

    return configuration;
}

bool Configuration::option_defined(const std::string& option_name){
    return values[option_name].defined;
}

std::string Configuration::option_value(const std::string& option_name){
    return boost::any_cast<std::string>(values[option_name].value);
}

int Configuration::option_int_value(const std::string& option_name){
    return boost::any_cast<int>(values[option_name].value);
}

void eddic::print_help(){
    std::cout << visible << std::endl;
}

void eddic::print_version(){
    std::cout << "eddic version 1.2.1" << std::endl;
}
