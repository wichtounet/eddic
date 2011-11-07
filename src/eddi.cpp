//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Options.hpp"
#include "Compiler.hpp"

#include <iostream>

using std::string;
using std::cout;
using std::endl;

using namespace eddic;

void printUsage();

int main(int argc, const char* argv[]) {
    parseOptions(argc, argv);

    if(options.count("help")){
        printHelp();
        return 0;
    }

    if(options.count("version")){
        printVersion();
        return 0;
    }

    if (!options.count("input")) {
        cout << "eddic: no input files" << endl;
        printHelp();
        return -1;
    }

    Compiler compiler;
    return compiler.compile(options["input"].as<std::string>());
}
