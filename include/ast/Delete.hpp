//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_DELETE_H
#define AST_DELETE_H

#include <boost/spirit/home/x3/support/unused.hpp>

#include "ast/Position.hpp"

namespace eddic {

namespace ast {

/*!
 * \class Delete
 * \brief The AST node for delete a variable.
 */
struct Delete : x3::file_position_tagged {
    Position position;
    Value value;
	x3::unused_type fake_;

    mutable long references = 0;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Delete,
    (eddic::ast::Value, value)
    (x3::unused_type, fake_)
)

#endif
