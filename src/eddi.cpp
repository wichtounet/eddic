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
    if (argc == 1) {
        cout << "eddic: no input files" << endl;

        return -1;
    }

    Options::setDefaults();

    string sourceFile;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg[0] != '-') {
            sourceFile = arg;
            break;
        }

        if (arg == "--optimize-all") {
            Options::set(BooleanOption::OPTIMIZE_INTEGERS);
            Options::set(BooleanOption::OPTIMIZE_STRINGS);
        } else if (arg == "--optimize-integers") {
            Options::set(BooleanOption::OPTIMIZE_INTEGERS);
        } else if (arg == "--optimize-strings") {
            Options::set(BooleanOption::OPTIMIZE_STRINGS);
        } else if (arg == "-S") {
            Options::set(BooleanOption::ASSEMBLY_ONLY);
        } else if (arg == "--help" || arg == "-h") {
            printUsage();

            return 0;
        } else if (arg == "-o") {
            ++i;

            if (i >= argc) {
                cout << "eddic: -o is waiting for a value" << endl;

                printUsage();

                return -1;
            } else {
                Options::set(ValueOption::OUTPUT, argv[i]);
            }
        } else {
            cout << "eddic: unrecognized option \"" << arg << "\"" << endl;

            printUsage();

            return -1;
        }
    }

    if (sourceFile.size() == 0) {
        cout << "eddic: no input files" << endl;

        printUsage();

        return -1;
    }

    Compiler compiler;
    return compiler.compile(sourceFile);
}

void printUsage() {
    cout << "Usage: eddic [options] file" << endl;

    cout << "Options:" << endl;
    cout << "  -h, --help                 Display this help" << endl;
    cout << "  -o file                    Set the file name of the executable" << endl;
    cout << "  -S                         Generate only the assembly file" << endl;
    cout << "  --optimize-all             Enable all optimizations" << endl;
    cout << "  --optimize-strings         Enable the optimizations on strings" << endl;
    cout << "  --optimize-integers        Enable the optimizations on integers" << endl;
}
