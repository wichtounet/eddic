//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"

#include "ast/Null.hpp"

using namespace eddic;

std::ostream& ast::operator<< (std::ostream& stream, ast::Null){
    return stream << "null";
}
