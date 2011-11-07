//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SPIRIT_PARSER_H
#define SPIRIT_PARSER_H

#include <string>

namespace eddic {

class ASTProgram;

class SpiritParser {
    public:
        bool parse(const std::string& file, ASTProgram& program);
};

} //end of eddic

#endif
