//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <memory>

#include <boost/variant/variant.hpp>

#include "ast/VariablesAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/IsConstantVisitor.hpp"
#include "ast/ASTVisitor.hpp"

#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Types.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"

using namespace eddic;

struct VariablesVisitor : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_CALLS()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
    AUTO_RECURSE_BUILTIN_OPERATORS()
    AUTO_RECURSE_MINUS_PLUS_VALUES()
    AUTO_RECURSE_CAST_VALUES()
   
    void operator()(ast::FunctionDeclaration& declaration){
        //Add all the parameters to the function context
        for(auto& parameter : declaration.Content->parameters){
            Type type = visit(ast::TypeTransformer(), parameter.parameterType);
            
            declaration.Content->context->addParameter(parameter.parameterName, type);    
        }

        visit_each(*this, declaration.Content->instructions);
    }
    
    void operator()(ast::GlobalVariableDeclaration& declaration){
        if (declaration.Content->context->exists(declaration.Content->variableName)) {
            throw SemanticalException("The global Variable " + declaration.Content->variableName + " has already been declared", declaration.Content->position);
        }
    
        if(!visit(ast::IsConstantVisitor(), *declaration.Content->value)){
            throw SemanticalException("The value must be constant", declaration.Content->position);
        }
        
        declaration.Content->context->addVariable(declaration.Content->variableName, 
                newSimpleType(declaration.Content->variableType, declaration.Content->constant), *declaration.Content->value);
    }

    void operator()(ast::GlobalArrayDeclaration& declaration){
        if (declaration.Content->context->exists(declaration.Content->arrayName)) {
            throw SemanticalException("The global Variable " + declaration.Content->arrayName + " has already been declared", declaration.Content->position);
        }

        declaration.Content->context->addVariable(declaration.Content->arrayName, newArrayType(declaration.Content->arrayType, declaration.Content->arraySize));
    }
    
    void operator()(ast::Foreach& foreach){
        if(foreach.Content->context->exists(foreach.Content->variableName)){
            throw SemanticalException("The foreach variable " + foreach.Content->variableName  + " has already been declared", foreach.Content->position);
        }

        foreach.Content->context->addVariable(foreach.Content->variableName, newType(foreach.Content->variableType));

        visit_each(*this, foreach.Content->instructions);
    }
    
    void operator()(ast::ForeachIn& foreach){
        if(foreach.Content->context->exists(foreach.Content->variableName)){
            throw SemanticalException("The foreach variable " + foreach.Content->variableName  + " has already been declared", foreach.Content->position);
        }
        
        if(!foreach.Content->context->exists(foreach.Content->arrayName)){
            throw SemanticalException("The foreach array " + foreach.Content->arrayName  + " has not been declared", foreach.Content->position);
        }

        static int generated = 0;

        foreach.Content->var = foreach.Content->context->addVariable(foreach.Content->variableName, newType(foreach.Content->variableType));
        foreach.Content->arrayVar = foreach.Content->context->getVariable(foreach.Content->arrayName);
        foreach.Content->iterVar = foreach.Content->context->addVariable("foreach_iter_" + toString(++generated), newType("int"));

        visit_each(*this, foreach.Content->instructions);
    }

    template<typename A>
    void annotateAssignment(A& assignment){
        if (!assignment.Content->context->exists(assignment.Content->variableName)) {
            throw SemanticalException("Variable " + assignment.Content->variableName + " has not  been declared", assignment.Content->position);
        }

        visit(*this, assignment.Content->value);

        assignment.Content->context->getVariable(assignment.Content->variableName)->addReference();
    }

    void operator()(ast::Assignment& assignment){
        annotateAssignment(assignment);
    }
    
    void operator()(ast::CompoundAssignment& assignment){
        annotateAssignment(assignment);
    }

    template<typename Operation>
    void annotateSuffixOrPrefixOperation(Operation& operation){
        if (!operation.Content->context->exists(operation.Content->variableName)) {
            throw SemanticalException("Variable " + operation.Content->variableName + " has not  been declared", operation.Content->position);
        }

        operation.Content->variable = operation.Content->context->getVariable(operation.Content->variableName);
        operation.Content->variable->addReference();
    }
    
    void operator()(ast::SuffixOperation& operation){
        annotateSuffixOrPrefixOperation(operation);
    }
    
    void operator()(ast::PrefixOperation& operation){
        annotateSuffixOrPrefixOperation(operation);
    }

    void operator()(ast::Return& return_){
        visit(*this, return_.Content->value);
    }

    void operator()(ast::ArrayAssignment& assignment){
        if (!assignment.Content->context->exists(assignment.Content->variableName)) {
            throw SemanticalException("Array " + assignment.Content->variableName + " has not  been declared", assignment.Content->position);
        }

        visit(*this, assignment.Content->indexValue);
        visit(*this, assignment.Content->value);

        assignment.Content->context->getVariable(assignment.Content->variableName)->addReference();
    }
    
    void operator()(ast::VariableDeclaration& declaration){
        if (declaration.Content->context->exists(declaration.Content->variableName)) {
            throw SemanticalException("Variable " + declaration.Content->variableName + " has already been declared", declaration.Content->position);
        }
        
        visit(*this, *declaration.Content->value);

        Type type = newSimpleType(declaration.Content->variableType, declaration.Content->const_);

        if(type.isConst()){
            if(!visit(ast::IsConstantVisitor(), *declaration.Content->value)){
                throw SemanticalException("The value must be constant", declaration.Content->position);
            }
            
            declaration.Content->context->addVariable(declaration.Content->variableName, type, *declaration.Content->value);
        } else {
            declaration.Content->context->addVariable(declaration.Content->variableName, type);
        }
    }
    
    void operator()(ast::ArrayDeclaration& declaration){
        if (declaration.Content->context->exists(declaration.Content->arrayName)) {
            throw SemanticalException("The variable " + declaration.Content->arrayName + " has already been declared", declaration.Content->position);
        }

        Type type = newArrayType(declaration.Content->arrayType, declaration.Content->arraySize);
        declaration.Content->context->addVariable(declaration.Content->arrayName, type);
    }
    
    void operator()(ast::Swap& swap){
        if (swap.Content->lhs == swap.Content->rhs) {
            throw SemanticalException("Cannot swap a variable with itself", swap.Content->position);
        }

        if (!swap.Content->context->exists(swap.Content->lhs) || !swap.Content->context->exists(swap.Content->rhs)) {
            throw SemanticalException("Variable has not been declared in the swap", swap.Content->position);
        }

        swap.Content->lhs_var = swap.Content->context->getVariable(swap.Content->lhs);
        swap.Content->rhs_var = swap.Content->context->getVariable(swap.Content->rhs);

        //Reference both variables
        swap.Content->lhs_var->addReference();
        swap.Content->rhs_var->addReference();
    }

    void operator()(ast::VariableValue& variable){
        if (!variable.Content->context->exists(variable.Content->variableName)) {
            throw SemanticalException("Variable " + variable.Content->variableName + " has not been declared", variable.Content->position);
        }

        //Reference the variable
        variable.Content->var = variable.Content->context->getVariable(variable.Content->variableName);
        variable.Content->var->addReference();
    }

    void operator()(ast::ArrayValue& array){
        if (!array.Content->context->exists(array.Content->arrayName)) {
            throw SemanticalException("Array " + array.Content->arrayName + " has not been declared", array.Content->position);
        }
        
        //Reference the variable
        array.Content->var = array.Content->context->getVariable(array.Content->arrayName);
        array.Content->var->addReference();

        visit(*this, array.Content->indexValue);
    }

    void operator()(ast::Expression& value){
        visit(*this, value.Content->first);
        
        for_each(value.Content->operations.begin(), value.Content->operations.end(), 
            [&](ast::Operation& operation){ visit(*this, operation.get<1>()); });
    }

    void operator()(ast::Import&){
        //Nothing to check here
    }

    void operator()(ast::StandardImport&){
        //Nothing to check here
    }

    void operator()(ast::TerminalNode&){
        //Terminal nodes have no need for variable checking    
    }
};

void ast::defineVariables(ast::SourceFile& program){
    VariablesVisitor visitor;
    visit_non_variant(visitor, program);
}
