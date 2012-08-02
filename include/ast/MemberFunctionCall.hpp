//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_MEMBER_FUNCTION_CALL_H
#define AST_MEMBER_FUNCTION_CALL_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTMemberFunctionCall
 * \brief The AST node for a function call on a member. 
 * Should only be used from the Deferred version (eddic::ast::MemberFunctionCall).
 */
struct ASTMemberFunctionCall {
    std::shared_ptr<eddic::Function> function;
    std::shared_ptr<Context> context;
    std::string mangled_name;

    Position position;
    std::string object_name;
    std::string function_name;
    std::vector<Value> values;

    mutable long references = 0;
};

/*!
 * \typedef MemberFunctionCall
 * \brief The AST node for a member function call.
 */
typedef Deferred<ASTMemberFunctionCall> MemberFunctionCall;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::MemberFunctionCall, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->object_name)
    (std::string, Content->function_name)
    (std::vector<eddic::ast::Value>, Content->values)
)

#endif
