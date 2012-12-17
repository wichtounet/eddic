//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Function.hpp"
#include "Type.hpp"

#include "mtac/Utils.hpp"

using namespace eddic;

ParameterType::ParameterType(const std::string& n, std::shared_ptr<const Type> t) : name(n), paramType(t) {}

Function::Function(std::shared_ptr<const Type> ret, const std::string& n, const std::string& mangled_name) : returnType(ret), name(n), mangledName(mangled_name), references(0) {}

std::shared_ptr<const Type> Function::getParameterType(const std::string& name) const {
    for(auto& p : parameters){
        if(p.name == name){
            return p.paramType;
        }
    }

    eddic_unreachable("This parameter does not exists in the given function");
}

unsigned int Function::getParameterPositionByType(const std::string& name) const {
    auto type = getParameterType(name);

    if(mtac::is_single_int_register(type)){
        unsigned int position = 0;
        
        for(auto& p : parameters){
            if(mtac::is_single_int_register(p.paramType)){
                ++position; 
            }

            if(p.name == name){
                return position;
            }
        }
        
        eddic_unreachable("This parameter does not exists in the given function");
    } else if(mtac::is_single_float_register(type)){
        unsigned int position = 0;
        
        for(auto& p : parameters){
            if(mtac::is_single_float_register(p.paramType)){
                ++position; 
            }

            if(p.name == name){
                return position;
            }
        }
        
        eddic_unreachable("This parameter does not exists in the given function");
    } else {
        return 0;
    }

}
