//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_RETURN_H
#define AST_RETURN_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3/support/unused.hpp>

#include "ast/Value.hpp"

namespace eddic {

class FunctionContext;

namespace ast {

/*!
 * \class ASTReturn
 * \brief The AST node for a return.
 */
struct Return : x3::file_position_tagged {
    std::string mangled_name;
    std::shared_ptr<FunctionContext> context;

    Value value;
    x3::unused_type fake_;

};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Return,
    (eddic::ast::Value, value)
    (x3::unused_type, fake_)
)

#endif
