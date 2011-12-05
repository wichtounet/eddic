//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include <memory>
#include <boost/variant/variant.hpp>

#include "TypeChecker.hpp"

#include "Compiler.hpp"

#include "IsConstantVisitor.hpp"
#include "GetTypeVisitor.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Types.hpp"
#include "Variable.hpp"

#include "Options.hpp"
#include "TypeTransformer.hpp"

#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

#include "ast/SourceFile.hpp"

using namespace eddic;

struct CheckerVisitor : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_CALLS()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
   
    void operator()(ast::FunctionDeclaration& declaration){
        visit_each(*this, declaration.Content->instructions);
    }
    
    void operator()(ast::GlobalVariableDeclaration& declaration){
        Type type = stringToType(declaration.Content->variableType); 

        Type valueType = boost::apply_visitor(GetTypeVisitor(), *declaration.Content->value);
        if (valueType != type) {
            throw SemanticalException("Incompatible type for global variable " + declaration.Content->variableName);
        }
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

    void operator()(ast::Assignment& assignment){
        visit(*this, assignment.Content->value);

        auto var = assignment.Content->context->getVariable(assignment.Content->variableName);

        Type valueType = boost::apply_visitor(GetTypeVisitor(), assignment.Content->value);
        if (valueType != var->type()) {
            throw SemanticalException("Incompatible type in assignment of variable " + assignment.Content->variableName);
        }

        if(var->type().isConst()){
            throw SemanticalException("The variable " + assignment.Content->variableName + " is const, cannot edit it");
        }
    }

    void operator()(ast::Return& return_){
        visit(*this, return_.Content->value);
       
        Type returnValueType = boost::apply_visitor(GetTypeVisitor(), return_.Content->value);
        if(returnValueType != return_.Content->function->returnType){
            throw SemanticalException("The return value is not of the good type in the function " + return_.Content->function->name);
        }
    }

    void operator()(ast::ArrayAssignment& assignment){
        visit(*this, assignment.Content->indexValue);
        visit(*this, assignment.Content->value);

        auto var = assignment.Content->context->getVariable(assignment.Content->variableName);

        Type valueType = boost::apply_visitor(GetTypeVisitor(), assignment.Content->value);
        if (valueType.base() != var->type().base()) {
            throw SemanticalException("Incompatible type in assignment of array " + assignment.Content->variableName);
        }
        
        Type indexType = boost::apply_visitor(GetTypeVisitor(), assignment.Content->indexValue);
        if (indexType.base() != BaseType::INT) {
            throw SemanticalException("Invalid index value type in assignment of array " + assignment.Content->variableName);
        }
    }
    
    void operator()(ast::VariableDeclaration& declaration){
        visit(*this, *declaration.Content->value);

        Type variableType = stringToType(declaration.Content->variableType);
        Type valueType = boost::apply_visitor(GetTypeVisitor(), *declaration.Content->value);
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

        Type valueType = boost::apply_visitor(GetTypeVisitor(), array.Content->indexValue);
        if (valueType.base() != BaseType::INT || valueType.isArray()) {
            throw SemanticalException("Invalid index for the array " + array.Content->arrayName);
        }
    }

    void operator()(ast::ComposedValue& value){
        visit(*this, value.Content->first);
        
        for_each(value.Content->operations.begin(), value.Content->operations.end(), 
            [&](boost::tuple<char, ast::Value>& operation){ visit(*this, operation.get<1>()); });

        GetTypeVisitor visitor;
        Type type = boost::apply_visitor(visitor, value.Content->first);

        for(auto& operation : value.Content->operations){
            Type operationType = boost::apply_visitor(visitor, operation.get<1>());

            if(type != operationType){
                throw SemanticalException("Incompatible type");
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

void TypeChecker::check(ast::SourceFile& program) const {
    CheckerVisitor visitor;
    visit_non_variant(visitor, program);
}
