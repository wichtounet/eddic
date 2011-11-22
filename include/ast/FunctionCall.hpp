//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_FUNCTION_CALL_H
#define AST_FUNCTION_CALL_H

#include <boost/intrusive_ptr.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {

struct ASTFunctionCall {
    std::string functionName;
    std::vector<Value> values;

    mutable long references;
    ASTFunctionCall() : references(0) {}
};

typedef Deferred<ASTFunctionCall, boost::intrusive_ptr<ASTFunctionCall>> FunctionCall;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::FunctionCall, 
    (std::string, Content->functionName)
    (std::vector<eddic::ast::Value>, Content->values)
)

#endif
