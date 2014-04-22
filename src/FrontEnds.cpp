//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "FrontEnd.hpp"
#include "FrontEnds.hpp"
#include "EDDIFrontEnd.hpp"
#include "Utils.hpp"

using namespace eddic;

std::unique_ptr<FrontEnd> eddic::get_front_end(const std::string& file){
    //Handle .eddi files with the EDDI FrontEnd
    if(has_extension(file, "eddi")){
        return std::make_unique<EDDIFrontEnd>();
    }

    //This kind of file is not handled
    return nullptr;
}
