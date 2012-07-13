//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "assert.hpp"
#include "Options.hpp"

using namespace eddic;

namespace po = boost::program_options;

struct ConfigValue {
    bool defined;
    boost::any value;
};

struct Configuration {
    std::unordered_map<std::string, ConfigValue> values;
};

std::shared_ptr<Configuration> configuration;
std::unordered_map<std::string, std::vector<std::string>> triggers;

bool desc_init = false;
po::options_description visible("Usage : eddic [options] source.eddi");
po::options_description all("Usage : eddic [options] source.eddi");

std::pair<std::string, std::string> numeric_parser(const std::string& s){
    if (s.find("-32") == 0) {
        return make_pair("32", std::string("true"));
    } else if (s.find("-64") == 0) {
        return make_pair("64", std::string("true"));
    } else {
        return make_pair(std::string(), std::string());
    }
}

void add_trigger(const std::string& option, std::vector<std::string> childs){
   triggers[option] = childs; 
}

inline void trigger_childs(const std::vector<std::string>& childs){
    for(auto& child : childs){
        configuration->values[child].defined = true;
        configuration->values[child].value = std::string("true");
    }
}

bool eddic::parseOptions(int argc, const char* argv[]) {
    try {
        //Only if the description has not been already defined
        if(!desc_init){
            po::options_description general("General options");
            general.add_options()
                ("help,h", "Generate this help message")
                ("assembly,S", "Generate only the assembly")
                ("keep,k", "Keep the assembly file")
                ("version", "Print the version of eddic")
                ("output,o", po::value<std::string>()->default_value("a.out"), "Set the name of the executable")
               
                ("debug,g", "Add debugging symbols")
                
                ("32", "Force the compilation for 32 bits platform")
                ("64", "Force the compilation for 64 bits platform")

                ("warning-all", "Enable all the warning messages")
                ("warning-unused", "Warn about unused variables, parameters and functions")
                ("warning-cast", "Warn about useless casts");
            
            po::options_description display("Display options");
            display.add_options()
                ("ast", "Print the Abstract Syntax Tree representation of the source")
                ("ast-only", "Only print the Abstract Syntax Tree representation of the source (do not continue compilation after printing)")

                ("mtac", "Print the medium-level Three Address Code representation of the source")
                ("mtac-opt", "Print the medium-level Three Address Code representation of the source before any optimization has been performed")
                ("mtac-only", "Only print the medium-level Three Address Code representation of the source (do not continue compilation after printing)")

                ("ltac", "Print the low-level Three Address Code representation of the source")
                ("ltac-only", "Only print the low-level Three Address Code representation of the source (do not continue compilation after printing)");

            po::options_description optimization("Optimization options");
            optimization.add_options()
                ("Opt,O", po::value<int>()->implicit_value(0)->default_value(2), "Define the optimization level")
                ("O0", "Disable all optimizations")
                ("O1", "Enable low-level optimizations")
                ("O2", "Enable all optimizations. This can be slow for big programs.")
                
                ("fglobal-optimization", "Enable optimizer engine")
                ("fvariable-allocation", "Enable variable allocation in register")
                ("fpeephole-optimization", "Enable peephole optimizer")
                ("finline-functions", "Enable inlining")
                ("fno-inline-functions", "Disable inlining");
            
            po::options_description backend("Backend options");
            backend.add_options()
                ("quiet,q", "Do not print anything")
                ("verbose,v", "Make the compiler verbose")
                ("dev,d", "Activate development mode (very verbose)")
                ("perfs", "Display performance information")
                ("input", po::value<std::string>(), "Input file");

            all.add(general).add(display).add(optimization).add(backend);
            visible.add(general).add(display).add(optimization);

            add_trigger("warning-all", {"warning-unused", "warning-cast"});
            
            //TODO Should be a better way to do that
            add_trigger("__1", {"fpeephole-optimization"});
            add_trigger("__2", {"fglobal-optimization", "fvariable-allocation", "finline-functions"});
            
            desc_init = true;
        }

        //Add the option of the input file
        po::positional_options_description p;
        p.add("input", -1);

        //Create a new set of options
        po::variables_map options;

        //Create a new configuration
        configuration = std::make_shared<Configuration>();

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

            return false;
        }

        if(options.count("64") && options.count("32")){
            std::cout << "Invalid command line options : a compilation cannot be both 32 and 64 bits" << std::endl;

            return false;
        }

        //TODO Perhaps a more clear way to do that
        if(options.count("O0")){
            configuration->values["Opt"].value = 0;
        } else if(options.count("O1")){
            configuration->values["Opt"].value = 1;
        } else if(options.count("O2")){
            configuration->values["Opt"].value = 2;
        }

        //Triggers dependent options
        for(auto& trigger : triggers){
            if(option_defined(trigger.first)){
                trigger_childs(trigger.second);
            }
        }

        if(option_int_value("Opt") >= 1){
            trigger_childs(triggers["__1"]);
        } 
        
        if(option_int_value("Opt") >= 2){
            trigger_childs(triggers["__2"]);
        }
    } catch (const po::ambiguous_option& e) {
        std::cout << "Invalid command line options : " << e.what() << std::endl;

        return false;
    } catch (const po::unknown_option& e) {
        std::cout << "Invalid command line options : " << e.what() << std::endl;

        return false;
    } catch (const po::multiple_occurrences& e) {
        std::cout << "Only one file can be compiled" << std::endl;

        return false;
    }

    return true;
}

bool eddic::option_defined(const std::string& option_name){
    ASSERT(configuration, "The configuration have not been initialized");

    return configuration->values[option_name].defined;
}

std::string eddic::option_value(const std::string& option_name){
    ASSERT(configuration, "The configuration have not been initialized");

    return boost::any_cast<std::string>(configuration->values[option_name].value);
}

int eddic::option_int_value(const std::string& option_name){
    ASSERT(configuration, "The configuration have not been initialized");

    return boost::any_cast<int>(configuration->values[option_name].value);
}

void eddic::print_help(){
    std::cout << visible << std::endl;
}

void eddic::print_version(){
    std::cout << "eddic version 1.0.3" << std::endl;
}
