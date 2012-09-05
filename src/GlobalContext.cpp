//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GlobalContext.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"
#include "mangling.hpp"

#include "ast/GetConstantValue.hpp"

using namespace eddic;
        
GlobalContext::GlobalContext(Platform platform) : Context(NULL), platform(platform) {
    Val zero = 0;
    
    variables["_mem_start"] = std::make_shared<Variable>("_mem_start", INT, Position(PositionType::GLOBAL, "_mem_start"), zero);
    variables["_mem_start"]->addReference(); //In order to not display a warning

    variables["_mem_last"] = std::make_shared<Variable>("_mem_last", INT, Position(PositionType::GLOBAL, "_mem_last"), zero);
    variables["_mem_last"]->addReference(); //In order to not display a warning
    
    defineStandardFunctions();
}

std::unordered_map<std::string, std::shared_ptr<Variable>> GlobalContext::getVariables(){
    return variables;
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type){
    //Only global array have no value, other types have all values
    assert(type->is_array());

    Position position(PositionType::GLOBAL, variable);
    
    return variables[variable] = std::make_shared<Variable>(variable, type, position);
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type, ast::Value& value){
    auto val = visit(ast::GetConstantValue(), value);
     
    if(type->is_const()){
        Position position(PositionType::CONST);
        return variables[variable] = std::make_shared<Variable>(variable, type, position, val);
    }

    Position position(PositionType::GLOBAL, variable);
    return variables[variable] = std::make_shared<Variable>(variable, type, position, val);
}

void GlobalContext::addFunction(std::shared_ptr<Function> function){
    m_functions[function->mangledName] = function;
}

std::shared_ptr<Function> GlobalContext::getFunction(const std::string& function){
    ASSERT(exists(function), "The function must exists");

    return m_functions[function];
}

bool GlobalContext::exists(const std::string& function){
    return m_functions.find(function) != m_functions.end();
}

void GlobalContext::add_struct(std::shared_ptr<Struct> struct_){
    m_structs[struct_->name] = struct_;
}

std::shared_ptr<Struct> GlobalContext::get_struct(const std::string& struct_){
    return m_structs[struct_];
}

int GlobalContext::member_offset(std::shared_ptr<Struct> struct_, const std::string& member){
    int offset = 0;

    for(auto m : struct_->members){
        if(m->name == member){
            return offset;
        }

        offset += m->type->size(platform);
    }

    ASSERT_PATH_NOT_TAKEN("The member is not part of the struct");
}

std::shared_ptr<const Type> GlobalContext::member_type(std::shared_ptr<Struct> struct_, int offset){
    int current_offset = 0;
    std::shared_ptr<Member> member = nullptr;

    for(auto m : struct_->members){
        member = m;

        if(offset <= current_offset){
            return member->type;
        }
        
        current_offset += m->type->size(platform);
    }

    return member->type;
}

int GlobalContext::size_of_struct(const std::string& struct_name){
    int struct_size = 0;

    auto struct_ = get_struct(struct_name);

    for(auto m : struct_->members){
        struct_size += m->type->size(platform);
    }
    
    return struct_size;
}

bool GlobalContext::is_recursively_nested(const std::string& struct_name, unsigned int left){
    if(left == 0){
        return true;
    }

    auto struct_ = get_struct(struct_name);

    for(auto m : struct_->members){
        auto type = m->type;

        if(type->is_custom_type()){
            if(is_recursively_nested(type->mangle(), left - 1)){
                return true;
            }
        }
    }
    
    return false;
}

bool GlobalContext::is_recursively_nested(const std::string& struct_){
    return is_recursively_nested(struct_, 100);
}

bool GlobalContext::struct_exists(const std::string& struct_){
    return m_structs.find(struct_) != m_structs.end();
}

void GlobalContext::addReference(const std::string& function){
    ASSERT(exists(function), "The function must exists");
    
    ++(m_functions[function]->references);
}

void GlobalContext::removeReference(const std::string& function){
    ASSERT(exists(function), "The function must exists");
    
    --(m_functions[function]->references);
}

int GlobalContext::referenceCount(const std::string& function){
    ASSERT(exists(function), "The function must exists");
    
    return m_functions[function]->references;
}

void GlobalContext::addPrintFunction(const std::string& function, std::shared_ptr<const Type> parameterType){
    auto printFunction = std::make_shared<Function>(VOID, "print");
    printFunction->standard = true;
    printFunction->mangledName = function;
    printFunction->parameters.push_back({"a", parameterType});
    addFunction(printFunction);
}

void GlobalContext::defineStandardFunctions(){
    auto printLineFunction = std::make_shared<Function>(VOID, "print");
    printLineFunction->standard = true;
    printLineFunction->mangledName = "_F7println";
    addFunction(printLineFunction);

    //print string
    addPrintFunction("_F5printS", STRING);
    addPrintFunction("_F7printlnS", STRING);

    //print integer
    addPrintFunction("_F5printI", INT);
    addPrintFunction("_F7printlnI", INT);

    //print char
    addPrintFunction("_F5printC", CHAR);
    addPrintFunction("_F7printlnC", CHAR);

    //print bool
    addPrintFunction("_F5printB", BOOL);
    addPrintFunction("_F7printlnB", BOOL);

    //print float
    addPrintFunction("_F5printF", FLOAT);
    addPrintFunction("_F7printlnF", FLOAT);

    auto read_char_function = std::make_shared<Function>(CHAR, "read_char");
    read_char_function->standard = true;
    read_char_function->mangledName = "_F9read_char";
    addFunction(read_char_function);
    
    //concat function
    auto concatFunction = std::make_shared<Function>(STRING, "concat");
    concatFunction->standard = true;
    concatFunction->mangledName = "_F6concatSS";
    concatFunction->parameters.push_back({"a", STRING});
    concatFunction->parameters.push_back({"b", STRING});
    addFunction(concatFunction);
    
    //alloc function
    auto allocFunction = std::make_shared<Function>(new_pointer_type(INT), "alloc");
    allocFunction->standard = true;
    allocFunction->mangledName = "_F5allocI";
    allocFunction->parameters.push_back({"a", INT});
    addFunction(allocFunction);
    
    //free function
    auto freeFunction = std::make_shared<Function>(VOID, "free");
    freeFunction->standard = true;
    freeFunction->mangledName = "_F4freePI";
    freeFunction->parameters.push_back({"a", INT});
    addFunction(freeFunction);
    
    //time function
    auto timeFunction = std::make_shared<Function>(VOID, "time");
    timeFunction->standard = true;
    timeFunction->mangledName = "_F4timeAI";
    timeFunction->parameters.push_back({"a", new_array_type(INT)});
    addFunction(timeFunction);
    
    //duration function
    auto durationFunction = std::make_shared<Function>(VOID, "duration");
    durationFunction->standard = true;
    durationFunction->mangledName = "_F8durationAIAI";
    durationFunction->parameters.push_back({"a", new_array_type(INT)});
    durationFunction->parameters.push_back({"b", new_array_type(INT)});
    addFunction(durationFunction);
}

GlobalContext::FunctionMap GlobalContext::functions(){
    return m_functions;
}
