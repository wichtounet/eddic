//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FunctionTable.hpp"

using namespace eddic;

FunctionTable::FunctionTable(){
    //Add the standard functions to the function table
    defineStandardFunctions();
}

FunctionMap::const_iterator FunctionTable::begin(){
    return functions.cbegin();
}

FunctionMap::const_iterator FunctionTable::end(){
    return functions.cend();
}

void FunctionTable::addFunction(std::shared_ptr<Function> function){
    functions[function->mangledName] = function;
}

std::shared_ptr<Function> FunctionTable::getFunction(const std::string& function){
    return functions[function];
}

bool FunctionTable::exists(const std::string& function){
    return functions.find(function) != functions.end();
}

void FunctionTable::addReference(const std::string& function){
    ++(functions[function]->references);
}

int FunctionTable::referenceCount(const std::string& function){
    return functions[function]->references;
}

void FunctionTable::addPrintFunction(const std::string& function, BaseType parameterType){
    auto printFunction = std::make_shared<Function>(newSimpleType(BaseType::VOID), "print");
    printFunction->mangledName = function;
    printFunction->parameters.push_back({"a", newSimpleType(parameterType)});
    addFunction(printFunction);
}

void FunctionTable::defineStandardFunctions(){
    //Function time()
    auto timeFunction = std::make_shared<Function>(newSimpleType(BaseType::INT), "time");
    timeFunction->mangledName = "_F4timeAI";
    addFunction(timeFunction);

    //print string
    addPrintFunction("_F5printS", BaseType::STRING);
    addPrintFunction("_F7printlnS", BaseType::STRING);

    //print integer
    addPrintFunction("_F5printI", BaseType::INT);
    addPrintFunction("_F7printlnI", BaseType::INT);

    //print bool
    addPrintFunction("_F5printB", BaseType::BOOL);
    addPrintFunction("_F7printlnB", BaseType::BOOL);

    //print float
    addPrintFunction("_F5printF", BaseType::FLOAT);
    addPrintFunction("_F7printlnF", BaseType::FLOAT);
    
    //concat function
    auto concatFunction = std::make_shared<Function>(newSimpleType(BaseType::STRING), "concat");
    concatFunction->mangledName = "_F6concatSS";
    concatFunction->parameters.push_back({"a", newSimpleType(BaseType::STRING)});
    concatFunction->parameters.push_back({"b", newSimpleType(BaseType::STRING)});
    addFunction(concatFunction);
}

Type Function::getParameterType(const std::string& name){
    for(auto& p : parameters){
        if(p.name == name){
            return p.paramType;
        }
    }
    
    assert(false && "This parameter does not exists in the given function");
}

unsigned int Function::getParameterPositionByType(const std::string& name){
    unsigned int position = 0;

    auto type = getParameterType(name);
    
    for(auto& p : parameters){
        if(p.paramType == type){
            ++position; 
        }

        if(p.name == name){
            return position;
        }
    }

    assert(false && "The parameter does not exists in the function");
}
