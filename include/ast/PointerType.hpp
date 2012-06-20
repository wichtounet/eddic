//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_POINTER_TYPE_H
#define AST_POINTER_TYPE_H

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

namespace ast {

/*!
 * \class PointerType
 * \brief A pointer type in the AST.  
 */
struct PointerType {
    std::string type;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::PointerType, 
    (std::string, type)
)

#endif
