//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_TEMPLATE_TYPE_H
#define AST_TEMPLATE_TYPE_H

#include <vector>
#include <string>

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

namespace ast {

class SimpleType;
class TemplateType;
class PointerType;
class ArrayType;

typedef boost::variant<SimpleType, ArrayType, TemplateType, PointerType> Type;

/*!
 * \class TemplateType
 * \brief A template type in the AST.  
 */
struct TemplateType {
    std::string type;
    std::vector<Type> template_types;
    bool resolved = false;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::TemplateType, 
    (std::string, type)
    (std::vector<eddic::ast::Type>, template_types)
)

#endif
