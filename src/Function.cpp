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

Function::Function(std::shared_ptr<const Type> ret, const std::string& n, const std::string& mangled_name) : returnType(ret), name(n), mangledName(mangled_name), references(0) {}
        
const Parameter& Function::parameter(std::size_t i) const {
    return m_parameters.at(i);
}

std::vector<Parameter>& Function::parameters(){
    return m_parameters;
}

const std::vector<Parameter>& Function::parameters() const {
    return m_parameters;
}

std::shared_ptr<const Type> Function::getParameter(const std::string& name) const {
    for(auto& p : m_parameters){
        if(p.name() == name){
            return p.type();
        }
    }

    eddic_unreachable("This parameter does not exists in the given function");
}

unsigned int Function::getParameterPositionByType(const std::string& name) const {
    auto type = getParameter(name);

    if(mtac::is_single_int_register(type)){
        unsigned int position = 0;
        
        for(auto& p : m_parameters){
            if(mtac::is_single_int_register(p.type())){
                ++position; 
            }

            if(p.name() == name){
                return position;
            }
        }
        
        eddic_unreachable("This parameter does not exists in the given function");
    } else if(mtac::is_single_float_register(type)){
        unsigned int position = 0;
        
        for(auto& p : m_parameters){
            if(mtac::is_single_float_register(p.type())){
                ++position; 
            }

            if(p.name() == name){
                return position;
            }
        }
        
        eddic_unreachable("This parameter does not exists in the given function");
    } else {
        return 0;
    }
}

bool Function::operator==(const Function& rhs) const {
    return mangledName == rhs.mangledName;
}
