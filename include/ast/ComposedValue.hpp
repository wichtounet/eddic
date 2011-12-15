//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_COMPOSED_VALUE_H
#define AST_COMPOSED_VALUE_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/boost_tuple.hpp>

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {

typedef std::vector<boost::tuple<char, Value>> Operations;

struct ASTComposedValue {
    Value first;
    Operations operations;

    mutable long references;
    ASTComposedValue() : references(0) {}
};

typedef Deferred<ASTComposedValue> ComposedValue;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::ComposedValue, 
    (eddic::ast::Value, Content->first)
    (eddic::ast::Operations, Content->operations)
)

#endif