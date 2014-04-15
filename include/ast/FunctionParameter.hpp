//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_FUNCTION_PARAMETER_H
#define AST_FUNCTION_PARAMETER_H

#include <string>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/VariableType.hpp"

namespace eddic {

namespace ast {

/*!
 * \class FunctionParameter
 * \brief The AST node for a function parameter in a function declaration.
 */
struct FunctionParameter {
    Type parameterType;
    std::string parameterName;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::FunctionParameter, 
    (eddic::ast::Type, parameterType)
    (std::string, parameterName)
)

#endif
