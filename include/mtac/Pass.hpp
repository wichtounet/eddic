//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_PASS_H
#define MTAC_PASS_H

namespace eddic {

namespace mtac {

//Use for two pass optimization
enum class Pass : unsigned int {
    DATA_MINING,
    OPTIMIZE
};

} //end of mtac

} //end of eddic

#endif
