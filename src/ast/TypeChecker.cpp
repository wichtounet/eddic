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

#include "Compiler.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Types.hpp"
#include "Variable.hpp"
#include "Options.hpp"
#include "VisitorUtils.hpp"

using namespace eddic;

struct CheckerVisitor : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_CALLS()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
    AUTO_RECURSE_MINUS_PLUS_VALUES()
   
    void operator()(ast::FunctionDeclaration& declaration){
        visit_each(*this, declaration.Content->instructions);
    }
    
    void operator()(ast::GlobalVariableDeclaration& declaration){
        Type type = newType(declaration.Content->variableType); 

        Type valueType = visit(ast::GetTypeVisitor(), *declaration.Content->value);
        if (valueType != type) {
            throw SemanticalException("Incompatible type for global variable " + declaration.Content->variableName);
        }
    }

    void operator()(ast::Import&){
        //Nothing to check here
    }

    void operator()(ast::StandardImport&){
        //Nothing to check here
    }

    void operator()(ast::GlobalArrayDeclaration&){
        //Nothing to check here
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
        if (valueType != var->type()) {
            throw SemanticalException("Incompatible type in assignment of variable " + assignment.Content->variableName);
        }

        if(var->type().isConst()){
            throw SemanticalException("The variable " + assignment.Content->variableName + " is const, cannot edit it");
        }

        if(var->position().isParameter()){
            throw SemanticalException("Cannot change the value of the parameter " + assignment.Content->variableName);
        }
    }

    void operator()(ast::Assignment& assignment){
        checkAssignment(assignment);
    }
    
    void operator()(ast::CompoundAssignment& assignment){
        checkAssignment(assignment);
    }

    void operator()(ast::SuffixOperation& operation){
        auto var = operation.Content->variable;
        
        if(var->type().isArray() || var->type().base() != BaseType::INT){
            throw SemanticalException("The variable " + var->name() + " is not of type int, cannot increment or decrement it");
        }

        if(var->type().isConst()){
            throw SemanticalException("The variable " + var->name() + " is const, cannot edit it");
        }
    }

    void operator()(ast::PrefixOperation& operation){
        auto var = operation.Content->variable;
        
        if(var->type().isArray() || var->type().base() != BaseType::INT){
            throw SemanticalException("The variable " + var->name() + " is not of type int, cannot increment or decrement it");
        }
        
        if(var->type().isConst()){
            throw SemanticalException("The variable " + var->name() + " is const, cannot edit it");
        }
    }

    void operator()(ast::Return& return_){
        visit(*this, return_.Content->value);
       
        Type returnValueType = visit(ast::GetTypeVisitor(), return_.Content->value);
        if(returnValueType != return_.Content->function->returnType){
            throw SemanticalException("The return value is not of the good type in the function " + return_.Content->function->name);
        }
    }

    void operator()(ast::ArrayAssignment& assignment){
        visit(*this, assignment.Content->indexValue);
        visit(*this, assignment.Content->value);

        auto var = assignment.Content->context->getVariable(assignment.Content->variableName);

        Type valueType = visit(ast::GetTypeVisitor(), assignment.Content->value);
        if (valueType.base() != var->type().base()) {
            throw SemanticalException("Incompatible type in assignment of array " + assignment.Content->variableName);
        }
        
        Type indexType = visit(ast::GetTypeVisitor(), assignment.Content->indexValue);
        if (indexType.base() != BaseType::INT) {
            throw SemanticalException("Invalid index value type in assignment of array " + assignment.Content->variableName);
        }
    }
    
    void operator()(ast::VariableDeclaration& declaration){
        visit(*this, *declaration.Content->value);

        Type variableType = newType(declaration.Content->variableType);
        Type valueType = visit(ast::GetTypeVisitor(), *declaration.Content->value);
        if (valueType != variableType) {
            throw SemanticalException("Incompatible type in declaration of variable " + declaration.Content->variableName);
        }
    }
    
    void operator()(ast::ArrayDeclaration&){
        //No need for type checking here
    }
    
    void operator()(ast::Swap& swap){
        if (swap.Content->lhs_var->type() != swap.Content->rhs_var->type()) {
            throw SemanticalException("Swap of variables of incompatible type");
        }
    }

    void operator()(ast::ArrayValue& array){
        visit(*this, array.Content->indexValue);

        Type valueType = visit(ast::GetTypeVisitor(), array.Content->indexValue);
        if (valueType.base() != BaseType::INT || valueType.isArray()) {
            throw SemanticalException("Invalid index for the array " + array.Content->arrayName);
        }
    }

    void operator()(ast::ComposedValue& value){
        visit(*this, value.Content->first);
        
        for_each(value.Content->operations.begin(), value.Content->operations.end(), 
            [&](ast::Operation& operation){ visit(*this, operation.get<1>()); });

        ast::GetTypeVisitor visitor;
        Type type = visit(visitor, value.Content->first);

        for(auto& operation : value.Content->operations){
            Type operationType = visit(visitor, operation.get<1>());

            if(type != operationType){
                throw SemanticalException("Incompatible type");
            }
                
            auto op = operation.get<0>();
            
            if(type.base() == BaseType::INT){
                if(op != ast::Operator::DIV && op != ast::Operator::MUL && op != ast::Operator::SUB && op != ast::Operator::ADD && op != ast::Operator::MOD){
                    throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on int");
                }
            }

            if(type.base() == BaseType::FLOAT){
                if(op != ast::Operator::DIV && op != ast::Operator::MUL && op != ast::Operator::SUB && op != ast::Operator::ADD){
                    throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on float");
                }
            }
            
            if(type.base() == BaseType::STRING){
                if(op != ast::Operator::ADD){
                    throw SemanticalException("The " + ast::toString(op) + " operator cannot be applied on string");
                }
            }
            
            if(type.base() == BaseType::BOOL){
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
            throw SemanticalException("Too few arguments to the builtin operator");
        }
       
        if(builtin.Content->values.size() > 1){
            throw SemanticalException("Too many arguments to the builtin operator");
        }
        
        ast::GetTypeVisitor visitor;
        Type type = visit(visitor, builtin.Content->values[0]);

        if(builtin.Content->type == ast::BuiltinType::SIZE){
            if(!type.isArray()){
                throw SemanticalException("The builtin size() operator takes only array as arguments");
            }
        } else if(builtin.Content->type == ast::BuiltinType::LENGTH){
            if(type.isArray() || type.base() != BaseType::STRING){
                throw SemanticalException("The builtin length() operator takes only string as arguments");
            }
        }
    }

    void operator()(ast::VariableValue&){
        //Nothing to check here
    }

    void operator()(ast::TerminalNode&){
        //Terminal nodes have no need for type checking    
    }
};

void ast::TypeChecker::check(ast::SourceFile& program) const {
    CheckerVisitor visitor;
    visit_non_variant(visitor, program);
}
