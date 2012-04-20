//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PREFIX_OPERATION_H
#define AST_PREFIX_OPERATION_H

#include <memory>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Operator.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;
class Variable;

namespace ast {

struct ASTPrefixOperation {
    std::shared_ptr<Context> context;

    Position position;
    std::string variableName;
    std::shared_ptr<Variable> variable;
    ast::Operator op;

    mutable long references = 0;
};

typedef Deferred<ASTPrefixOperation> PrefixOperation;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::PrefixOperation, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Operator, Content->op)
    (std::string, Content->variableName)
)

#endif
