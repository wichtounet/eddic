//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BACK_ENDS_H
#define BACK_ENDS_H

#include <memory>

#include "BackEnd.hpp"

namespace eddic {

enum class Output : unsigned int {
    NATIVE_EXECUTABLE
};

std::shared_ptr<BackEnd> get_back_end(Output output);

}

#endif
