//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
