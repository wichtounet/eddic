//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_SIMPLE_TYPE_H
#define AST_SIMPLE_TYPE_H

#include <string>
#include <ostream>

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

std::ostream& operator<<(std::ostream& out, const ast::SimpleType& type);

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::SimpleType, 
    (bool, const_)
    (std::string, type)
)

#endif
