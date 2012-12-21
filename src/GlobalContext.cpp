//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
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
    variables["_mem_last"] = std::make_shared<Variable>("_mem_last", INT, Position(PositionType::GLOBAL, "_mem_last"), zero);
    
    //In order to not display a warning
    variables["_mem_start"]->add_reference();
    variables["_mem_last"]->add_reference();      

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

std::shared_ptr<Variable> GlobalContext::generate_variable(const std::string&, std::shared_ptr<const Type>){
    eddic_unreachable("Cannot generate global variable");
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

Function& GlobalContext::add_function(std::shared_ptr<const Type> ret, const std::string& name, const std::string& mangled_name){
    m_functions.emplace(std::piecewise_construct, std::forward_as_tuple(mangled_name), std::forward_as_tuple(ret, name, mangled_name));

    return m_functions.at(mangled_name);
}

Function& GlobalContext::getFunction(const std::string& function){
    eddic_assert(exists(function), "The function must exists");

    return m_functions.at(function);
}

bool GlobalContext::exists(const std::string& function) const {
    return m_functions.find(function) != m_functions.end();
}

void GlobalContext::add_struct(std::shared_ptr<Struct> struct_){
    m_structs[struct_->name] = struct_;
}

bool GlobalContext::struct_exists(const std::string& struct_) const {
    return m_structs.find(struct_) != m_structs.end();
}

bool GlobalContext::struct_exists(std::shared_ptr<const Type> type) const {
    if(type->is_pointer()){
        type = type->data_type();
    }

    eddic_assert(type->is_custom_type() || type->is_template_type(), "This type has no corresponding struct");
    
    auto struct_name = type->mangle();
    return struct_exists(struct_name);
}

std::shared_ptr<Struct> GlobalContext::get_struct(const std::string& struct_name) const {
    auto it = m_structs.find(struct_name);
    
    if(it == m_structs.end()){
        return nullptr;
    } else {
        return it->second;
    }
}
        
std::shared_ptr<Struct> GlobalContext::get_struct(std::shared_ptr<const Type> type) const {
    if(!type){
        return nullptr;
    }

    if(type->is_pointer()){
        type = type->data_type();
    }

    eddic_assert(type->is_custom_type() || type->is_template_type(), "This type has no corresponding struct");
    
    auto struct_name = type->mangle();
    return get_struct(struct_name);
}

int GlobalContext::member_offset(std::shared_ptr<const Struct> struct_, const std::string& member) const {
    int offset = 0;

    for(auto& m : struct_->members){
        if(m->name == member){
            return offset;
        }

        offset += m->type->size(platform);
    }

    eddic_unreachable("The member is not part of the struct");
}

std::shared_ptr<const Type> GlobalContext::member_type(std::shared_ptr<const Struct> struct_, int offset) const {
    int current_offset = 0;
    std::shared_ptr<Member> member = nullptr;

    for(auto& m : struct_->members){
        member = m;

        if(offset <= current_offset){
            return member->type;
        }
        
        current_offset += m->type->size(platform);
    }

    return member->type;
}

int GlobalContext::self_size_of_struct(std::shared_ptr<const Struct> struct_) const {
    int struct_size = 0;

    for(auto& m : struct_->members){
        struct_size += m->type->size(platform);
    }
    
    return struct_size;
}

int GlobalContext::total_size_of_struct(std::shared_ptr<const Struct> struct_) const {
   int total = self_size_of_struct(struct_);

   while(struct_->parent_type){
       struct_ = get_struct(struct_->parent_type);
       total += self_size_of_struct(struct_);
   }

   return total;
}

bool GlobalContext::is_recursively_nested(std::shared_ptr<const Struct> struct_, unsigned int left) const {
    if(left == 0){
        return true;
    }

    for(auto& m : struct_->members){
        auto type = m->type;

        if(type->is_structure()){
            if(is_recursively_nested(get_struct(type), left - 1)){
                return true;
            }
        }
    }
    
    return false;
}

bool GlobalContext::is_recursively_nested(std::shared_ptr<const Struct> struct_) const {
    return is_recursively_nested(struct_, 100);
}

void GlobalContext::addReference(const std::string& function){
    ++(getFunction(function).references);
}

void GlobalContext::removeReference(const std::string& function){
    --(getFunction(function).references);
}

int GlobalContext::referenceCount(const std::string& function){
    return getFunction(function).references;
}

void GlobalContext::addPrintFunction(const std::string& function, std::shared_ptr<const Type> parameterType){
    auto& printFunction = add_function(VOID, "print", function);
    printFunction.standard = true;
    printFunction.parameters().emplace_back("a", parameterType);
}

void GlobalContext::defineStandardFunctions(){
    auto& printLineFunction = add_function(VOID, "print", "_F7println");
    printLineFunction.standard = true;

    //print string
    addPrintFunction("_F5printS", STRING);
    addPrintFunction("_F7printlnS", STRING);

    //print integer
    addPrintFunction("_F5printI", INT);
    addPrintFunction("_F7printlnI", INT);

    //print char
    addPrintFunction("_F5printC", CHAR);
    addPrintFunction("_F7printlnC", CHAR);

    //print float
    addPrintFunction("_F5printF", FLOAT);
    addPrintFunction("_F7printlnF", FLOAT);

    auto& read_char_function = add_function(CHAR, "read_char", "_F9read_char");
    read_char_function.standard = true;
    
    //alloc function
    auto& allocFunction = add_function(new_pointer_type(INT), "alloc", "_F5allocI");
    allocFunction.standard = true;
    allocFunction.parameters().emplace_back("a", INT);
    
    //free function
    auto& freeFunction = add_function(VOID, "free", "_F4freePI");
    freeFunction.standard = true;
    freeFunction.parameters().emplace_back("a", INT);
    
    //time function
    auto& timeFunction = add_function(VOID, "time", "_F4timeAI");
    timeFunction.standard = true;
    timeFunction.parameters().emplace_back("a", new_array_type(INT));
    
    //duration function
    auto& durationFunction = add_function(VOID, "duration", "_F8durationAIAI");
    durationFunction.standard = true;
    durationFunction.parameters().emplace_back("a", new_array_type(INT));
    durationFunction.parameters().emplace_back("b", new_array_type(INT));
}

const GlobalContext::FunctionMap& GlobalContext::functions() const {
    return m_functions;
}

Platform GlobalContext::target_platform() const {
    return platform;
}
