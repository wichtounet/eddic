//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "Function.hpp"

#include "mtac/Program.hpp"

using namespace eddic;

mtac::Program::Program(){
    //Nothing to do
}

mtac::Function& mtac::Program::mtac_function(const eddic::Function& function){
    for(auto& f : functions){
        if(f.definition() == function){
            return f;
        }
    }

    cpp_unreachable(("There are no function \"" + function.mangled_name() + "\"").c_str());
}

std::ostream& mtac::operator<<(std::ostream& stream, mtac::Program& program){
    stream << "TAC Program " << std::endl << std::endl; 

    for(auto& function : program.functions){
        stream << function;
    }

    return stream;
}
