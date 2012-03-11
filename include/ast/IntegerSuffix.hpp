//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_INTEGER_SUFFIX_H
#define AST_INTEGER_SUFFIX_H

#include <string>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/TerminalNode.hpp"

namespace eddic {

namespace ast {

/*!
 * \class IntegerSuffix
 * \brief The AST node for an integer suffixed to be another type.    
 */
struct IntegerSuffix : public TerminalNode {
    int value;
    std::string suffix;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::IntegerSuffix, 
    (int, value)
    (std::string, suffix)
)

#endif
