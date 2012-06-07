//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <memory>
#include <boost/variant/variant.hpp>

#include "ast/TypeChecker.hpp"
#include "ast/SourceFile.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/TypeTransformer.hpp"

#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Types.hpp"
#include "Variable.hpp"
#include "Options.hpp"
#include "VisitorUtils.hpp"
#include "SymbolTable.hpp"

using namespace eddic;

struct CheckerVisitor : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_DECLARATION()
    AUTO_RECURSE_FUNCTION_CALLS()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
    AUTO_RECURSE_MINUS_PLUS_VALUES()
        
    AUTO_IGNORE_ARRAY_DECLARATION()
    AUTO_IGNORE_FALSE()
    AUTO_IGNORE_TRUE()
    AUTO_IGNORE_LITERAL()
    AUTO_IGNORE_FLOAT()
    AUTO_IGNORE_INTEGER()
    AUTO_IGNORE_IMPORT()
    AUTO_IGNORE_STANDARD_IMPORT()
    AUTO_IGNORE_STRUCT()
    AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION()
    AUTO_IGNORE_VARIABLE_VALUE()
    
    void operator()(ast::GlobalVariableDeclaration& declaration){
        Type type = newType(declaration.Content->variableType); 

        Type valueType = visit(ast::GetTypeVisitor(), *declaration.Content->value);
        if (valueType != type) {
            throw SemanticalException("Incompatible type for global variable " + declaration.Content->variableName, declaration.Content->position);
        }
    }
    
    void operator()(ast::Foreach& foreach){
        visit_each(*this, foreach.Content->instructions);
    }
    
    void operator()(ast::ForeachIn& foreach){
        //TODO Check types of array
        //TODO Check type of varaible = base of array

        visit_each(*this, foreach.Content->instructions);
    }

    template<typename T>
    void checkAssignment(T& assignment){
        visit(*this, assignment.Content->value);

        auto var = assignment.Content->context->getVariable(assignment.Content->variableName);

        Type valueType = visit(ast::GetTypeVisitor(), assignment.Content->value);
        if (valueType != var->type().non_const()) {
            throw SemanticalException("Incompatible type in assignment of variable " + assignment.Content->variableName, assignment.Content->position);
        }

        if(var->type().isConst()){
            throw SemanticalException("The variable " + assignment.Content->variableName + " is const, cannot edit it", assignment.Content->position);
        }

        if(var->position().isParameter() || var->position().isParamRegister()){
            throw SemanticalException("Cannot change the value of the parameter " + assignment.Content->variableName, assignment.Content->position);
        }
    }

    void operator()(ast::Assignment& assignment){
        checkAssignment(assignment);
    }
    
    void operator()(ast::CompoundAssignment& assignment){
        checkAssignment(assignment);
    }
    
    void operator()(ast::StructCompoundAssignment& assignment){
        visit(*this, assignment.Content->value);
        
        auto var = (*assignment.Content->context)[assignment.Content->variableName];

        auto struct_name = var->type().type();
        auto struct_type = symbols.get_struct(struct_name);

        auto& members = assignment.Content->memberNames;
        for(std::size_t i = 0; i < members.size(); ++i){
            auto& member = members[i];

            auto member_type = (*struct_type)[member]->type;
            
            if(i == members.size() - 1){
                Type valueType = visit(ast::GetTypeVisitor(), assignment.Content->value);
                if (valueType != member_type) {
                    throw SemanticalException("Incompatible type in assignment of struct member " + assignment.Content->variableName, assignment.Content->position);
                }
            } else {
                struct_name = member_type.type();
                struct_type = symbols.get_struct(struct_name);
            }
        }
    }

    template<typename Operation>
    void checkSuffixOrPrefixOperation(Operation& operation){
        auto var = operation.Content->variable;
        
        if(var->type() != BaseType::INT && var->type() != BaseType::FLOAT){
            throw SemanticalException("The variable " + var->name() + " is not of type int or float, cannot increment or decrement it", operation.Content->position);
        }

        if(var->type().isConst()){
            throw SemanticalException("The variable " + var->name() + " is const, cannot edit it", operation.Content->position);
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
       
        Type returnValueType = visit(ast::GetTypeVisitor(), return_.Content->value);
        if(returnValueType != return_.Content->function->returnType){
            throw SemanticalException("The return value is not of the good type in the function " + return_.Content->function->name, return_.Content->position);
        }
    }

    void operator()(ast::ArrayAssignment& assignment){
        visit(*this, assignment.Content->indexValue);
        visit(*this, assignment.Content->value);

        auto var = assignment.Content->context->getVariable(assignment.Content->variableName);

        Type valueType = visit(ast::GetTypeVisitor(), assignment.Content->value);
        if (valueType.base() != var->type().base()) {
            throw SemanticalException("Incompatible type in assignment of array " + assignment.Content->variableName, assignment.Content->position);
        }
        
        Type indexType = visit(ast::GetTypeVisitor(), assignment.Content->indexValue);
        if (indexType.base() != BaseType::INT) {
            throw SemanticalException("Invalid index value type in assignment of array " + assignment.Content->variableName, assignment.Content->position);
        }
    }

    void operator()(ast::StructAssignment& assignment){
        visit(*this, assignment.Content->value);
        
        auto var = (*assignment.Content->context)[assignment.Content->variableName];
        auto struct_name = var->type().type();
        auto struct_type = symbols.get_struct(struct_name);

        auto& members = assignment.Content->memberNames;
        for(std::size_t i = 0; i < members.size(); ++i){
            auto& member = members[i];

            auto member_type = (*struct_type)[member]->type;
            
            if(i == members.size() - 1){
                Type valueType = visit(ast::GetTypeVisitor(), assignment.Content->value);
                if (valueType != member_type) {
                    throw SemanticalException("Incompatible type in assignment of struct member " + assignment.Content->variableName, assignment.Content->position);
                }
            } else {
                struct_name = member_type.type();
                struct_type = symbols.get_struct(struct_name);
            }
        }
    }
    
    void operator()(ast::VariableDeclaration& declaration){
        if(declaration.Content->value){
            visit(*this, *declaration.Content->value);

            auto var = (*declaration.Content->context)[declaration.Content->variableName];
            
            Type valueType = visit(ast::GetTypeVisitor(), *declaration.Content->value);
            if (valueType != var->type().non_const()) {
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

        Type valueType = visit(ast::GetTypeVisitor(), array.Content->indexValue);
        if (valueType.base() != BaseType::INT || valueType.isArray()) {
            throw SemanticalException("Invalid index for the array " + array.Content->arrayName, array.Content->position);
        }
    }
    
    void operator()(ast::Cast& cast){
        Type destType = visit(ast::TypeTransformer(), cast.Content->type);

        if(destType == BaseType::STRING){
            throw SemanticalException("Cannot cast to string", cast.Content->position);
        } else if(destType == BaseType::VOID){
            throw SemanticalException("Cannot cast to void", cast.Content->position);
        }
    }

    void operator()(ast::Expression& value){
        visit(*this, value.Content->first);
        
        for_each(value.Content->operations.begin(), value.Content->operations.end(), 
            [&](ast::Operation& operation){ visit(*this, operation.get<1>()); });

        ast::GetTypeVisitor visitor;
        Type type = visit(visitor, value.Content->first);

        for(auto& operation : value.Content->operations){
            Type operationType = visit(visitor, operation.get<1>());

            if(type != operationType){
                throw SemanticalException("Incompatible type", value.Content->position);
            }
                
            auto op = operation.get<0>();
            
            if(type == BaseType::INT){
                if(op != ast::Operator::DIV && op != ast::Operator::MUL && op != ast::Operator::SUB && op != ast::Operator::ADD && op != ast::Operator::MOD &&
                    op != ast::Operator::GREATER && op != ast::Operator::GREATER_EQUALS && op != ast::Operator::LESS && op != ast::Operator::LESS_EQUALS &&
                        op != ast::Operator::EQUALS && op != ast::Operator::NOT_EQUALS){
                    throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on int");
                }
            }

            if(type == BaseType::FLOAT){
                if(op != ast::Operator::DIV && op != ast::Operator::MUL && op != ast::Operator::SUB && op != ast::Operator::ADD &&
                    op != ast::Operator::GREATER && op != ast::Operator::GREATER_EQUALS && op != ast::Operator::LESS && op != ast::Operator::LESS_EQUALS &&
                        op != ast::Operator::EQUALS && op != ast::Operator::NOT_EQUALS){
                    throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on float");
                }
            }
            
            if(type == BaseType::STRING){
                if(op != ast::Operator::ADD){
                    throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on string");
                }
            }
            
            if(type == BaseType::BOOL){
                if(op != ast::Operator::AND && op != ast::Operator::OR){
                    throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on bool");
                }
            }
        }
    }

    void operator()(ast::BuiltinOperator& builtin){
        for_each(builtin.Content->values.begin(), builtin.Content->values.end(), 
            [&](ast::Value& value){ visit(*this, value); });
       
        if(builtin.Content->values.size() < 1){
            throw SemanticalException("Too few arguments to the builtin operator", builtin.Content->position);
        }
       
        if(builtin.Content->values.size() > 1){
            throw SemanticalException("Too many arguments to the builtin operator", builtin.Content->position);
        }
        
        ast::GetTypeVisitor visitor;
        Type type = visit(visitor, builtin.Content->values[0]);

        if(builtin.Content->type == ast::BuiltinType::SIZE){
            if(!type.isArray()){
                throw SemanticalException("The builtin size() operator takes only array as arguments", builtin.Content->position);
            }
        } else if(builtin.Content->type == ast::BuiltinType::LENGTH){
            if(type != BaseType::STRING){
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
};

void ast::checkTypes(ast::SourceFile& program){
    CheckerVisitor visitor;
    visit_non_variant(visitor, program);
}
