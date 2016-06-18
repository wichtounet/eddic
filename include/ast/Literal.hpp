//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_LITERAL_H
#define AST_LITERAL_H

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

namespace ast {

/*!
 * \class Literal
 * \brief The AST node for a string literal.
 */
struct Literal {
    std::string value;
    std::string label;

    Literal(){}
    Literal(std::string value) : value(value) {}
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Literal,
    (std::string, value)
)

#endif
