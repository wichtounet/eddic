//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "SymbolTable.hpp"

using namespace eddic;

SymbolTable::SymbolTable(){
    //Add the standard functions to the function table
    defineStandardFunctions();
}

FunctionMap::const_iterator SymbolTable::begin(){
    return functions.cbegin();
}

FunctionMap::const_iterator SymbolTable::end(){
    return functions.cend();
}

void SymbolTable::addFunction(std::shared_ptr<Function> function){
    functions[function->mangledName] = function;
}

std::shared_ptr<Function> SymbolTable::getFunction(const std::string& function){
    return functions[function];
}

bool SymbolTable::exists(const std::string& function){
    return functions.find(function) != functions.end();
}

void SymbolTable::add_struct(std::shared_ptr<Struct> struct_){
    structs[struct_->name] = struct_;
}

std::shared_ptr<Struct> SymbolTable::get_struct(const std::string& struct_){
    return structs[struct_];
}

int member_offset(std::shared_ptr<Struct> struct_, const std::string& member){
    int offset = 0;

    for(auto& m : struct_->members){
        if(m.name == member){
            return offset;
        }

        offset += size(m.type);
    }

    assert(false);
}

bool SymbolTable::struct_exists(const std::string& struct_){
    return structs.find(struct_) != structs.end();
}

void SymbolTable::addReference(const std::string& function){
    ++(functions[function]->references);
}

int SymbolTable::referenceCount(const std::string& function){
    return functions[function]->references;
}

void SymbolTable::addPrintFunction(const std::string& function, BaseType parameterType){
    auto printFunction = std::make_shared<Function>(newSimpleType(BaseType::VOID), "print");
    printFunction->mangledName = function;
    printFunction->parameters.push_back({"a", newSimpleType(parameterType)});
    addFunction(printFunction);
}

void SymbolTable::defineStandardFunctions(){
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
    
    //time function
    auto timeFunction = std::make_shared<Function>(newSimpleType(BaseType::VOID), "time");
    timeFunction->mangledName = "_F4timeAI";
    timeFunction->parameters.push_back({"a", newArrayType(BaseType::INT)});
    addFunction(timeFunction);
    
    //duration function
    auto durationFunction = std::make_shared<Function>(newSimpleType(BaseType::VOID), "duration");
    durationFunction->mangledName = "_F8durationAIAI";
    durationFunction->parameters.push_back({"a", newArrayType(BaseType::INT)});
    durationFunction->parameters.push_back({"b", newArrayType(BaseType::INT)});
    addFunction(durationFunction);
}
