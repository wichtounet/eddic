//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPE_TRANSFORMER_H
#define TYPE_TRANSFORMER_H

#include <boost/variant/static_visitor.hpp>

#include "ast/Type.hpp"
#include "Types.hpp"

namespace eddic {

namespace ast {

/*!
 * \struct TypeTransformer
 * \brief AST visitor to transform an AST type into a type descriptor.   
 */
struct TypeTransformer : public boost::static_visitor<eddic::Type> {
    eddic::Type operator()(ast::SimpleType& type) const;
    eddic::Type operator()(ast::ArrayType& type) const;
};

} //end of ast

} //end of eddic

#endif
