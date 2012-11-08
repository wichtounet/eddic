//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"
#include "Options.hpp"
#include "Compiler.hpp"

#include <iostream>

using namespace eddic;

void printUsage();

int main(int argc, const char* argv[]) {
    auto configuration = parseOptions(argc, argv);
    if(!configuration){
        return -1;
    }
    
    configure_logging(configuration->option_int_value("log"));

    if(configuration->option_defined("help")){
        print_help();
        return 0;
    }

    if(configuration->option_defined("version")){
        print_version();
        return 0;
    }

    if (!configuration->option_defined("input")) {
        std::cout << "eddic: no input files" << std::endl;
        print_help();
        return -1;
    }

    Compiler compiler;
    return compiler.compile(configuration->option_value("input"), configuration);
}
