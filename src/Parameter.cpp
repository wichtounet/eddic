//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Parameter.hpp"

using namespace eddic;

Parameter::Parameter(const std::string& name, std::shared_ptr<const Type> type) : _name(name), _type(type){
    //Nothing to do
}

Parameter::Parameter(Parameter&& rhs) : _name(std::move(rhs._name)), _type(std::move(rhs._type)) {
    //Nothing to do 
}

Parameter& Parameter::operator=(Parameter&& rhs){
    _name = std::move(rhs._name);
    _type = std::move(rhs._type);

    return *this;
}

const std::string& Parameter::name() const {
    return _name;
}

const std::shared_ptr<const Type>& Parameter::type() const {
    return _type;
}
