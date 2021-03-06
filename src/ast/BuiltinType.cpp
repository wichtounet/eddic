//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "ast/Value.hpp"
#include "ast/BuiltinOperator.hpp"

using namespace eddic;

std::ostream& ast::operator<< (std::ostream& stream, ast::BuiltinType type){
    switch(type){
        case ast::BuiltinType::SIZE:
            return stream << "SIZE";
        case ast::BuiltinType::LENGTH:
            return stream << "LENGTH";
        default:
            cpp_unreachable("Unhandled builtin type");
    }
}
