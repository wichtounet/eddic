//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "SymbolTable.hpp"
#include "Types.hpp"
#include "Type.hpp"

using namespace eddic;

//Global symbol table
SymbolTable eddic::symbols;

SymbolTable::SymbolTable(){
    reset();
}

void SymbolTable::reset(){
    functions.clear();
    structs.clear();
    
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

int SymbolTable::member_offset(std::shared_ptr<Struct> struct_, const std::string& member){
    int offset = 0;

    for(auto m : struct_->members){
        if(m->name == member){
            return offset;
        }

        offset -= m->type->size();
    }

    ASSERT_PATH_NOT_TAKEN("The member is not part of the struct");
}

int SymbolTable::member_offset_reverse(std::shared_ptr<Struct> struct_, const std::string& member){
    int offset = -size_of_struct(struct_->name) + size(BaseType::INT); 

    for(auto m : struct_->members){
        if(m->name == member){
            return offset;
        }

        offset -= m->type->size();
    }

    ASSERT_PATH_NOT_TAKEN("The member is not part of the struct");
}

int SymbolTable::size_of_struct(const std::string& struct_name){
    int struct_size = 0;

    auto struct_ = get_struct(struct_name);

    for(auto m : struct_->members){
        struct_size += m->type->size();
    }
    
    return struct_size;
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

void SymbolTable::addPrintFunction(const std::string& function, std::shared_ptr<Type> parameterType){
    auto printFunction = std::make_shared<Function>(VOID, "print");
    printFunction->mangledName = function;
    printFunction->parameters.push_back({"a", parameterType});
    addFunction(printFunction);
}

void SymbolTable::defineStandardFunctions(){
    //print string
    addPrintFunction("_F5printS", STRING);
    addPrintFunction("_F7printlnS", STRING);

    //print integer
    addPrintFunction("_F5printI", INT);
    addPrintFunction("_F7printlnI", INT);

    //print bool
    addPrintFunction("_F5printB", BOOL);
    addPrintFunction("_F7printlnB", BOOL);

    //print float
    addPrintFunction("_F5printF", FLOAT);
    addPrintFunction("_F7printlnF", FLOAT);
    
    //concat function
    auto concatFunction = std::make_shared<Function>(STRING, "concat");
    concatFunction->mangledName = "_F6concatSS";
    concatFunction->parameters.push_back({"a", STRING});
    concatFunction->parameters.push_back({"b", STRING});
    addFunction(concatFunction);
    
    //time function
    auto timeFunction = std::make_shared<Function>(VOID, "time");
    timeFunction->mangledName = "_F4timeAI";
    timeFunction->parameters.push_back({"a", new_array_type(BaseType::INT)});
    addFunction(timeFunction);
    
    //duration function
    auto durationFunction = std::make_shared<Function>(VOID, "duration");
    durationFunction->mangledName = "_F8durationAIAI";
    durationFunction->parameters.push_back({"a", new_array_type(BaseType::INT)});
    durationFunction->parameters.push_back({"b", new_array_type(BaseType::INT)});
    addFunction(durationFunction);
}
