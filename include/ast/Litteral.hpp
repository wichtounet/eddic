//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_LITTERAL_H
#define AST_LITTERAL_H

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

namespace ast {

/*!
 * \class Litteral
 * \brief The AST node for a string litteral.    
 */
struct Litteral {
    std::string value;
    std::string label;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Litteral, 
    (std::string, value)
)

#endif
