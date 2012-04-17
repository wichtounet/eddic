//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FRONT_ENDS_H
#define FRONT_ENDS_H

#include <memory>

#include "FrontEnd.hpp"

namespace eddic {

std::shared_ptr<FrontEnd> get_front_end(const std::string& file);

}

#endif
