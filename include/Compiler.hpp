//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_H
#define COMPILER_H

#include <string>

#include "Lexer.hpp"
#include "AssemblyFileWriter.hpp"

namespace eddic {

class Compiler {
    private:
        Lexer lexer;
        AssemblyFileWriter writer;

    public:
        int compile (std::string file);
};

} //end of eddic

#endif
