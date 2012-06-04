//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NATIVE_BACK_END_H
#define NATIVE_BACK_END_H

#include "BackEnd.hpp"

namespace eddic {

struct NativeBackEnd : public BackEnd {
    void generate(std::shared_ptr<mtac::Program> mtacProgram);
};

}

#endif
