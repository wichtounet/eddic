//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_CONDITION_H
#define AST_CONDITION_H

#include <boost/variant/variant.hpp>

#include "ast/False.hpp"
#include "ast/True.hpp"

namespace eddic {

typedef boost::variant<ASTTrue, ASTFalse/*, BinaryCondition*/> ASTCondition;

} //end of eddic

#endif
