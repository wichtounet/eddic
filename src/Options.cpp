//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>

#include "Options.hpp"

using namespace eddic;

bool eddic::OptimizeIntegers;
bool eddic::OptimizeStrings;

po::variables_map eddic::options;

po::options_description desc("Usage : edic [options]");

void eddic::parseOptions(int argc, const char* argv[]) {
    desc.add_options()
        ("help,h", "Generate this help message")
        ("assembly,S", "Generate only the assembly")
        ("version", "Print the version of eddic")
        ("output,o", po::value<std::string>()->default_value("a.out"), "Set the name of the executable")
        ("optimize-all", "Enable all optimizations")
        ("optimize-strings", po::bool_switch(&OptimizeStrings), "Enable the optimizations on strings")
        ("optimize-integers", po::bool_switch(&OptimizeIntegers), "Enable the optimizations on integers")
        ("input", po::value<std::string>(), "Input file");

    po::positional_options_description p;
    p.add("input", -1);

    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), options);
    po::notify(options);

    if(options.count("optimize-all")){
        OptimizeStrings = OptimizeIntegers = true;
    }
}

void eddic::printHelp(){
    std::cout << desc << std::endl;
}

void eddic::printVersion(){
    std::cout << "eddic version 0.5.0" << std::endl;
}
