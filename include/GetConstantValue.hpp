//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GET_CONSTANT_VALUE_H
#define GET_CONSTANT_VALUE_H

#include <string>
#include <utility>

#include "ast/Deferred.hpp"

#include <boost/variant/variant.hpp>
#include <boost/variant/static_visitor.hpp>

namespace eddic {

class ASTLitteral;
class ASTInteger;

struct TmpVariable;
typedef Deferred<TmpVariable> ASTVariable;

struct ComposedValue;
typedef Deferred<ComposedValue> ASTComposedValue;

typedef boost::variant<int, std::pair<std::string, int>> Val;

struct GetConstantValue : public boost::static_visitor<Val> {
    Val operator()(const ASTLitteral& litteral) const;
    Val operator()(const ASTInteger& litteral) const;
    Val operator()(const ASTVariable& variable) const;
    Val operator()(const ASTComposedValue& value) const; 
};

} //end of eddic

#endif
