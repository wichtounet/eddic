//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef EDDI_FRONT_END_H
#define EDDI_FRONT_END_H

#include "FrontEnd.hpp"

namespace eddic {

struct EDDIFrontEnd : public FrontEnd {
    std::unique_ptr<mtac::Program> compile(const std::string& file, Platform platform);
};

}

#endif
