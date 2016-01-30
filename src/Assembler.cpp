//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "Assembler.hpp"
#include "PerfsTimer.hpp"
#include "Utils.hpp"
#include "SemanticalException.hpp"

using namespace eddic;

namespace {

void exec(const std::string& command, bool verbose) {
    PerfsTimer timer("Exec " + command);

    if(verbose){
        std::cout << "eddic : exec command : " << command << std::endl;
    }

    std::string result = execCommand(command);

    if(result.size() > 0){
        std::cout << result << std::endl;
    }
}

void assembleWithoutDebug(Platform platform, const std::string& s, const std::string& o, const std::string& output, bool verbose){
    switch(platform){
        case Platform::INTEL_X86:
            exec("nasm -f elf32 -o " + o + " " + s, verbose);
            exec("ld -S -m elf_i386 " + o  + " -o " + output, verbose);

            break;
        case Platform::INTEL_X86_64:
            exec("nasm -f elf64 -o " + o + " " + s, verbose);
            exec("ld -S -m elf_x86_64 " + o  + " -o " + output, verbose);

            break;
    }
}

void assembleWithDebug(Platform platform, const std::string& s, const std::string& o, const std::string& output, bool verbose){
    switch(platform){
        case Platform::INTEL_X86:
            exec("nasm -g -f elf32 -o " + o + " " + s, verbose);
            exec("ld -m elf_i386 " + o  + " -o " + output, verbose);

            break;
        case Platform::INTEL_X86_64:
            exec("nasm -g -f elf64 -o " + o + " " + s, verbose);
            exec("ld -m elf_x86_64 " + o  + " -o " + output, verbose);

        break;
   }
}

} //end of anonymous namespace

void eddic::assemble(Platform platform, const std::string& s, const std::string& o, const std::string& output, bool debug, bool verbose){
   if(debug){
       assembleWithDebug(platform, s, o, output, verbose);
   } else {
       assembleWithoutDebug(platform, s, o, output, verbose);
   }
}

void eddic::verify_dependencies(){
    if(system("nasm -v >> /dev/null") != 0){
        throw SemanticalException("Error: Unable to use nasm");
    }

    if(system("ld -v >> /dev/null") != 0){
        throw SemanticalException("Error: Unable to use ld");
    }
}
