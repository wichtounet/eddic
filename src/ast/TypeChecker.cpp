//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <memory>

#include "variant.hpp"
#include "SemanticalException.hpp"
#include "TerminationException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Options.hpp"
#include "VisitorUtils.hpp"
#include "Utils.hpp"
#include "Type.hpp"
#include "mangling.hpp"

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

        AUTO_RECURSE_FUNCTION_DECLARATION()
        AUTO_RECURSE_CONSTRUCTOR()
        AUTO_RECURSE_DESTRUCTOR()
        AUTO_RECURSE_FUNCTION_CALLS()
        AUTO_RECURSE_SIMPLE_LOOPS()
        AUTO_RECURSE_BRANCHES()
        AUTO_RECURSE_DEFAULT_CASE()
        AUTO_RECURSE_STRUCT_DECLARATION()

        AUTO_IGNORE_MEMBER_DECLARATION()
        AUTO_IGNORE_TEMPLATE_FUNCTION_DECLARATION()
        AUTO_IGNORE_ARRAY_DECLARATION()
        AUTO_IGNORE_BOOLEAN()
        AUTO_IGNORE_NULL()
        AUTO_IGNORE_LITERAL()
        AUTO_IGNORE_CHAR_LITERAL()
        AUTO_IGNORE_FLOAT()
        AUTO_IGNORE_INTEGER()
        AUTO_IGNORE_IMPORT()
        AUTO_IGNORE_STANDARD_IMPORT()
        AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION()
        AUTO_IGNORE_VARIABLE_VALUE()

        void operator()(ast::struct_definition& struct_){
            if(!struct_.is_template_declaration()){
                visit_each(*this, struct_.blocks);
            }
        }

        void operator()(ast::GlobalVariableDeclaration& declaration){
            auto type = visit(ast::TypeTransformer(context), declaration.variableType);

            auto valueType = visit(ast::GetTypeVisitor(), *declaration.value);
            if (valueType != type) {
                throw SemanticalException("Incompatible type for global variable " + declaration.variableName, declaration.position);
            }
        }

        void operator()(ast::Foreach& foreach){
            visit_each(*this, foreach.instructions);
        }

        void operator()(ast::ForeachIn& foreach){
            auto var_type = foreach.var->type();
            auto array_type = foreach.arrayVar->type();

            if(array_type->is_array()){
                if(var_type != array_type->data_type()){
                    throw SemanticalException("Incompatible type in declaration of the foreach variable " + foreach.variableName, foreach.position);
                }
            } else if(array_type == STRING){
                if(var_type != CHAR){
                    throw SemanticalException("Foreach in string yields a char", foreach.position);
                }
            } else {
                throw SemanticalException("Cannot use foreach in variable " + foreach.arrayName, foreach.position);
            }

            visit_each(*this, foreach.instructions);
        }

        void operator()(ast::Switch& switch_){
            visit(*this, switch_.value);

            auto value_type = visit(ast::GetTypeVisitor(), switch_.value);

            if(value_type != INT && value_type != STRING){
                throw SemanticalException("Switch can only work on int and string types", switch_.position);
            }

            visit_each_non_variant(*this, switch_.cases);
            visit_optional_non_variant(*this, switch_.default_case);
        }

        void operator()(ast::SwitchCase& switch_){
            visit(*this, switch_.value);

            auto value_type = visit(ast::GetTypeVisitor(), switch_.value);

            if(value_type != INT && value_type != STRING){
                throw SemanticalException("Switch can only work on int and string types", switch_.position);
            }

            visit_each(*this, switch_.instructions);
        }

        void operator()(ast::Ternary& ternary){
            visit(*this, ternary.condition);
            visit(*this, ternary.false_value);
            visit(*this, ternary.true_value);

            auto condition_type = visit(ast::GetTypeVisitor(), ternary.condition);
            if(condition_type != BOOL){
                throw SemanticalException("Ternary can only be applied to bool", ternary.position);
            }

            auto true_type = visit(ast::GetTypeVisitor(), ternary.true_value);
            auto false_type = visit(ast::GetTypeVisitor(), ternary.false_value);
            if(true_type != false_type){
                throw SemanticalException("Incompatible values in ternary operator", ternary.position);
            }
        }

        void operator()(ast::Assignment& assignment){
            visit(*this, assignment.left_value);
            visit(*this, assignment.value);

            auto left_value_type = visit(ast::GetTypeVisitor(), assignment.left_value);
            auto right_value_type = visit(ast::GetTypeVisitor(), assignment.value);

            if (left_value_type != right_value_type){
                if(left_value_type->is_pointer()){
                    //Addresses are taken implicitly
                    if(left_value_type->data_type() != right_value_type){
                        throw SemanticalException("Incompatible type in assignment", assignment.position);
                    }
                } else {
                    throw SemanticalException("Incompatible type in assignment", assignment.position);
                }
            }

            if(left_value_type->is_structure()){
                std::vector<std::shared_ptr<const Type>> ctor_types = {new_pointer_type(left_value_type)};
                auto ctor_name = mangle_ctor(ctor_types, left_value_type);

                if(!context->exists(ctor_name)){
                    throw SemanticalException("Assigning to a structure needs a copy constructor", assignment.position);
                }
            }

            //Special rules for assignments of variables
            if(auto* ptr = boost::get<ast::VariableValue>(&assignment.left_value)){
                auto var = (*ptr).variable();

                if(var->type()->is_const()){
                    throw SemanticalException("The variable " + var->name() + " is const, cannot edit it", assignment.position);
                }

                if(var->position().isParameter() || var->position().isParamRegister()){
                    throw SemanticalException("Cannot change the value of the parameter " + var->name(), assignment.position);
                }
            }
        }

        void operator()(ast::PrefixOperation& operation){
            if(operation.op == ast::Operator::INC || operation.op == ast::Operator::DEC){
                auto type = visit(ast::GetTypeVisitor(), operation.left_value);

                if(type != INT && type != FLOAT){
                    throw SemanticalException("The value is not of type int or float, cannot increment or decrement it", operation.position);
                }

                if(type->is_const()){
                    throw SemanticalException("The value is const, cannot edit it", operation.position);
                }
            }
        }

        void operator()(ast::Return& return_){
            visit(*this, return_.value);

            auto return_type = visit(ast::GetTypeVisitor(), return_.value);
            auto& function = return_.context->global()->getFunction(return_.mangled_name);
            if(return_type != function.return_type()){
                throw SemanticalException("The return value is not of the good type in the function " + function.name(), return_.position);
            }
        }

        void operator()(ast::VariableDeclaration& declaration){
            if(declaration.value){
                visit(*this, *declaration.value);

                auto var = (*declaration.context)[declaration.variableName];

                auto valueType = visit(ast::GetTypeVisitor(), *declaration.value);
                if (valueType != var->type()) {
                    throw SemanticalException("Incompatible type in declaration of variable " + declaration.variableName, declaration.position);
                }
            }
        }

        void operator()(ast::Cast& cast){
            auto dst_type = visit_non_variant(ast::GetTypeVisitor(), cast);
            auto src_type = visit(ast::GetTypeVisitor(), cast.value);

            //Cast with no effects are always valid
            if(
                        dst_type == src_type
                    ||  (dst_type->is_pointer() && src_type->is_pointer())
                    ||  (dst_type->is_dynamic_array() && src_type->is_pointer())
                    ||  (dst_type->is_dynamic_array() && src_type->is_dynamic_array())){
                return;
            }

            if(dst_type == INT){
                if(src_type != FLOAT && src_type != CHAR){
                    throw SemanticalException("Invalid cast to int", cast.position);
                }
            } else if(dst_type == FLOAT){
                if(src_type != INT){
                    throw SemanticalException("Invalid cast to float", cast.position);
                }
            } else if(dst_type == CHAR){
                if(src_type != INT){
                    throw SemanticalException("Invalid cast to char", cast.position);
                }
            } else {
                throw SemanticalException("Invalid cast", cast.position);
            }
        }

        void operator()(ast::Expression& value){
            VISIT_COMPOSED_VALUE(value);

            ast::GetTypeVisitor visitor;
            auto type = visit(visitor, value.first);

            auto global_context = value.context->global();

            for(auto& operation : value.operations){
                auto op = operation.get<0>();

                //1. Verify that the left type is OK for the current operation
                if(op == ast::Operator::BRACKET){
                    if(!type->is_array() && type != STRING){
                        throw SemanticalException("The left value is not an array, neither a string", value.position);
                    }

                    auto index_type = visit(visitor, operation.get<1>());
                    if (index_type != INT || index_type->is_array()) {
                        throw SemanticalException("Invalid type for the index value, only int indices are allowed", value.position);
                    }
                } else if(op == ast::Operator::INC || op == ast::Operator::DEC){
                    if(type != INT && type != FLOAT){
                        throw SemanticalException("The value is not of type int or float, cannot increment or decrement it", value.position);
                    }

                    if(type->is_const()){
                        throw SemanticalException("The value is const, cannot edit it", value.position);
                    }
                } else if(op == ast::Operator::DOT){
                    //Checked by structure and variables annotators
                } else if(op == ast::Operator::CALL){
                    //Checked by the function checkers
                } else {
                    auto operationType = visit(visitor, operation.get<1>());

                    if(type->is_pointer()){
                        if(!operationType->is_pointer()){
                            throw SemanticalException("Incompatible type", value.position);
                        }
                    } else if(type != operationType){
                        throw SemanticalException("Incompatible type", value.position);
                    }

                    auto op = operation.get<0>();

                    if(type->is_pointer()){
                        if(op != ast::Operator::EQUALS && op != ast::Operator::NOT_EQUALS){
                            throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on pointers", value.position);
                        }
                    }

                    if(type == INT){
                        if(op != ast::Operator::DIV && op != ast::Operator::MUL && op != ast::Operator::SUB && op != ast::Operator::ADD && op != ast::Operator::MOD &&
                                op != ast::Operator::GREATER && op != ast::Operator::GREATER_EQUALS && op != ast::Operator::LESS && op != ast::Operator::LESS_EQUALS &&
                                op != ast::Operator::EQUALS && op != ast::Operator::NOT_EQUALS){
                            throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on int", value.position);
                        }
                    }

                    if(type == FLOAT){
                        if(op != ast::Operator::DIV && op != ast::Operator::MUL && op != ast::Operator::SUB && op != ast::Operator::ADD &&
                                op != ast::Operator::GREATER && op != ast::Operator::GREATER_EQUALS && op != ast::Operator::LESS && op != ast::Operator::LESS_EQUALS &&
                                op != ast::Operator::EQUALS && op != ast::Operator::NOT_EQUALS){
                            throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on float", value.position);
                        }
                    }

                    if(type == STRING){
                        throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on string", value.position);
                    }

                    if(type == BOOL){
                        if(op != ast::Operator::AND && op != ast::Operator::OR){
                            throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on bool", value.position);
                        }
                    }
                }

                //2. Compute the next type
                type = ast::operation_type(type, value.context, operation);
            }
        }

        void operator()(ast::BuiltinOperator& builtin){
            for_each(builtin.values, [&](ast::Value& value){ visit(*this, value); });

            if(builtin.values.size() < 1){
                throw SemanticalException("Too few arguments to the builtin operator", builtin.position);
            }

            if(builtin.values.size() > 1){
                throw SemanticalException("Too many arguments to the builtin operator", builtin.position);
            }

            auto type = visit(ast::GetTypeVisitor(), builtin.values[0]);

            if(builtin.type == ast::BuiltinType::SIZE){
                if(!type->is_array()){
                    throw SemanticalException("The builtin size() operator takes only array as arguments", builtin.position);
                }
            } else if(builtin.type == ast::BuiltinType::LENGTH){
                if(type != STRING){
                    throw SemanticalException("The builtin length() operator takes only string as arguments", builtin.position);
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
            visit_each(*this, new_.values);

            auto type = visit(ast::TypeTransformer(context), new_.type);

            if(!(type->is_standard_type() || type->is_custom_type() || type->is_template_type())){
                throw SemanticalException("Only standard types and struct types can be dynamically allocated", new_.position);
            }
        }

        void operator()(ast::NewArray& new_){
            auto type = visit(ast::TypeTransformer(context), new_.type);

            if(type->is_array()){
                throw SemanticalException("Multidimensional arrays are not supported", new_.position);
            }
        }

        void operator()(ast::Delete& delete_){
            auto type = visit(ast::GetTypeVisitor(), delete_.value);

            if(!type->is_pointer() && !type->is_dynamic_array()){
                throw SemanticalException("Only pointers can be deleted", delete_.position);
            }
        }

    private:
        std::shared_ptr<GlobalContext> context;
};

} //end of anonymous namespace

//TODO Rewrite the type checker so that it does not use apply_program
//And remove the exception code from here

void ast::TypeCheckingPass::apply_program(ast::SourceFile& program, bool){
    CheckerVisitor visitor(program.context);

    bool valid = true;

    for(auto& block : program.blocks){
        try {
            visit(visitor, block);
        } catch (const SemanticalException& e){
            if(!configuration->option_defined("quiet")){
                output_exception(e, program.context);
            }
            valid = false;
        }
    }

    if(!valid){
        throw TerminationException();
    }
}

bool ast::TypeCheckingPass::is_simple(){
    return true;
}
