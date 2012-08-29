//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_SIMPLE_TYPE_H
#define AST_SIMPLE_TYPE_H

#include <string>

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

namespace ast {

/*!
 * \class SimpleType
 * \brief A simple (non-array) type in the AST.  
 */
struct SimpleType {
    bool const_;
    std::string type;
};

bool operator==(const SimpleType& a, const SimpleType& b);

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::SimpleType, 
    (bool, const_)
    (std::string, type)
)

#endif
