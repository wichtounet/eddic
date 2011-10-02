//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Options.hpp"

using namespace eddic;

bool Options::booleanOptions[(int) BooleanOption::COUNT];
std::string Options::valueOptions[(int) ValueOption::COUNT];
