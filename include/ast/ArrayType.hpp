//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_ARRAY_TYPE_H
#define AST_ARRAY_TYPE_H

#include <ostream>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/VariableType.hpp"

namespace eddic {

namespace ast {

/*!
 * \struct ArrayType
 * \brief An array type in the AST.
 */
struct ArrayType {
    ast::Type type;
};

bool operator==(const ArrayType& a, const ArrayType& b);

template<typename T>
std::ostream& operator<<(std::ostream& out, const boost::recursive_wrapper<T>& type){
    return out << type.get();
}

std::ostream& operator<<(std::ostream& out, const ast::ArrayType& type);

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::ArrayType,
    (eddic::ast::Type, type)
)

#endif
