//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"

#include "mtac/Offset.hpp"

using namespace eddic;

std::ostream& mtac::operator<<(std::ostream& stream, const Offset& offset){
    return stream << "(" << (offset.variable ? offset.variable->name() : "null") << ")" << offset.offset;
}
