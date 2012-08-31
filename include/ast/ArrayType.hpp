//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_ARRAY_TYPE_H
#define AST_ARRAY_TYPE_H

#include <string>
#include <ostream>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/VariableType.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ArrayType
 * \brief An array type in the AST.  
 */
struct ArrayType {
    boost::recursive_wrapper<ast::Type> type;
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
    (boost::recursive_wrapper<eddic::ast::Type>, type)
)

#endif
