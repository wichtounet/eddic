//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_MEMBER_VALUE_H
#define AST_MEMBER_VALUE_H

#include <memory>
#include <vector>
#include <string>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \typedef MemberLocation
 * \brief the Type of value that can be accessed to get a member
 * The MemberValue is only nested by transformation never by grammar
 */
typedef boost::variant<MemberValue, VariableValue, ArrayValue> MemberLocation;

/*!
 * \class ASTMemberValue
 * \brief The AST node for a member value.  
 * Should only be used from the Deferred version (eddic::ast::MemberValue).
 */
struct ASTMemberValue {
    std::shared_ptr<Context> context;

    Position position;

    Value location;
    std::vector<std::string> memberNames;

    mutable long references = 0;
};

/*!
 * \struct MemberValue
 * \brief The AST node for a member value.
*/
typedef Deferred<ASTMemberValue> MemberValue;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::MemberValue, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Value, Content->location)
    (std::vector<std::string>, Content->memberNames)
)

#endif
