//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_CHAR_LITERAL_H
#define AST_CHAR_LITERAL_H

#include<string>

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

namespace ast {

/*!
 * \class CharLiteral
 * \brief The AST node for a char literal.    
 */
struct CharLiteral {
    std::string value;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::CharLiteral, 
    (std::string, value)
)

#endif
