//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "BackEnds.hpp"
#include "BackEnd.hpp"
#include "NativeBackEnd.hpp"

using namespace eddic;

std::unique_ptr<BackEnd> eddic::get_back_end(Output output){
    switch(output){
        case Output::NATIVE_EXECUTABLE:
            return std::make_unique<NativeBackEnd>();
        default:
            return nullptr;
    }
}
