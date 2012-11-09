//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef EDDI_FRONT_END_H
#define EDDI_FRONT_END_H

#include "FrontEnd.hpp"

namespace eddic {

struct EDDIFrontEnd : public FrontEnd {
    mtac::program_p compile(const std::string& file, Platform platform);
};

}

#endif
