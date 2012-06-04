//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
