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

Function::Function(std::shared_ptr<const Type> return_type, const std::string& name, const std::string& mangled_name) 
        : _return_type(return_type), _name(name), _mangled_name(mangled_name) {
    //Nothing to do
}
        
const Parameter& Function::parameter(std::size_t i) const {
    return _parameters.at(i);
}

const Parameter& Function::parameter(const std::string& name) const {
    for(auto& p : _parameters){
        if(p.name() == name){
            return p;
        }
    }

    eddic_unreachable("This parameter does not exists in the given function");
}

std::vector<Parameter>& Function::parameters(){
    return _parameters;
}

const std::vector<Parameter>& Function::parameters() const {
    return _parameters;
}

unsigned int Function::parameter_position_by_type(const std::string& name) const {
    auto type = parameter(name).type();

    if(mtac::is_single_int_register(type)){
        unsigned int position = 0;
        
        for(auto& p : _parameters){
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
        
        for(auto& p : _parameters){
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
    return _mangled_name == rhs._mangled_name;
}

std::shared_ptr<FunctionContext>& Function::context(){
    return _context;
}

const std::shared_ptr<FunctionContext>& Function::context() const {
    return _context;
}

std::shared_ptr<const Type>& Function::struct_type(){
    return _struct_type;
}

int Function::references() const {
   return _references; 
}

int& Function::references(){
   return _references; 
}

bool Function::standard() const {
    return _standard;
}

bool& Function::standard(){
    return _standard;
}

const std::shared_ptr<const Type>& Function::return_type() const {
    return _return_type;
}

const std::string& Function::name(){
    return _name;
}

const std::string& Function::mangled_name(){
    return _mangled_name;
}
