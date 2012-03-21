//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>

#include "Options.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>

using namespace eddic;

bool eddic::WarningUnused;
bool eddic::WarningCast;

po::variables_map eddic::options;

po::options_description desc("Usage : eddic [options] source.eddi");

std::pair<std::string, std::string> numeric_parser(const std::string& s){
    if (s.find("-32") == 0) {
        return make_pair("32", std::string("true"));
    } else if (s.find("-64") == 0) {
        return make_pair("64", std::string("true"));
    } else {
        return make_pair(std::string(), std::string());
    }
}

bool eddic::parseOptions(int argc, const char* argv[]) {
    try {
        desc.add_options()
            ("help,h", "Generate this help message")
            ("assembly,S", "Generate only the assembly")
            ("keep,k", "Keep the assembly file")
            ("version", "Print the version of eddic")
            ("output,o", po::value<std::string>()->default_value("a.out"), "Set the name of the executable")

            ("ast", "Print the Abstract Syntax Tree representation of the source")
            ("tac", "Print the Three Address Code representation of the source")
            
            ("ast-only", "Only print the Abstract Syntax Tree representation of the source (do not continue compilation after printing)")
            ("tac-only", "Only print the Three Address Code representation of the source (do not continue compilation after printing)")
            
            ("debug,g", "Add debugging symbols")

            ("warning-all", "Enable all the warnings")
            ("warning-unused", po::bool_switch(&WarningUnused), "Enable warnings for unused variables, parameters and functions")
            ("warning-cast", po::bool_switch(&WarningCast), "Enable warnings for casts")
            
            ("32", "Force the compilation for 32 bits platform")
            ("64", "Force the compilation for 64 bits platform")
           
            ("input", po::value<std::string>(), "Input file");

        po::positional_options_description p;
        p.add("input", -1);

        po::store(po::command_line_parser(argc, argv).options(desc).extra_parser(numeric_parser).positional(p).run(), options);
        po::notify(options);

        if(options.count("warning-all")){
            WarningUnused = true;
            WarningCast = true;
        }

        if(options.count("64") && options.count("32")){
            std::cout << "Invalid command line options : a compilation cannot be both 32 and 64 bits" << std::endl;

            return false;
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

void eddic::printHelp(){
    std::cout << desc << std::endl;
}

void eddic::printVersion(){
    std::cout << "eddic version 0.9.0" << std::endl;
}
