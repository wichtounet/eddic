//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_INTEGER_SUFFIX_H
#define AST_INTEGER_SUFFIX_H

#include <string>

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

namespace ast {

/*!
 * \class IntegerSuffix
 * \brief The AST node for an integer suffixed to be another type.    
 */
struct IntegerSuffix {
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
