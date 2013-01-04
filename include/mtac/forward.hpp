//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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

struct If;
struct IfFalse;
struct Quadruple;

typedef boost::variant<
        std::shared_ptr<mtac::Quadruple>,        //Basic quadruples
        std::shared_ptr<mtac::IfFalse>,          //Jumping quadruples
        std::shared_ptr<mtac::If>               //Jumping quadruples
    > Statement;

} //end of mtac

} //end of eddic

#endif
