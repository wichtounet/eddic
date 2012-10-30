//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FrontEnd.hpp"
#include "FrontEnds.hpp"
#include "EDDIFrontEnd.hpp"
#include "Utils.hpp"

using namespace eddic;

std::shared_ptr<FrontEnd> eddic::get_front_end(const std::string& file){
    //Handle .eddi files with the EDDI FrontEnd
    if(has_extension(file, "eddi")){
        return std::make_shared<EDDIFrontEnd>();
    }

    //This kind of file is not handled
    return nullptr;
}
