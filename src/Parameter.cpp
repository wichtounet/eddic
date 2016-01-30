//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
