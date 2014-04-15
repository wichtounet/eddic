//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "BackEnds.hpp"
#include "BackEnd.hpp"
#include "NativeBackEnd.hpp"

using namespace eddic;

std::shared_ptr<BackEnd> eddic::get_back_end(Output output){
    switch(output){
        case Output::NATIVE_EXECUTABLE:
            return std::make_shared<NativeBackEnd>();
        default:
            return nullptr;
    }
}
