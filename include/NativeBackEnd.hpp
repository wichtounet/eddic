//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef NATIVE_BACK_END_H
#define NATIVE_BACK_END_H

#include "BackEnd.hpp"

namespace eddic {

struct NativeBackEnd : public BackEnd {
    void generate(mtac::Program& program, Platform platform) override;
};

}

#endif
