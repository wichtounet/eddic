//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Utils.hpp"

using std::map;
using std::string;
using std::endl;

using namespace eddic;

//TODO Do not take parameters into account for counting (result into useless stack allocations in the function)
int FunctionContext::size(){
    int s = 0;
    for(auto it : m_stored){
        s += ::size(it.second->type());
    }

    return s;
}

std::shared_ptr<Variable> FunctionContext::newParameter(const std::string& variable, Type type){
    Position position(PARAMETER, currentParameter);
    
    currentParameter += ::size(type);

    return std::shared_ptr<Variable>(new Variable(variable, type, position));
}

std::shared_ptr<Variable> FunctionContext::newVariable(const std::string& variable, Type type){
    Position position(STACK, currentPosition);
    
    currentPosition += ::size(type);

    return std::shared_ptr<Variable>(new Variable(variable, type, position));
}

std::shared_ptr<Variable> FunctionContext::addVariable(const std::string& variable, Type type){
    std::shared_ptr<Variable> v = newVariable(variable, type);

    m_visibles[variable] = currentVariable;

    storeVariable(currentVariable, v);
    
    currentVariable++;

    return v;
}

std::shared_ptr<Variable> FunctionContext::addParameter(const std::string& parameter, Type type){
   std::shared_ptr<Variable> v = newParameter(parameter, type);

   m_visibles[parameter] = currentVariable;

   storeVariable(currentVariable, v);

   currentVariable++;

   return v;
}
