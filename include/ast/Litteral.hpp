//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_LITTERAL_H
#define AST_LITTERAL_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/TerminalNode.hpp"

namespace eddic {

struct ASTLitteral : public TerminalNode {
    std::string value;
    std::string label;
};

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTLitteral, 
    (std::string, value)
)

#endif
