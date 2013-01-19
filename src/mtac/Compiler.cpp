//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <utility>

#include <boost/range/adaptors.hpp>

#include "assert.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"
#include "SemanticalException.hpp"
#include "FunctionContext.hpp"
#include "mangling.hpp"
#include "Labels.hpp"
#include "Type.hpp"
#include "PerfsTimer.hpp"
#include "GlobalContext.hpp"

#include "mtac/Compiler.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

#include "ast/SourceFile.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/ASTVisitor.hpp"

using namespace eddic;

namespace {

typedef std::vector<mtac::Argument> arguments;

/* Assignments (left_value = value) */

void assign(mtac::Function& function, ast::Value& left_value, ast::Value& value);
void assign(mtac::Function& function, std::shared_ptr<Variable> left_value, ast::Value& value);

/* Compiles Values in Arguments (use ToArgumentsVisitor) */

mtac::Argument moveToArgument(ast::Value value, mtac::Function& function);
arguments move_to_arguments(ast::Value value, mtac::Function& function);

void pass_arguments(mtac::Function& function, eddic::Function& definition, std::vector<ast::Value>& values);

arguments compile_ternary(mtac::Function& function, ast::Ternary& ternary);

mtac::Argument index_of_array(std::shared_ptr<Variable> array, ast::Value indexValue, mtac::Function& function){
    mtac::Argument index = moveToArgument(indexValue, function);
    
    auto temp = function.context->new_temporary(INT);

    function.emplace_back(temp, index, mtac::Operator::MUL, array->type()->data_type()->size(function.context->global()->target_platform()));
    function.emplace_back(temp, temp, mtac::Operator::ADD, INT->size(function.context->global()->target_platform()));
   
    return temp;
}

std::vector<std::shared_ptr<const Type>> to_types(std::vector<ast::Value>& values){
    std::vector<std::shared_ptr<const Type>> types;

    for(auto& value : values){
        types.push_back(visit(ast::GetTypeVisitor(), value));
    }

    return types;
}

void construct(mtac::Function& function, std::shared_ptr<const Type> type, std::vector<ast::Value> values, mtac::Argument this_arg){
    auto ctor_name = mangle_ctor(to_types(values), type);
    auto global_context = function.context->global();

    //Get the constructor
    eddic_assert(global_context->exists(ctor_name), "The constructor must exists");
    auto& ctor_function = global_context->getFunction(ctor_name);

    //Pass all normal arguments
    pass_arguments(function, ctor_function, values);

    //Pass "this" parameter
    function.emplace_back(mtac::Operator::PPARAM, this_arg, ctor_function.context()->getVariable(ctor_function.parameter(0).name()), ctor_function);

    //Call the constructor
    global_context->addReference(ctor_name);
    function.emplace_back(mtac::Operator::CALL, ctor_function); 
}

void copy_construct(mtac::Function& function, std::shared_ptr<const Type> type, mtac::Argument this_arg, ast::Value rhs_arg){
    std::vector<std::shared_ptr<const Type>> ctor_types = {new_pointer_type(type)};
    auto ctor_name = mangle_ctor(ctor_types, type);

    auto global_context = function.context->global();

    eddic_assert(global_context->exists(ctor_name), "The copy constructor must exists. Something went wrong in default generation");

    auto& ctor_function = global_context->getFunction(ctor_name);

    //The values to be passed to the copy constructor
    std::vector<ast::Value> values;
    values.push_back(rhs_arg);

    //Pass the other structure (the pointer will automatically be handled
    pass_arguments(function, ctor_function, values);

    function.emplace_back(mtac::Operator::PPARAM, this_arg, ctor_function.context()->getVariable(ctor_function.parameter(0).name()), ctor_function);

    global_context->addReference(ctor_name);
    function.emplace_back(mtac::Operator::CALL, ctor_function); 
}

void destruct(mtac::Function& function, std::shared_ptr<const Type> type, mtac::Argument this_arg){
    auto global_context = function.context->global();
    auto dtor_name = mangle_dtor(type);

    eddic_assert(global_context->exists(dtor_name), "The destructor must exists");

    auto& dtor_function = global_context->getFunction(dtor_name);

    function.emplace_back(mtac::Operator::PPARAM, this_arg, dtor_function.context()->getVariable(dtor_function.parameter(0).name()), dtor_function);

    global_context->addReference(dtor_name);
    function.emplace_back(mtac::Operator::CALL, dtor_function); 
}

template<typename Source>
Offset variant_cast(Source source){
    if(auto* ptr = boost::get<int>(&source)){
        return *ptr;
    } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&source)){
        return *ptr;
    } else {
        eddic_unreachable("Invalid source type");
    }
}

void jump_if_true(mtac::Function& function, const std::string& l, ast::Value value){
    auto argument = moveToArgument(value, function);
    function.emplace_back(mtac::Operator::IF_UNARY, argument, l);
}

void jump_if_false(mtac::Function& function, const std::string& l, ast::Value value){
    auto argument = moveToArgument(value, function);
    function.emplace_back(mtac::Operator::IF_FALSE_UNARY, argument, l);
}

arguments struct_to_arguments(mtac::Function& function, std::shared_ptr<const Type> type, std::shared_ptr<Variable> base_var, unsigned int offset);

enum class ArgumentType : unsigned int {
    NORMAL,
    ADDRESS,
    REFERENCE
};
    
template<ArgumentType T = ArgumentType::NORMAL>
arguments get_member(mtac::Function& function, unsigned int offset, std::shared_ptr<const Type> member_type, std::shared_ptr<Variable> var){
    auto platform = function.context->global()->target_platform();

    if(member_type == STRING){
        auto t1 = function.context->new_temporary(INT);
        auto t2 = function.context->new_temporary(INT);

        function.emplace_back(t1, var, mtac::Operator::DOT, offset);
        function.emplace_back(t2, var, mtac::Operator::DOT, offset + INT->size(platform));

        return {t1, t2};
    } else if(member_type->is_array() && !member_type->is_dynamic_array()){
        //Get a reference to the array
        if(T == ArgumentType::REFERENCE){
            return {function.context->new_reference(member_type, var, offset)};
        } 
        //Get all the values of an array
        else {
            auto elements = member_type->elements();
            auto data_type = member_type->data_type();

            arguments result;

            //All the elements of the array
            for(unsigned int i = 0; i < elements; ++i){
                auto index_offset = offset + i * data_type->size(platform);

                if(data_type == STRING){
                    auto t1 = function.context->new_temporary(INT);
                    auto t2 = function.context->new_temporary(INT);
                    
                    function.emplace_back(t1, var, mtac::Operator::DOT, index_offset);
                    function.emplace_back(t2, var, mtac::Operator::DOT, index_offset + INT->size(function.context->global()->target_platform()));

                    result.push_back(t1);
                    result.push_back(t2);
                } else if(data_type->is_custom_type()){
                    auto base_var = function.context->new_reference(member_type, var, offset);

                    auto struct_type = function.context->global()->get_struct(data_type);

                    for(auto& member : struct_type->members){
                        std::shared_ptr<const Type> member_type;
                        unsigned int offset = 0;
                        boost::tie(offset, member_type) = mtac::compute_member(function.context->global(), base_var->type(), member->name);

                        auto new_args = get_member(function, offset, member_type, base_var);
                        std::copy(new_args.begin(), new_args.end(), std::back_inserter(result));
                    }
                } else if(data_type == FLOAT || data_type == INT || data_type == CHAR || data_type == BOOL || data_type->is_pointer()){
                    auto temp = function.context->new_temporary(data_type);

                    if(data_type == FLOAT){
                        function.emplace_back(temp, var, mtac::Operator::FDOT, index_offset);
                    } else if(data_type == INT || data_type == CHAR || data_type == BOOL || data_type->is_pointer()){
                        function.emplace_back(temp, var, mtac::Operator::DOT, index_offset);
                    }

                    result.push_back(temp);
                } else {
                    eddic_unreachable("Unhandled type");
                }
            }

            //The number of elements
            result.push_back(elements);

            return result;
        }
    } else if(member_type->is_structure()){
        if(T == ArgumentType::REFERENCE){
            return {function.context->new_reference(member_type, var, offset)};
        } else if(T == ArgumentType::NORMAL){
            return struct_to_arguments(function, member_type, var, offset);      
        }

        eddic_unreachable("Unhandled ArgumentType");
    } else {
        std::shared_ptr<Variable> temp;
        if(T == ArgumentType::REFERENCE){
            temp = function.context->new_reference(member_type, var, offset);
        } else {
            temp = function.context->new_temporary(member_type);
        }

        if(member_type == FLOAT){
            function.emplace_back(temp, var, mtac::Operator::FDOT, offset);
        } else if(member_type == INT || member_type == CHAR || member_type == BOOL || member_type->is_pointer() || member_type->is_dynamic_array()){
            function.emplace_back(temp, var, mtac::Operator::DOT, offset);
        } else if(member_type->is_custom_type() && T == ArgumentType::REFERENCE){
            //In this case, the reference is not initialized, will be used to refer to the member
        } else {
            eddic_unreachable("Unhandled type");
        }

        return {temp};
    }
}

arguments struct_to_arguments(mtac::Function& function, std::shared_ptr<const Type> type, std::shared_ptr<Variable> base_var, unsigned int offset){
    arguments result;

    auto struct_type = function.context->global()->get_struct(type);

    for(auto& member : struct_type->members){
        std::shared_ptr<const Type> member_type;
        unsigned int member_offset = 0;
        boost::tie(member_offset, member_type) = mtac::compute_member(function.context->global(), type, member->name);

        auto new_args = get_member<>(function, member_offset + offset, member_type, base_var);
        std::copy(new_args.begin(), new_args.end(), std::back_inserter(result));
    }

    return result;
}

template<ArgumentType T = ArgumentType::NORMAL>
arguments compute_expression_operation(mtac::Function& function, std::shared_ptr<const Type> type, arguments& left, ast::Operation& operation){
    auto operation_value = operation.get<1>();
    auto op = operation.get<0>();

    switch(op){
        case ast::Operator::ADD:
        case ast::Operator::SUB:
        case ast::Operator::MUL:
        case ast::Operator::MOD:
        case ast::Operator::DIV:
            {
                if(type == INT || type == FLOAT){
                    auto right_value = moveToArgument(boost::get<ast::Value>(*operation.get<1>()), function);
                    auto temp = function.context->new_temporary(type);

                    if(type == FLOAT){
                        function.emplace_back(temp, left[0], mtac::toFloatOperator(op), right_value);
                    } else {
                        function.emplace_back(temp, left[0], mtac::toOperator(op), right_value);
                    }

                    left = {temp};
                } else {
                    eddic_unreachable("Unsupported operator for this type");
                }

                break;
            }
        
        case ast::Operator::EQUALS:
        case ast::Operator::NOT_EQUALS:
        case ast::Operator::LESS:
        case ast::Operator::LESS_EQUALS:
        case ast::Operator::GREATER:
        case ast::Operator::GREATER_EQUALS:
            {
                auto t1 = function.context->new_temporary(INT);
                auto right = moveToArgument(boost::get<ast::Value>(*operation.get<1>()), function);

                if(type == INT || type == CHAR || type->is_pointer() || type->is_dynamic_array()){
                    function.emplace_back(t1, left[0], mtac::toRelationalOperator(op), right);
                } else if(type == FLOAT){
                    function.emplace_back(t1, left[0], mtac::toFloatRelationalOperator(op), right);
                } else {
                    eddic_unreachable("Unsupported type in relational operator");
                }

                left = {t1};

                break;
            }

        case ast::Operator::AND:
            {
                auto t1 = function.context->new_temporary(INT); 

                auto falseLabel = newLabel();
                auto endLabel = newLabel();

                function.emplace_back(mtac::Operator::IF_FALSE_UNARY, left[0], falseLabel);

                jump_if_false(function, falseLabel, boost::get<ast::Value>(*operation.get<1>()));

                function.emplace_back(t1, 1, mtac::Operator::ASSIGN);
                function.emplace_back(endLabel, mtac::Operator::GOTO);

                function.emplace_back(falseLabel, mtac::Operator::LABEL);
                function.emplace_back(t1, 0, mtac::Operator::ASSIGN);

                function.emplace_back(endLabel, mtac::Operator::LABEL);

                left = {t1};

                break;
            }

        case ast::Operator::OR:
            {
                auto t1 = function.context->new_temporary(INT); 

                auto trueLabel = newLabel();
                auto endLabel = newLabel();

                function.emplace_back(mtac::Operator::IF_UNARY, left[0], trueLabel);

                jump_if_true(function, trueLabel, boost::get<ast::Value>(*operation.get<1>()));

                function.emplace_back(t1, 0, mtac::Operator::ASSIGN);
                function.emplace_back(endLabel, mtac::Operator::GOTO);

                function.emplace_back(trueLabel, mtac::Operator::LABEL);
                function.emplace_back(t1, 1, mtac::Operator::ASSIGN);

                function.emplace_back(endLabel, mtac::Operator::LABEL);

                left = {t1};

                break;
            }

        case ast::Operator::BRACKET:
            {
                auto index_value = boost::get<ast::Value>(*operation_value);

                if(type == STRING){
                    assert(left.size()  == 1 || left.size() == 2);

                    auto index = moveToArgument(index_value, function);
                    auto pointer_temp = function.context->new_temporary(INT);
                    auto t1 = function.context->new_temporary(INT);

                    //Get the label
                    function.emplace_back(pointer_temp, left[0], mtac::Operator::ASSIGN);

                    //Get the specified char 
                    mtac::Quadruple quadruple(t1, pointer_temp, mtac::Operator::DOT, index);
                    quadruple.size = mtac::Size::BYTE;
                    function.emplace_back(std::move(quadruple));

                    left = {t1};
                } else {
                    assert(left.size() == 1);

                    auto index = index_of_array(boost::get<std::shared_ptr<Variable>>(left[0]), index_value, function); 
                    auto data_type = type->data_type();
                    
                    if(T == ArgumentType::ADDRESS){
                        auto temp = function.context->new_temporary(data_type->is_pointer() ? data_type : new_pointer_type(data_type));

                        function.emplace_back(temp, left[0], mtac::Operator::PDOT, index);

                        left = {temp};
                    } else {
                        if(data_type == BOOL || data_type == CHAR || data_type == INT || data_type == FLOAT || data_type->is_pointer()){
                            std::shared_ptr<Variable> temp;
                            if(T == ArgumentType::REFERENCE){
                                temp = function.context->new_reference(data_type, boost::get<std::shared_ptr<Variable>>(left[0]), variant_cast(index));
                            } else {
                                temp = function.context->new_temporary(data_type);
                            }

                            function.emplace_back(temp, left[0], mtac::Operator::DOT, index);
                            left = {temp};
                        } else if (data_type == STRING){
                            auto t1 = function.context->new_temporary(INT);
                            function.emplace_back(t1, left[0], mtac::Operator::DOT, index);

                            auto t2 = function.context->new_temporary(INT);
                            auto t3 = function.context->new_temporary(INT);

                            //Assign the second part of the string
                            function.emplace_back(t3, index, mtac::Operator::ADD, INT->size(function.context->global()->target_platform()));
                            function.emplace_back(t2, left[0], mtac::Operator::DOT, t3);

                            left = {t1, t2};
                        } else {
                            //A reference to a structure is a special case
                            //Does not get any value from the array
                            if(T == ArgumentType::REFERENCE){
                                left = {function.context->new_reference(data_type, boost::get<std::shared_ptr<Variable>>(left[0]), variant_cast(index))};
                            } else {
                                eddic_unreachable("Type not handled by BRACKET");
                            }
                        }
                    }
                }

                break;
            }

        case ast::Operator::DOT:
            {
                assert(left.size() == 1);
                auto variable = boost::get<std::shared_ptr<Variable>>(left[0]);
                auto member = boost::get<std::string>(*operation_value);

                std::shared_ptr<const Type> member_type;
                unsigned int offset = 0;
                boost::tie(offset, member_type) = mtac::compute_member(function.context->global(), variable->type(), member);

                if(T == ArgumentType::ADDRESS){
                    auto temp = function.context->new_temporary(member_type->is_pointer() ? member_type : new_pointer_type(member_type));

                    function.emplace_back(temp, variable, mtac::Operator::PDOT, offset);

                    left = {temp};
                } else {
                    left = get_member<T>(function, offset, member_type, variable);
                }

                break;
            }

        case ast::Operator::CALL:
            {
                auto global_context = function.context->global();
                auto call_operation_value = boost::get<ast::CallOperationValue>(*operation_value);
                auto& definition = global_context->getFunction(call_operation_value.mangled_name);

                auto left_value = left[0];

                if(call_operation_value.left_type){
                    auto dest_type = call_operation_value.left_type;
                    dest_type = dest_type->is_pointer() ? dest_type->data_type() : dest_type;

                    auto src_struct_type = global_context->get_struct(type);
                    auto dest_struct_type = global_context->get_struct(dest_type);

                    auto parent = src_struct_type->parent_type;
                    int offset = global_context->self_size_of_struct(src_struct_type);

                    while(parent){
                        if(parent == dest_type){
                            break;
                        }
                        
                        auto struct_type = global_context->get_struct(parent);
                        parent = struct_type->parent_type;
                        
                        offset += global_context->self_size_of_struct(struct_type);
                    }
                    
                    auto t1 = function.context->new_temporary(type->is_pointer() ? type : new_pointer_type(type));

                    if(type->is_pointer()){
                        function.emplace_back(t1, left_value, mtac::Operator::ADD, offset);
                    } else {
                        function.emplace_back(t1, left_value, mtac::Operator::PASSIGN);
                        function.emplace_back(t1, t1, mtac::Operator::ADD, offset);
                    }

                    left_value = t1;
                }

                auto type = definition.return_type();

                if(type->is_custom_type() || type->is_template_type()){
                    auto var = function.context->generate_variable("ret_t_", type);

                    //Initialize the temporary
                    construct(function, type, {}, var);
                    
                    //Pass the address of return
                    function.emplace_back(mtac::Operator::PPARAM, var, definition.context()->getVariable("__ret"), definition);

                    //Pass the normal arguments of the function
                    pass_arguments(function, definition, call_operation_value.values);
                    
                    //Pass the address of the object to the member function
                    function.emplace_back(mtac::Operator::PPARAM, left_value, definition.context()->getVariable(definition.parameter(0).name()), definition);

                    function.emplace_back(mtac::Operator::CALL, definition);

                    left = {var};
                    break;
                }

                std::shared_ptr<Variable> return_;
                std::shared_ptr<Variable> return2_;

                if(type == BOOL || type == CHAR || type == INT || type == FLOAT || type->is_pointer()){
                    return_ = function.context->new_temporary(type);

                    left = {return_};
                } else if(type == STRING){
                    return_ = function.context->new_temporary(INT);
                    return2_ = function.context->new_temporary(INT);

                    left = {return_, return2_};
                } else if(type == VOID){
                    //It is void, does not return anything
                    left = {};
                } else {
                    eddic_unreachable("Unhandled return type");
                }

                //Pass all normal arguments
                pass_arguments(function, definition, call_operation_value.values);

                //Pass the address of the object to the member function
                function.emplace_back(mtac::Operator::PPARAM, left_value, definition.context()->getVariable(definition.parameter(0).name()), definition);

                //Call the function
                function.emplace_back(mtac::Operator::CALL, definition, return_, return2_);

                break;
            }

        case ast::Operator::INC:
            {
                eddic_assert(mtac::isVariable(left[0]), "The visitor should return a variable");

                auto t1 = boost::get<std::shared_ptr<Variable>>(left[0]);
                auto t2 = function.context->new_temporary(type);

                if(type == FLOAT){
                    function.emplace_back(t2, t1, mtac::Operator::FASSIGN);
                    function.emplace_back(t1, t1, mtac::Operator::FADD, 1.0);
                } else if (type == INT){
                    function.emplace_back(t2, t1, mtac::Operator::ASSIGN);
                    function.emplace_back(t1, t1, mtac::Operator::ADD, 1);
                } else {
                    eddic_unreachable("invalid type");
                }

                left = {t2};

                break;
            }

        case ast::Operator::DEC:
            {
                eddic_assert(mtac::isVariable(left[0]), "The visitor should return a variable");

                auto t1 = boost::get<std::shared_ptr<Variable>>(left[0]);
                auto t2 = function.context->new_temporary(type);

                if(type == FLOAT){
                    function.emplace_back(t2, t1, mtac::Operator::FASSIGN);
                    function.emplace_back(t1, t1, mtac::Operator::FSUB, 1.0);
                } else if (type == INT){
                    function.emplace_back(t2, t1, mtac::Operator::ASSIGN);
                    function.emplace_back(t1, t1, mtac::Operator::SUB, 1);
                } else {
                    eddic_unreachable("invalid type");
                }

                left = {t2};

                break;
            }

        default:
            eddic_unreachable("Invalid operator");
    }

    //The computed values by this operation
    return left;
}

//Indicate if a postfix operator needs a pointer as left value
bool need_pointer(ast::Operator op, std::shared_ptr<const Type> left_type){
    return (op == ast::Operator::CALL && !left_type->is_pointer());
}

//Indicate if a postfix operator needs a reference as left value
bool need_reference(ast::Operator op, std::shared_ptr<const Type> left_type){
    return 
            op == ast::Operator::INC || op == ast::Operator::DEC    //Modifies the left value, needs a reference
        ||  op == ast::Operator::BRACKET                            //Needs a reference to the left array
        ||  (op == ast::Operator::DOT && !left_type->is_pointer()); //If it is not a pointer, needs a reference to the struct variable
}

//Visitor used to transform a right value into a set of MTAC arguments

template<ArgumentType T = ArgumentType::NORMAL>
struct ToArgumentsVisitor : public boost::static_visitor<arguments> {
    ToArgumentsVisitor(mtac::Function& f) : function(f) {}
    
    mtac::Function& function;

    result_type operator()(ast::Literal& literal) const {
        return {literal.label, (int) literal.value.size() - 2};
    }

    result_type operator()(ast::CharLiteral& literal) const {
        char v = literal.value[1];

        return {static_cast<int>(v)};
    }

    result_type operator()(ast::Integer& integer) const {
        return {integer.value};
    }
    
    result_type operator()(ast::IntegerSuffix& integer) const {
        return {(double) integer.value};
    }
    
    result_type operator()(ast::Float& float_) const {
        return {float_.value};
    }
    
    result_type operator()(ast::False&) const {
        return {0};
    }
    
    result_type operator()(ast::True&) const {
        return {1};
    }
    
    result_type operator()(ast::Null&) const {
        return {0};
    }

    result_type operator()(ast::New& new_) const {
        auto type = visit(ast::TypeTransformer(function.context->global()), new_.Content->type);
    
        function.emplace_back(mtac::Operator::PARAM, type->size(function.context->global()->target_platform()), "a", function.context->global()->getFunction("_F5allocI"));

        auto t1 = function.context->new_temporary(new_pointer_type(INT));

        function.context->global()->addReference("_F5allocI");
        function.emplace_back(mtac::Operator::CALL, function.context->global()->getFunction("_F5allocI"), t1); 
            
        //If structure type, call the constructor
        if(type->is_custom_type() || type->is_template_type()){
            construct(function, type, new_.Content->values, t1);
        }

        return {t1};
    }
    
    result_type operator()(ast::NewArray& new_) const {
        auto type = visit_non_variant(ast::GetTypeVisitor(), new_);

        auto size = function.context->new_temporary(INT);
        auto size_temp = visit(ToArgumentsVisitor<>(function), new_.Content->size)[0];
        
        auto platform = function.context->global()->target_platform();

        function.emplace_back(size, size_temp, mtac::Operator::MUL, static_cast<int>(type->data_type()->size(platform)));
        function.emplace_back(size, size, mtac::Operator::ADD, static_cast<int>(INT->size(platform)));
    
        function.emplace_back(mtac::Operator::PARAM, size, "a", function.context->global()->getFunction("_F5allocI"));

        auto t1 = function.context->new_temporary(new_pointer_type(INT));

        function.context->global()->addReference("_F5allocI");
        function.emplace_back(mtac::Operator::CALL, function.context->global()->getFunction("_F5allocI"), t1); 
        
        function.emplace_back(t1, 0, mtac::Operator::DOT_ASSIGN, size_temp);

        return {t1};
    }

    result_type operator()(ast::BuiltinOperator& builtin) const {
        switch(builtin.Content->type){
            case ast::BuiltinType::SIZE:
                {
                    //Get a reference to the array
                    arguments right = visit(ToArgumentsVisitor<ArgumentType::REFERENCE>(function), builtin.Content->values.at(0));

                    eddic_assert(mtac::isVariable(right[0]), "The visitor should return a variable");

                    auto variable = boost::get<std::shared_ptr<Variable>>(right[0]);

                    if(variable->position().isGlobal()){
                        return {variable->type()->elements()};
                    } else if((variable->position().is_variable() || variable->position().isStack()) && variable->type()->has_elements()){
                        return {variable->type()->elements()};
                    } else if(variable->type()->is_dynamic_array() || variable->is_reference() || variable->position().isParameter() || variable->position().isStack() || variable->position().is_variable()){
                        auto t1 = function.context->new_temporary(INT);

                        //The size of the array is at the address pointed by the variable
                        function.emplace_back(t1, variable, mtac::Operator::DOT, 0);

                        return {t1};
                    }

                    eddic_unreachable("The variable is not of a valid type");
                }
            
            case ast::BuiltinType::LENGTH:
                {
                    auto& value = builtin.Content->values[0];

                    return {visit(*this, value)[1]};
                }
        }

        eddic_unreachable("This builtin operator is not handled");
    }

    result_type operator()(ast::FunctionCall& call) const {
        auto& definition = call.Content->context->global()->getFunction(call.Content->mangled_name);
        auto type = definition.return_type();

        if(type == VOID){
            pass_arguments(function, definition, call.Content->values);

            function.emplace_back(mtac::Operator::CALL, definition, nullptr, nullptr);

            return {};
        } else if(type == BOOL || type == CHAR || type == INT || type == FLOAT || type->is_pointer()){
            auto t1 = function.context->new_temporary(type);

            pass_arguments(function, definition, call.Content->values);

            function.emplace_back(mtac::Operator::CALL, definition, t1, nullptr);

            return {t1};
        } else if(type == STRING){
            auto t1 = function.context->new_temporary(INT);
            auto t2 = function.context->new_temporary(INT);

            pass_arguments(function, definition, call.Content->values);

            function.emplace_back(mtac::Operator::CALL, definition, t1, t2);

            return {t1, t2};
        } else if(type->is_custom_type() || type->is_template_type()){
            auto var = function.context->generate_variable("ret_t_", type);

            //Initialize the temporary
            construct(function, type, {}, var);
            
            //Pass the address of return
            function.emplace_back(mtac::Operator::PPARAM, var, definition.context()->getVariable("__ret"), definition);
    
            //Pass the normal arguments of the function
            pass_arguments(function, definition, call.Content->values);

            function.emplace_back(mtac::Operator::CALL, definition, nullptr, nullptr);
            
            return {var};
        }
        
        eddic_unreachable("Unhandled function return type");
    }

    result_type operator()(ast::Assignment& assignment) const {
        eddic_assert(assignment.Content->op == ast::Operator::ASSIGN, "Compound assignment should be transformed into Assignment");

        assign(function, assignment.Content->left_value, assignment.Content->value);

        return visit(*this, assignment.Content->left_value);
    }

    result_type operator()(ast::Ternary& ternary) const {
        return compile_ternary(function, ternary);
    }

    result_type operator()(std::shared_ptr<Variable> var) const {
        return {var};
    }

    result_type operator()(ast::VariableValue& value) const {
        if(T == ArgumentType::ADDRESS || T == ArgumentType::REFERENCE){
            return {value.Content->var};
        }

        auto type = value.Content->var->type();

        //If it's a const, we just have to replace it by its constant value
        if(type->is_const()){
            auto val = value.Content->var->val();
            auto nc_type = type;//->non_const();

            if(nc_type == INT || nc_type == BOOL){
                return {boost::get<int>(val)};
            } else if(nc_type == CHAR){
                return {boost::get<char>(val)};        
            } else if(nc_type == FLOAT){
                return {boost::get<double>(val)};        
            } else if(nc_type == STRING){
                auto value = boost::get<std::pair<std::string, int>>(val);

                return {value.first, value.second};
            } 

            eddic_unreachable("void is not a type");
        } else if(type->is_array() || type->is_pointer()){
            return {value.Content->var};
        } else  if(type == INT || type == CHAR || type == BOOL || type == FLOAT){
            return {value.Content->var};
        } else if(type == STRING){
            auto temp = value.Content->context->new_temporary(INT);
            function.emplace_back(temp, value.Content->var, mtac::Operator::DOT, INT->size(function.context->global()->target_platform()));

            return {value.Content->var, temp};
        } else if(type->is_structure()) {
            return struct_to_arguments(function, type, value.Content->var, 0);
        } 
    
        eddic_unreachable("Unhandled type");
    }

    result_type dereference_variable(std::shared_ptr<Variable> variable, std::shared_ptr<const Type> type) const {
        if(type == INT || type == CHAR || type == BOOL){
            auto temp = function.context->new_temporary(type);

            function.emplace_back(temp, variable, mtac::Operator::DOT, 0);

            return {temp};
        } else if(type == FLOAT){
            auto temp = function.context->new_temporary(type);

            function.emplace_back(temp, variable, mtac::Operator::FDOT, 0);

            return {temp};
        } else if(type == STRING){
            auto t1 = function.context->new_temporary(INT);
            auto t2 = function.context->new_temporary(INT);

            function.emplace_back(t1, variable, mtac::Operator::DOT, 0);
            function.emplace_back(t2, variable, mtac::Operator::DOT, INT->size(function.context->global()->target_platform()));

            return {t1, t2};
        } else if(type->is_structure()){
            return struct_to_arguments(function, type, variable, 0);         
        }

        eddic_unreachable("Unhandled type");
    }

    result_type operator()(ast::PrefixOperation& operation) const {
        auto op = operation.Content->op;
        auto type = visit(ast::GetTypeVisitor(), operation.Content->left_value);
            
        switch(op){
            case ast::Operator::STAR:
            {
                auto left = visit(ToArgumentsVisitor<>(function), operation.Content->left_value);

                eddic_assert(left.size() == 1, "STAR only support one value");
                eddic_assert(mtac::isVariable(left[0]), "The visitor should return a temporary variable");

                auto variable = boost::get<std::shared_ptr<Variable>>(left[0]);
                return dereference_variable(variable, type->data_type());
            }
            
            case ast::Operator::ADDRESS:
            {
                auto left = visit(ToArgumentsVisitor<ArgumentType::ADDRESS>(function), operation.Content->left_value);

                eddic_assert(left.size() == 1, "ADDRESS only support one value");

                return left;
            }

            case ast::Operator::NOT:
            {
                auto left = visit(ToArgumentsVisitor<>(function), operation.Content->left_value);

                eddic_assert(left.size() == 1, "NOT only support one value");

                auto t1 = function.context->new_temporary(BOOL);

                function.emplace_back(t1, left[0], mtac::Operator::NOT);

                return {t1};
            }

            //The + unary operator has no effect
            case ast::Operator::ADD:
                return visit(ToArgumentsVisitor<>(function), operation.Content->left_value);

            case ast::Operator::SUB:
            {
                auto left = visit(ToArgumentsVisitor<>(function), operation.Content->left_value);

                eddic_assert(left.size() == 1, "SUB only support one value");

                auto t1 = function.context->new_temporary(type);

                if(type == FLOAT){
                    function.emplace_back(t1, left[0], mtac::Operator::FMINUS);
                } else if(type == INT){
                    function.emplace_back(t1, left[0], mtac::Operator::MINUS);
                } else {
                    eddic_unreachable("Invalid type");
                }

                return {t1};
            }
            
            case ast::Operator::INC:
            {
                //INC needs a reference to the left value
                auto left = visit(ToArgumentsVisitor<ArgumentType::REFERENCE>(function), operation.Content->left_value);

                eddic_assert(mtac::isVariable(left[0]), "The visitor should return a variable");

                auto t1 = boost::get<std::shared_ptr<Variable>>(left[0]);

                if(type == FLOAT){
                    function.emplace_back(t1, t1, mtac::Operator::FADD, 1.0);
                } else if (type == INT){
                    function.emplace_back(t1, t1, mtac::Operator::ADD, 1);
                } else {
                    eddic_unreachable("Invalid type");
                }

                return {t1};
            }
            
            case ast::Operator::DEC:
            {
                //DEC needs a reference to the left value
                auto left = visit(ToArgumentsVisitor<ArgumentType::REFERENCE>(function), operation.Content->left_value);

                eddic_assert(mtac::isVariable(left[0]), "The visitor should return a variable");

                auto t1 = boost::get<std::shared_ptr<Variable>>(left[0]);

                if(type == FLOAT){
                    function.emplace_back(t1, t1, mtac::Operator::FSUB, 1.0);
                } else if (type == INT){
                    function.emplace_back(t1, t1, mtac::Operator::SUB, 1);
                } else {
                    eddic_unreachable("Invalid type");
                }

                return {t1};
            }

            default:
                eddic_unreachable("Unsupported operator");    
        }
    }

    result_type operator()(ast::Expression& value) const {
        auto type = visit(ast::GetTypeVisitor(), value.Content->first);
        
        arguments left;
        if(need_reference(value.Content->operations[0].get<0>(), type)){
            left = visit(ToArgumentsVisitor<ArgumentType::REFERENCE>(function), value.Content->first);
        } else if(need_pointer(value.Content->operations[0].get<0>(), type)){
            left = visit(ToArgumentsVisitor<ArgumentType::ADDRESS>(function), value.Content->first);
        } else {
            left = visit(*this, value.Content->first);
        }

        //Compute each operation
        for(std::size_t i = 0; i < value.Content->operations.size(); ++i){
            auto& operation = value.Content->operations[i];

            //Get the type computed by the current operation for the next one
            auto future_type = ast::operation_type(type, value.Content->context, operation);

            //Execute the current operation
            if(i < value.Content->operations.size() - 1 && need_reference(value.Content->operations[i + 1].get<0>(), future_type)){
                left = compute_expression_operation<ArgumentType::REFERENCE>(function, type, left, operation);
            } else if(i < value.Content->operations.size() - 1 && need_pointer(value.Content->operations[i + 1].get<0>(), future_type)){
                left = compute_expression_operation<ArgumentType::ADDRESS>(function, type, left, operation);
            } else {
                left = compute_expression_operation<T>(function, type, left, operation);
            }

            type = future_type;
        }

        //Return the value returned by the last operation
        return left;
    }

    result_type operator()(ast::Cast& cast) const {
        mtac::Argument arg = moveToArgument(cast.Content->value, function);
        
        auto dest_type = visit_non_variant(ast::GetTypeVisitor(), cast);
        auto src_type = visit(ast::GetTypeVisitor(), cast.Content->value);

        if(src_type != dest_type){
            auto t1 = function.context->new_temporary(dest_type);

            if(dest_type == FLOAT){
                function.emplace_back(t1, arg, mtac::Operator::I2F);
            } else if(dest_type == INT){
                if(src_type == FLOAT){
                    function.emplace_back(t1, arg, mtac::Operator::F2I);
                } else if(src_type == CHAR){
                    function.emplace_back(t1, arg, mtac::Operator::ASSIGN);
                }
            } else if(dest_type == CHAR){
                function.emplace_back(t1, arg, mtac::Operator::ASSIGN);
            } else if(dest_type->is_pointer() && src_type->is_pointer()){
                if(dest_type->data_type()->is_structure() && src_type->data_type()->is_structure()){
                    auto global_context = function.context->global();

                    auto src_struct_type = global_context->get_struct(src_type);
                    auto dest_struct_type = global_context->get_struct(dest_type);

                    bool is_parent = false;
                    auto parent = src_struct_type->parent_type;
                    int offset = global_context->self_size_of_struct(src_struct_type);

                    while(parent){
                        if(parent == dest_type->data_type()){
                            is_parent = true;
                            break;
                        }
                        
                        auto struct_type = global_context->get_struct(parent);
                        parent = struct_type->parent_type;
                        
                        offset += global_context->self_size_of_struct(struct_type);
                    }

                    if(is_parent){
                        function.emplace_back(t1, arg, mtac::Operator::ADD, offset);

                        return {t1};
                    }
                }

                //At this point, the cast has no effect
                return {arg};
            } else {
                return {arg};
            }
            
            return {t1};
        }

        //If src_type == dest_type, there is nothing to do
        return {arg};
    }
};

//Visitor used to assign a right value to a left value

struct AssignmentVisitor : public boost::static_visitor<> {
    mtac::Function& function;
    ast::Value& right_value;

    AssignmentVisitor(mtac::Function& function, ast::Value& right_value) : function(function), right_value(right_value) {}

    //Assignment of the form A = X

    void operator()(ast::VariableValue& variable_value){
        auto platform = function.context->global()->target_platform();

        auto variable = variable_value.Content->var;
        auto type = visit(ast::GetTypeVisitor(), right_value); 

        if(type->is_pointer() || (variable->type()->is_array() && variable->type()->data_type()->is_pointer())){
            auto values = visit(ToArgumentsVisitor<>(function), right_value);
            function.emplace_back(variable, values[0], mtac::Operator::PASSIGN);
        } else if(type->is_array() || type == INT || type == CHAR || type == BOOL){
            auto values = visit(ToArgumentsVisitor<>(function), right_value);
            function.emplace_back(variable, values[0], mtac::Operator::ASSIGN);
        } else if(type == STRING){
            auto values = visit(ToArgumentsVisitor<>(function), right_value);
            function.emplace_back(variable, values[0], mtac::Operator::ASSIGN);
            function.emplace_back(variable, INT->size(platform), mtac::Operator::DOT_ASSIGN, values[1]);
        } else if(type == FLOAT){
            auto values = visit(ToArgumentsVisitor<>(function), right_value);
            function.emplace_back(variable, values[0], mtac::Operator::FASSIGN);
        } else if(type->is_custom_type() || type->is_template_type()){
            copy_construct(function, variable->type(), variable, right_value);
        } else {
            eddic_unreachable("Unhandled value type");
        }
    }

    //Assignment of the form A[i] = X or A.i = X

    void operator()(ast::Expression& value){
        auto type = visit(ast::GetTypeVisitor(), value.Content->first);

        arguments left;
        if(need_reference(value.Content->operations[0].get<0>(), type)){
            left = visit(ToArgumentsVisitor<ArgumentType::REFERENCE>(function), value.Content->first);
        } else if(need_pointer(value.Content->operations[0].get<0>(), type)){
            left = visit(ToArgumentsVisitor<ArgumentType::ADDRESS>(function), value.Content->first);
        } else {
            left = visit(ToArgumentsVisitor<>(function), value.Content->first);
        }

        auto platform = function.context->global()->target_platform();

        //Compute each operation but the last
        for(std::size_t i = 0; i < value.Content->operations.size() - 1; ++i){
            auto& operation = value.Content->operations[i];

            //Get the type computed by the current operation for the next one
            auto future_type = ast::operation_type(type, value.Content->context, operation);

            //Execute the current operation
            if(i < value.Content->operations.size() - 1 && need_reference(value.Content->operations[i + 1].get<0>(), future_type)){
                left = compute_expression_operation<ArgumentType::REFERENCE>(function, type, left, operation);
            } else if(i < value.Content->operations.size() - 1 && need_pointer(value.Content->operations[i + 1].get<0>(), future_type)){
                left = compute_expression_operation<ArgumentType::ADDRESS>(function, type, left, operation);
            } else {
                left = compute_expression_operation<>(function, type, left, operation);
            }
            
            type = future_type;
        }

        //Assign the right value to the left value generated

        auto& last_operation = value.Content->operations.back();

        //Assign to an element of an array
        if(last_operation.get<0>() == ast::Operator::BRACKET){
            assert(mtac::isVariable(left[0]));
            auto array_variable = boost::get<std::shared_ptr<Variable>>(left[0]);

            auto& index_value = boost::get<ast::Value>(*last_operation.get<1>());
            auto index = index_of_array(array_variable, index_value, function); 
        
            auto left_type = array_variable->type()->data_type();
            auto right_type = visit(ast::GetTypeVisitor(), right_value); 
            
            arguments values;
            if(left_type->is_pointer()){
                values = visit(ToArgumentsVisitor<ArgumentType::ADDRESS>(function), right_value);
            } else {
                values = visit(ToArgumentsVisitor<>(function), right_value);
            }

            if(left_type->is_pointer()){
                function.emplace_back(array_variable, index, mtac::Operator::DOT_PASSIGN, values[0]);
            } else if(right_type->is_array() || right_type == INT || right_type == CHAR || right_type == BOOL){
                function.emplace_back(array_variable, index, mtac::Operator::DOT_ASSIGN, values[0]);
            } else if(right_type == STRING){
                function.emplace_back(array_variable, index, mtac::Operator::DOT_ASSIGN, values[0]);

                auto temp1 = function.context->new_temporary(INT);
                function.emplace_back(temp1, index, mtac::Operator::ADD, INT->size(platform));
                function.emplace_back(array_variable, temp1, mtac::Operator::DOT_ASSIGN, values[1]);
            } else if(right_type == FLOAT){
                function.emplace_back(array_variable, index, mtac::Operator::DOT_FASSIGN, values[0]);
            } else {
                eddic_unreachable("Unhandled value type");
            }
        } 
        //Assign to a member of a structure
        else if(last_operation.get<0>() == ast::Operator::DOT){
            assert(mtac::isVariable(left[0]));
            auto struct_variable = boost::get<std::shared_ptr<Variable>>(left[0]);

            auto& member = boost::get<std::string>(*last_operation.get<1>());

            unsigned int offset = 0;
            std::shared_ptr<const Type> member_type;
            boost::tie(offset, member_type) = mtac::compute_member(function.context->global(), struct_variable->type(), member);
            
            arguments values;
            if(member_type->is_pointer()){
                values = visit(ToArgumentsVisitor<ArgumentType::ADDRESS>(function), right_value);
            } else {
                values = visit(ToArgumentsVisitor<>(function), right_value);
            }
            
            if(member_type->is_pointer()){
                function.emplace_back(struct_variable, offset, mtac::Operator::DOT_PASSIGN, values[0]);
            } else if(member_type->is_array() || member_type == INT || member_type == CHAR || member_type == BOOL){
                function.emplace_back(struct_variable, offset, mtac::Operator::DOT_ASSIGN, values[0]);
            } else if(member_type == STRING){
                function.emplace_back(struct_variable, offset, mtac::Operator::DOT_ASSIGN, values[0]);
                function.emplace_back(struct_variable, offset + INT->size(platform), mtac::Operator::DOT_ASSIGN, values[1]);
            } else if(member_type == FLOAT){
                function.emplace_back(struct_variable, offset, mtac::Operator::DOT_FASSIGN, values[0]);
            } else {
                eddic_unreachable("Unhandled value type");
            }
        } else {
            eddic_unreachable("This postfix operator does not result in a left value");
        }
    }

    //Assignment of the form *A = X

    void operator()(ast::PrefixOperation& dereference_value){
        auto platform = function.context->global()->target_platform();

        if(dereference_value.Content->op == ast::Operator::STAR){
            auto left = visit(ToArgumentsVisitor<>(function), dereference_value.Content->left_value);
            assert(mtac::isVariable(left[0]));
            auto pointer_variable = boost::get<std::shared_ptr<Variable>>(left[0]);
        
            auto values = visit(ToArgumentsVisitor<>(function), right_value);
            auto right_type = visit(ast::GetTypeVisitor(), right_value); 

            if(right_type->is_pointer()){
                function.emplace_back(pointer_variable, 0, mtac::Operator::DOT_PASSIGN, values[0]);
            } else if(right_type->is_array() || right_type == INT || right_type == CHAR || right_type == BOOL){
                function.emplace_back(pointer_variable, 0, mtac::Operator::DOT_ASSIGN, values[0]);
            } else if(right_type == STRING){
                function.emplace_back(pointer_variable, 0, mtac::Operator::DOT_ASSIGN, values[0]);
                function.emplace_back(pointer_variable, INT->size(platform), mtac::Operator::DOT_ASSIGN, values[1]);
            } else if(right_type == FLOAT){
                function.emplace_back(pointer_variable, 0, mtac::Operator::DOT_FASSIGN, values[0]);
            } else {
                eddic_unreachable("Unhandled variable type");
            }
        } 
        //The others prefix operators does not yield left values
        else {
            eddic_unreachable("This prefix operator does not result in a left value");
        }
    }

    //Other ast::Value type does not yield a left value
    
    template<typename T>
    void operator()(T&){
        eddic_unreachable("Not a left value");
    }
};

void assign(mtac::Function& function, ast::Value& left_value, ast::Value& value){
    AssignmentVisitor visitor(function, value);
    visit(visitor, left_value);
}

void assign(mtac::Function& function, std::shared_ptr<Variable> variable, ast::Value& value){
    ast::VariableValue left_value;
    left_value.Content->var = variable;
    left_value.Content->variableName = variable->name();
    left_value.Content->context = function.context;

    AssignmentVisitor visitor(function, value);
    visit_non_variant(visitor, left_value);
}

arguments compile_ternary(mtac::Function& function, ast::Ternary& ternary){
    auto type = visit_non_variant(ast::GetTypeVisitor(), ternary);

    auto falseLabel = newLabel();
    auto endLabel = newLabel();

    if(type == INT || type == CHAR || type == BOOL || type == FLOAT){
        auto t1 = function.context->new_temporary(type);

        jump_if_false(function, falseLabel, ternary.Content->condition); 
        assign(function, t1, ternary.Content->true_value);
        function.emplace_back(endLabel, mtac::Operator::GOTO);
        
        function.emplace_back(falseLabel, mtac::Operator::LABEL);
        assign(function, t1, ternary.Content->false_value);
        
        function.emplace_back(endLabel, mtac::Operator::LABEL);

        return {t1};
    } else if(type == STRING){
        auto t1 = function.context->new_temporary(INT);
        auto t2 = function.context->new_temporary(INT);
        
        jump_if_false(function, falseLabel, ternary.Content->condition); 
        auto args = visit(ToArgumentsVisitor<>(function), ternary.Content->true_value);
        function.emplace_back(t1, args[0], mtac::Operator::ASSIGN);
        function.emplace_back(t2, args[1], mtac::Operator::ASSIGN);

        function.emplace_back(endLabel, mtac::Operator::GOTO);
        
        function.emplace_back(falseLabel, mtac::Operator::LABEL);
        args = visit(ToArgumentsVisitor<>(function), ternary.Content->false_value);
        function.emplace_back(t1, args[0], mtac::Operator::ASSIGN);  
        function.emplace_back(t2, args[1], mtac::Operator::ASSIGN);  
        
        function.emplace_back(endLabel, mtac::Operator::LABEL);
        
        return {t1, t2};
    }

    eddic_unreachable("Unhandled ternary type");
}

//Visitor used to compile each source instructions

class FunctionCompiler : public boost::static_visitor<> {
    private:
        mtac::Program& program;
        mtac::Function& function;
    
    public:
        FunctionCompiler(mtac::Program& program, mtac::Function& function) : program(program), function(function)  {}

        //No code is generated for these nodes
        AUTO_IGNORE_ARRAY_DECLARATION()
        AUTO_IGNORE_MEMBER_DECLARATION();

        void issue_destructors(std::shared_ptr<Context> context){
            for(auto& pair : *context){
                auto var = pair.second;

                if(var->position().isStack() || var->position().is_variable()){
                    auto type = var->type();

                    if(type->is_structure()){
                        destruct(function, type, var);
                    }
                }
            }
        }

        void operator()(ast::If& if_){
            if (if_.Content->elseIfs.empty()) {
                std::string endLabel = newLabel();

                jump_if_false(function, endLabel, if_.Content->condition);

                visit_each(*this, if_.Content->instructions);

                issue_destructors(if_.Content->context);

                if (if_.Content->else_) {
                    std::string elseLabel = newLabel();

                    function.emplace_back(elseLabel, mtac::Operator::GOTO);

                    function.emplace_back(endLabel, mtac::Operator::LABEL);

                    visit_each(*this, (*if_.Content->else_).instructions);
                    
                    issue_destructors((*if_.Content->else_).context);

                    function.emplace_back(elseLabel, mtac::Operator::LABEL);
                } else {
                    function.emplace_back(endLabel, mtac::Operator::LABEL);
                }
            } else {
                std::string end = newLabel();
                std::string next = newLabel();

                jump_if_false(function, next, if_.Content->condition);

                visit_each(*this, if_.Content->instructions);
                
                issue_destructors(if_.Content->context);

                function.emplace_back(end, mtac::Operator::GOTO);

                for (std::vector<ast::ElseIf>::size_type i = 0; i < if_.Content->elseIfs.size(); ++i) {
                    ast::ElseIf& elseIf = if_.Content->elseIfs[i];

                    function.emplace_back(next, mtac::Operator::LABEL);

                    //Last elseif
                    if (i == if_.Content->elseIfs.size() - 1) {
                        if (if_.Content->else_) {
                            next = newLabel();
                        } else {
                            next = end;
                        }
                    } else {
                        next = newLabel();
                    }

                    jump_if_false(function, next, elseIf.condition);

                    visit_each(*this, elseIf.instructions);
                    
                    issue_destructors(elseIf.context);

                    function.emplace_back(end, mtac::Operator::GOTO);
                }

                if (if_.Content->else_) {
                    function.emplace_back(next, mtac::Operator::LABEL);

                    visit_each(*this, (*if_.Content->else_).instructions);
                    
                    issue_destructors((*if_.Content->else_).context);
                }

                function.emplace_back(end, mtac::Operator::LABEL);
            }
        }

        void operator()(ast::StructDeclaration& declaration){
            auto var = declaration.Content->context->getVariable(declaration.Content->variableName);

            //Initialize the structure variable
            construct(function, var->type(), declaration.Content->values, var);
        }

        void operator()(ast::VariableDeclaration& declaration){
            auto var = declaration.Content->context->getVariable(declaration.Content->variableName);

            if(var->type()->is_custom_type() || var->type()->is_template_type()){
                if(declaration.Content->value){
                    copy_construct(function, var->type(), var, *declaration.Content->value);
                } else {
                    //If there is no value, it is a simple initialization
                    construct(function, var->type(), {}, var);
                }
            } else {
                if(declaration.Content->value){
                    if(!var->type()->is_const()){
                        assign(function, var, *declaration.Content->value);
                    }
                }
            }
        }

        void operator()(ast::Swap& swap){
            auto lhs_var = swap.Content->lhs_var;
            auto rhs_var = swap.Content->rhs_var;
            
            auto t1 = swap.Content->context->new_temporary(INT);

            if(lhs_var->type() == INT || lhs_var->type() == CHAR || lhs_var->type() == BOOL || lhs_var->type() == STRING){
                function.emplace_back(t1, rhs_var, mtac::Operator::ASSIGN);
                function.emplace_back(rhs_var, lhs_var, mtac::Operator::ASSIGN);
                function.emplace_back(lhs_var, t1, mtac::Operator::ASSIGN);
                
                if(lhs_var->type() == STRING){
                    auto t2 = swap.Content->context->new_temporary(INT);

                    //t1 = 4(b)
                    function.emplace_back(t1, rhs_var, mtac::Operator::DOT, INT->size(function.context->global()->target_platform()));
                    //t2 = 4(a)
                    function.emplace_back(t2, lhs_var, mtac::Operator::DOT, INT->size(function.context->global()->target_platform()));
                    //4(b) = t2
                    function.emplace_back(rhs_var, INT->size(function.context->global()->target_platform()), mtac::Operator::DOT_ASSIGN, t2);
                    //4(a) = t1
                    function.emplace_back(lhs_var, INT->size(function.context->global()->target_platform()), mtac::Operator::DOT_ASSIGN, t1);
                }
            } else {
                eddic_unreachable("Unhandled variable type");
            }
        }

        void operator()(ast::DoWhile& while_){
            std::string startLabel = newLabel();

            function.emplace_back(startLabel, mtac::Operator::LABEL);

            visit_each(*this, while_.Content->instructions);

            issue_destructors(while_.Content->context);

            jump_if_true(function, startLabel, while_.Content->condition);
        }

        void operator()(ast::Return& return_){
            auto& definition = return_.Content->context->global()->getFunction(return_.Content->mangled_name);
            auto return_type = definition.return_type();

            //If the function returns a struct by value, it does not really returns something
            if(return_type->is_custom_type() || return_type->is_template_type()){
                copy_construct(function, return_type, function.context->getVariable("__ret"), return_.Content->value);
            } else {
                auto arguments = visit(ToArgumentsVisitor<>(function), return_.Content->value);

                if(arguments.size() == 1){
                    function.emplace_back(mtac::Operator::RETURN, arguments[0]);
                } else if(arguments.size() == 2){
                    function.emplace_back(mtac::Operator::RETURN, arguments[0], arguments[1]);
                } else {
                    eddic_unreachable("Unhandled arguments size");
                }   
            }
        }

        void operator()(ast::Delete& delete_){
            auto arg = visit(ToArgumentsVisitor<ArgumentType::ADDRESS>(function), delete_.Content->value)[0];
            auto type = visit(ast::GetTypeVisitor(), delete_.Content->value);
            if(type->data_type()->is_structure()){
                destruct(function, type->data_type(), arg);
            }

            auto free_name = "_F4freePI";
            auto& free_function = program.context->getFunction(free_name);

            function.emplace_back(mtac::Operator::PARAM, arg, "a", free_function);

            program.context->addReference(free_name);
            function.emplace_back(mtac::Operator::CALL, free_function); 
        }
       
        void operator()(ast::Assignment& assignment){
            eddic_assert(assignment.Content->op == ast::Operator::ASSIGN, "Compound assignment should be transformed into Assignment");

            assign(function, assignment.Content->left_value, assignment.Content->value);
        }

        //For statements that are also values, it is enough to transform them to arguments
        //If they have side effects, it will be handled and the eventually useless generated
        //temporaries will be removed by optimizations
        
        void operator()(ast::Expression& expression){
            visit_non_variant(ToArgumentsVisitor<>(function), expression);
        }

        void operator()(ast::PrefixOperation& operation){
            visit_non_variant(ToArgumentsVisitor<>(function), operation);
        }
        
        void operator()(ast::FunctionCall& functionCall){
            visit_non_variant(ToArgumentsVisitor<>(function), functionCall);
        }

        //All the other statements should have been transformed during the AST passes

        template<typename T>
        void operator()(T&){
            eddic_unreachable("This element should have been transformed"); 
        }
};

arguments move_to_arguments(ast::Value value, mtac::Function& function){
    return visit(ToArgumentsVisitor<>(function), value);
}

mtac::Argument moveToArgument(ast::Value value, mtac::Function& function){
    return move_to_arguments(value, function)[0];
}

void pass_arguments(mtac::Function& function, eddic::Function& definition, std::vector<ast::Value>& values){
    //Fail quickly if no values to pass
    if(values.empty()){
        return;
    }

    auto context = definition.context();

    //If it's a standard function, there are no context
    if(!context){
        int i = definition.parameters().size()-1;

        for(auto& first : boost::adaptors::reverse(values)){
            auto param = definition.parameter(i--).name(); 
            
            auto args = visit(ToArgumentsVisitor<>(function), first);
            for(auto& arg : boost::adaptors::reverse(args)){
                function.emplace_back(mtac::Operator::PARAM, arg, param, definition);
            }
        }
    } else {
        int i = definition.parameters().size()-1;

        if(values.size() != definition.parameters().size()){
            i = values.size() - 1;
        }

        if(definition.parameter(0).name() == "this"){
            i++;
        }

        for(auto& first : boost::adaptors::reverse(values)){
            auto param = context->getVariable(definition.parameter(i--).name());

            arguments args;
            if(param->type()->is_pointer()){
                args = visit(ToArgumentsVisitor<ArgumentType::ADDRESS>(function), first);
            } else {
                args = visit(ToArgumentsVisitor<>(function), first);
            }
            
            for(auto& arg : boost::adaptors::reverse(args)){
                function.emplace_back(
                        param->type()->is_pointer() ? mtac::Operator::PPARAM : mtac::Operator::PARAM, 
                        arg, param, definition);
            }
        }
    }
}

} //end of anonymous namespace

void mtac::Compiler::compile(ast::SourceFile& source, std::shared_ptr<StringPool>, mtac::Program& program) const {
    PerfsTimer timer("MTAC Compilation");

    program.context = source.Content->context;

    for(auto& block : source.Content->blocks){
        if(auto* ptr = boost::get<ast::FunctionDeclaration>(&block)){
            if(ptr->Content->functionName == "main" || program.context->referenceCount(ptr->Content->mangledName) > 0){
                program.functions.emplace_back(ptr->Content->context, ptr->Content->mangledName, program.context->getFunction(ptr->Content->mangledName));
                auto& function = program.functions.back();

                FunctionCompiler compiler(program, function);

                visit_each(compiler, ptr->Content->instructions);
                compiler.issue_destructors(ptr->Content->context);
            }
        } else if(auto* ptr = boost::get<ast::Struct>(&block)){
            for(auto& struct_block : ptr->Content->blocks){
                if(auto* ptr = boost::get<ast::FunctionDeclaration>(&struct_block)){
                    if(program.context->referenceCount(ptr->Content->mangledName) > 0){
                        program.functions.emplace_back(ptr->Content->context, ptr->Content->mangledName, program.context->getFunction(ptr->Content->mangledName));
                        auto& function = program.functions.back();

                        FunctionCompiler compiler(program, function);

                        visit_each(compiler, ptr->Content->instructions);
                        compiler.issue_destructors(ptr->Content->context);
                    }
                } else if(auto* ptr = boost::get<ast::Constructor>(&struct_block)){
                    program.functions.emplace_back(ptr->Content->context, ptr->Content->mangledName, program.context->getFunction(ptr->Content->mangledName));
                    auto& function = program.functions.back();

                    FunctionCompiler compiler(program, function);

                    visit_each(compiler, ptr->Content->instructions);
                    compiler.issue_destructors(ptr->Content->context);
                } else if(auto* ptr = boost::get<ast::Destructor>(&struct_block)){
                    program.functions.emplace_back(ptr->Content->context, ptr->Content->mangledName, program.context->getFunction(ptr->Content->mangledName));
                    auto& function = program.functions.back();

                    FunctionCompiler compiler(program, function);

                    visit_each(compiler, ptr->Content->instructions);
                    compiler.issue_destructors(ptr->Content->context);
                }
            }
        }
    }
}
