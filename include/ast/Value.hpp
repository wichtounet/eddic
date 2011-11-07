//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VALUE_H
#define AST_VALUE_H

#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_variant.hpp>

#include "ast/Deferred.hpp"
#include "ast/Integer.hpp"
#include "ast/Litteral.hpp"
#include "ast/Variable.hpp"

namespace eddic {

struct ComposedValue;
typedef Deferred<ComposedValue> ASTComposedValue;

typedef boost::variant<
            ASTInteger, 
            ASTLitteral, 
            ASTVariable,
            boost::recursive_wrapper<ASTComposedValue>
        > ASTValue;

} //end of eddic

#include "ast/ComposedValue.hpp"

#endif
