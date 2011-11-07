//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_BINARY_CONDITION_H
#define AST_BINARY_CONDITION_H

#include <boost/variant/variant.hpp>

#include "ast/Equals.hpp"
#include "ast/NotEquals.hpp"
#include "ast/Less.hpp"
#include "ast/LessEquals.hpp"
#include "ast/Greater.hpp"
#include "ast/GreaterEquals.hpp"

namespace eddic {

typedef boost::variant<
            ASTEquals, 
            ASTNotEquals, 
            ASTGreater,
            ASTGreaterEquals, 
            ASTLess, 
            ASTLessEquals>
        ASTBinaryCondition;

} //end of eddic

#endif
