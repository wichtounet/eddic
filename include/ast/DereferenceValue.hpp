//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_DEREFERENCE_VARIABLE_VALUE_H
#define AST_DEREFERENCE_VARIABLE_VALUE_H

#include <memory>
#include <vector>
#include <string>

#include <boost/variant.hpp>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;
class Variable;

namespace ast {

typedef boost::variant<ast::VariableValue, ast::ArrayValue> Ref;

/*!
 * \class ASTDereferenceValue
 * \brief The AST node for a variable value.  
 * Should only be used from the Deferred version (eddic::ast::VariableValue).
 */
struct ASTDereferenceValue {
    Position position;
    char op;
    Ref ref;

    mutable long references = 0;
};

/*!
 * \typedef DereferenceValue
 * \brief The AST node for a variable value.
*/
typedef Deferred<ASTDereferenceValue> DereferenceValue;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::DereferenceValue, 
    (eddic::ast::Position, Content->position)
    (char, Content->op)
    (eddic::ast::Ref, Content->ref)
)

#endif
