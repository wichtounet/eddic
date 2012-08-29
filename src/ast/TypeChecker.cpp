//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>

#include "variant.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Options.hpp"
#include "VisitorUtils.hpp"
#include "Utils.hpp"
#include "Type.hpp"

#include "ast/TypeChecker.hpp"
#include "ast/SourceFile.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

namespace {

class CheckerVisitor : public boost::static_visitor<> {
    public:
        CheckerVisitor(std::shared_ptr<GlobalContext> context) : context(context) {}

        AUTO_RECURSE_PROGRAM()
        AUTO_RECURSE_UNMARKED_FUNCTION_DECLARATION()
        AUTO_RECURSE_UNMARKED_STRUCT()
        AUTO_RECURSE_CONSTRUCTOR()
        AUTO_RECURSE_DESTRUCTOR()
        AUTO_RECURSE_FUNCTION_CALLS()
        AUTO_RECURSE_MEMBER_FUNCTION_CALLS()
        AUTO_RECURSE_SIMPLE_LOOPS()
        AUTO_RECURSE_BRANCHES()
        AUTO_RECURSE_BINARY_CONDITION()
        AUTO_RECURSE_UNARY_VALUES()
        AUTO_RECURSE_DEFAULT_CASE()
        AUTO_RECURSE_STRUCT_DECLARATION()
        AUTO_RECURSE_MEMBER_VALUE()
            
        AUTO_IGNORE_TEMPLATE_FUNCTION_DECLARATION()
        AUTO_IGNORE_TEMPLATE_STRUCT()
        AUTO_IGNORE_ARRAY_DECLARATION()
        AUTO_IGNORE_FALSE()
        AUTO_IGNORE_TRUE()
        AUTO_IGNORE_NULL()
        AUTO_IGNORE_LITERAL()
        AUTO_IGNORE_CHAR_LITERAL()
        AUTO_IGNORE_FLOAT()
        AUTO_IGNORE_INTEGER()
        AUTO_IGNORE_IMPORT()
        AUTO_IGNORE_STANDARD_IMPORT()
        AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION()
        AUTO_IGNORE_VARIABLE_VALUE()
        AUTO_IGNORE_DEREFERENCE_VALUE()
        
        void operator()(ast::GlobalVariableDeclaration& declaration){
            auto type = visit(ast::TypeTransformer(context), declaration.Content->variableType);

            auto valueType = visit(ast::GetTypeVisitor(), *declaration.Content->value);
            if (valueType != type) {
                throw SemanticalException("Incompatible type for global variable " + declaration.Content->variableName, declaration.Content->position);
            }
        }
        
        void operator()(ast::Foreach& foreach){
            visit_each(*this, foreach.Content->instructions);
        }
        
        void operator()(ast::ForeachIn& foreach){
            auto var_type = foreach.Content->var->type();
            auto array_type = foreach.Content->arrayVar->type();

            if(var_type != array_type->data_type()){
                throw SemanticalException("Incompatible type in declaration of the foreach variable " + foreach.Content->variableName, foreach.Content->position);
            }

            visit_each(*this, foreach.Content->instructions);
        }

        void operator()(ast::Switch& switch_){
            visit(*this, switch_.Content->value);

            auto value_type = visit(ast::GetTypeVisitor(), switch_.Content->value);

            if(value_type != INT){
                throw SemanticalException("Switch can only work on int type", switch_.Content->position);
            }
            
            visit_each_non_variant(*this, switch_.Content->cases);
            visit_optional_non_variant(*this, switch_.Content->default_case);
        }
        
        void operator()(ast::SwitchCase& switch_){
            visit(*this, switch_.value);

            auto value_type = visit(ast::GetTypeVisitor(), switch_.value);

            if(value_type != INT){
                throw SemanticalException("Switch can only work on int type", switch_.position);
            }

            visit_each(*this, switch_.instructions);
        }
        
        void operator()(ast::Ternary& ternary){
            visit(*this, ternary.Content->condition);
            visit(*this, ternary.Content->false_value);
            visit(*this, ternary.Content->true_value);

            auto condition_type = visit(ast::GetTypeVisitor(), ternary.Content->condition);
            if(condition_type != BOOL){
                throw SemanticalException("Ternary can only be applied to bool", ternary.Content->position);
            }
            
            auto true_type = visit(ast::GetTypeVisitor(), ternary.Content->true_value);
            auto false_type = visit(ast::GetTypeVisitor(), ternary.Content->false_value);
            if(true_type != false_type){
                throw SemanticalException("Incompatible values in ternary operator", ternary.Content->position);
            }
        }

        void operator()(ast::Assignment& assignment){
            visit(*this, assignment.Content->left_value);
            visit(*this, assignment.Content->value);
                        
            auto left_value_type = visit(ast::GetTypeVisitor(), assignment.Content->left_value);
            auto right_value_type = visit(ast::GetTypeVisitor(), assignment.Content->value);
                
            if (left_value_type != right_value_type){
                if(left_value_type->is_pointer()){
                    //Addresses are taken implicitly
                    if(left_value_type->data_type() != right_value_type){
                        throw SemanticalException("Incompatible type in assignment", assignment.Content->position);
                    }
                } else {
                    throw SemanticalException("Incompatible type in assignment", assignment.Content->position);
                }
            }

            //Special rules for assignments of variales
            if(auto* ptr = boost::get<ast::VariableValue>(&assignment.Content->left_value)){
                auto var = (*ptr).variable();

                if(var->type()->is_const()){
                    throw SemanticalException("The variable " + var->name() + " is const, cannot edit it", assignment.Content->position);
                }

                if(var->position().isParameter() || var->position().isParamRegister()){
                    throw SemanticalException("Cannot change the value of the parameter " + var->name(), assignment.Content->position);
                }
            }
        }

        template<typename Operation>
        void checkSuffixOrPrefixOperation(Operation& operation){
            auto type = visit(ast::GetTypeVisitor(), operation.Content->left_value);
            
            if(type != INT && type != FLOAT){
                throw SemanticalException("The value is not of type int or float, cannot increment or decrement it", operation.Content->position);
            }

            if(type->is_const()){
                throw SemanticalException("The value is const, cannot edit it", operation.Content->position);
            }
        }

        void operator()(ast::SuffixOperation& operation){
            checkSuffixOrPrefixOperation(operation);
        }

        void operator()(ast::PrefixOperation& operation){
            checkSuffixOrPrefixOperation(operation);
        }

        void operator()(ast::Return& return_){
            visit(*this, return_.Content->value);
           
            auto return_type = visit(ast::GetTypeVisitor(), return_.Content->value);
            if(return_type != return_.Content->function->returnType){
                throw SemanticalException("The return value is not of the good type in the function " + return_.Content->function->name, return_.Content->position);
            }
        }
        
        void operator()(ast::VariableDeclaration& declaration){
            if(declaration.Content->value){
                visit(*this, *declaration.Content->value);

                auto var = (*declaration.Content->context)[declaration.Content->variableName];
                
                auto valueType = visit(ast::GetTypeVisitor(), *declaration.Content->value);
                if (valueType != var->type()->non_const()) {
                    throw SemanticalException("Incompatible type in declaration of variable " + declaration.Content->variableName, declaration.Content->position);
                }
            }
        }
        
        void operator()(ast::Swap& swap){
            if (swap.Content->lhs_var->type() != swap.Content->rhs_var->type()) {
                throw SemanticalException("Swap of variables of incompatible type", swap.Content->position);
            }
        }

        void operator()(ast::ArrayValue& array){
            visit(*this, array.Content->indexValue);

            auto var_type = array.Content->var->type();

            if(!var_type->is_array() && var_type != STRING){
                throw SemanticalException(array.Content->arrayName + " is not an array, neither a string", array.Content->position);
            }

            auto index_type = visit(ast::GetTypeVisitor(), array.Content->indexValue);
            if (index_type != INT || index_type->is_array()) {
                throw SemanticalException("Invalid index for the array " + array.Content->arrayName, array.Content->position);
            }
        }
        
        void operator()(ast::Cast& cast){
            auto dst_type = visit(ast::TypeTransformer(context), cast.Content->type);
            auto src_type = visit(ast::GetTypeVisitor(), cast.Content->value);

            if(dst_type == INT){
                if(src_type != FLOAT && src_type != INT && src_type != CHAR){
                    throw SemanticalException("Invalid cast", cast.Content->position);
                }
            } else if(dst_type == FLOAT){
                if(src_type != INT && src_type != FLOAT){
                    throw SemanticalException("Invalid cast", cast.Content->position);
                }
            } else if(dst_type == CHAR){
                if(src_type != INT && src_type != CHAR){
                    throw SemanticalException("Invalid cast", cast.Content->position);
                }
            } else {
                throw SemanticalException("Invalid cast", cast.Content->position);
            }
        }

        void operator()(ast::Expression& value){
            visit(*this, value.Content->first);
            
            for_each(value.Content->operations, [&](ast::Operation& operation){ visit(*this, operation.get<1>()); });

            ast::GetTypeVisitor visitor;
            auto type = visit(visitor, value.Content->first);

            for(auto& operation : value.Content->operations){
                auto operationType = visit(visitor, operation.get<1>());

                if(type != operationType){
                    throw SemanticalException("Incompatible type", value.Content->position);
                }
                    
                auto op = operation.get<0>();
                
                if(type == INT){
                    if(op != ast::Operator::DIV && op != ast::Operator::MUL && op != ast::Operator::SUB && op != ast::Operator::ADD && op != ast::Operator::MOD &&
                        op != ast::Operator::GREATER && op != ast::Operator::GREATER_EQUALS && op != ast::Operator::LESS && op != ast::Operator::LESS_EQUALS &&
                            op != ast::Operator::EQUALS && op != ast::Operator::NOT_EQUALS){
                        throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on int");
                    }
                }

                if(type == FLOAT){
                    if(op != ast::Operator::DIV && op != ast::Operator::MUL && op != ast::Operator::SUB && op != ast::Operator::ADD &&
                        op != ast::Operator::GREATER && op != ast::Operator::GREATER_EQUALS && op != ast::Operator::LESS && op != ast::Operator::LESS_EQUALS &&
                            op != ast::Operator::EQUALS && op != ast::Operator::NOT_EQUALS){
                        throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on float");
                    }
                }
                
                if(type == STRING){
                    if(op != ast::Operator::ADD){
                        throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on string");
                    }
                }
                
                if(type == BOOL){
                    if(op != ast::Operator::AND && op != ast::Operator::OR){
                        throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on bool");
                    }
                }
            }
        }

        void operator()(ast::BuiltinOperator& builtin){
            for_each(builtin.Content->values, [&](ast::Value& value){ visit(*this, value); });
           
            if(builtin.Content->values.size() < 1){
                throw SemanticalException("Too few arguments to the builtin operator", builtin.Content->position);
            }
           
            if(builtin.Content->values.size() > 1){
                throw SemanticalException("Too many arguments to the builtin operator", builtin.Content->position);
            }
            
            auto type = visit(ast::GetTypeVisitor(), builtin.Content->values[0])->non_const();

            if(builtin.Content->type == ast::BuiltinType::SIZE){
                if(!type->is_array()){
                    throw SemanticalException("The builtin size() operator takes only array as arguments", builtin.Content->position);
                }
            } else if(builtin.Content->type == ast::BuiltinType::LENGTH){
                if(type != STRING){
                    throw SemanticalException("The builtin length() operator takes only string as arguments", builtin.Content->position);
                }
            }
        }

        void operator()(ast::IntegerSuffix& integer){
            std::string suffix = integer.suffix;

            if(suffix != "f"){
                throw SemanticalException("There are no such suffix as \"" + suffix  + "\" for integers. ");
            }
        }
        
        void operator()(ast::New& new_){
            auto type = visit(ast::TypeTransformer(context), new_.Content->type);

            if(!(type->is_standard_type() || type->is_custom_type())){
                throw SemanticalException("Only standard types and struct types can be dynamically allocated", new_.Content->position);
            }
        }
        
        void operator()(ast::Delete& delete_){
            auto type = delete_.Content->variable->type();

            if(!type->is_pointer()){
                throw SemanticalException("Only pointers can be deleted", delete_.Content->position);
            }
        }

    private:
        std::shared_ptr<GlobalContext> context;
};

} //end of anonymous namespace

void ast::checkTypes(ast::SourceFile& program){
    CheckerVisitor visitor(program.Content->context);
    visit_non_variant(visitor, program);
}
