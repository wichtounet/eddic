//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef FRONT_ENDS_H
#define FRONT_ENDS_H

#include <memory>
#include <string>

namespace eddic {

class FrontEnd;

std::unique_ptr<FrontEnd> get_front_end(const std::string& file);

}

#endif
