//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_FORWARD_DECLARATIONS_H
#define MTAC_FORWARD_DECLARATIONS_H

#include <memory>

#include "variant.hpp"

namespace eddic {

namespace mtac {

struct Program;
class Function;

class basic_block;
typedef std::shared_ptr<mtac::basic_block> basic_block_p;
typedef std::shared_ptr<const mtac::basic_block> basic_block_cp;

struct Quadruple;

} //end of mtac

} //end of eddic

#endif
