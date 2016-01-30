//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "Variable.hpp"

#include "mtac/Offset.hpp"

using namespace eddic;

std::ostream& mtac::operator<<(std::ostream& stream, const Offset& offset){
    return stream << "(" << (offset.variable ? offset.variable->name() : "null") << ")" << offset.offset;
}
