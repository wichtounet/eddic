//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GET_TYPE_VISITOR_H
#define GET_TYPE_VISITOR_H

#include <boost/variant/static_visitor.hpp>

#include "Types.hpp"

#include "ast/values_def.hpp"

namespace eddic {

struct GetTypeVisitor : public boost::static_visitor<Type> {
    Type operator()(const ast::Litteral& litteral) const;
    Type operator()(const ast::Integer& litteral) const;
    Type operator()(const ast::VariableValue& variable) const;
    Type operator()(const ast::ComposedValue& value) const; 
};

} //end of eddic

#endif
