//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_FUNCTION_CALL_H
#define AST_FUNCTION_CALL_H

#include <memory>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/Value.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTFunctionCall
 * \brief The AST node for a function call. 
 * Should only be used from the Deferred version (eddic::ast::FunctionCall).
 */
struct ASTFunctionCall {
    std::shared_ptr<Context> context;
    std::string mangled_name;

    Position position;
    std::string function_name;
    std::vector<ast::Type> template_types;
    std::vector<Value> values;

    mutable long references = 0;
};

/*!
 * \typedef FunctionCall
 * \brief The AST node for a function call.
 */
typedef Deferred<ASTFunctionCall> FunctionCall;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::FunctionCall, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->function_name)
    (std::vector<eddic::ast::Type>, Content->template_types)
    (std::vector<eddic::ast::Value>, Content->values)
)

#endif
