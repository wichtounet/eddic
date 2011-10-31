//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VALUE_H
#define AST_VALUE_H

#include <boost/variant/variant.hpp>

#include "ast/Integer.hpp"
#include "ast/Litteral.hpp"

namespace eddic {

typedef boost::variant<ASTInteger, ASTLitteral> ASTValue;

} //end of eddic

#endif
