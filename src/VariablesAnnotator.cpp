//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include <memory>
#include <boost/variant/variant.hpp>

#include "VariablesAnnotator.hpp"

#include "ast/Program.hpp"

#include "IsConstantVisitor.hpp"
#include "GetTypeVisitor.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Types.hpp"
#include "Variable.hpp"

#include "Compiler.hpp"
#include "Options.hpp"
#include "TypeTransformer.hpp"

#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

using namespace eddic;

struct VariablesVisitor : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_CALLS()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
   
    void operator()(ast::FunctionDeclaration& declaration){
        //Add all the parameters to the function context
        for(auto& parameter : declaration.Content->parameters){
            Type type = boost::apply_visitor(TypeTransformer(), parameter.parameterType);
            
            declaration.Content->context->addParameter(parameter.parameterName, type);    
        }

        visit_each(*this, declaration.Content->instructions);
    }
    
    void operator()(ast::GlobalVariableDeclaration& declaration){
        if (declaration.Content->context->exists(declaration.Content->variableName)) {
            throw SemanticalException("The global Variable " + declaration.Content->variableName + " has already been declared");
        }
    
        if(!boost::apply_visitor(IsConstantVisitor(), *declaration.Content->value)){
            throw SemanticalException("The value must be constant");
        }

        Type type = stringToType(declaration.Content->variableType); 
        declaration.Content->context->addVariable(declaration.Content->variableName, type, *declaration.Content->value);
    }

    void operator()(ast::GlobalArrayDeclaration& declaration){
        if (declaration.Content->context->exists(declaration.Content->arrayName)) {
            throw SemanticalException("The global Variable " + declaration.Content->arrayName + " has already been declared");
        }

        BaseType baseType = stringToBaseType(declaration.Content->arrayType); 
        Type type(baseType, declaration.Content->arraySize);

        declaration.Content->context->addVariable(declaration.Content->arrayName, type);
    }
    
    void operator()(ast::Foreach& foreach){
        if(foreach.Content->context->exists(foreach.Content->variableName)){
            throw SemanticalException("The foreach variable " + foreach.Content->variableName  + " has already been declared");
        }

        foreach.Content->context->addVariable(foreach.Content->variableName, stringToType(foreach.Content->variableType));

        visit_each(*this, foreach.Content->instructions);
    }
    
    void operator()(ast::ForeachIn& foreach){
        if(foreach.Content->context->exists(foreach.Content->variableName)){
            throw SemanticalException("The foreach variable " + foreach.Content->variableName  + " has already been declared");
        }
        
        if(!foreach.Content->context->exists(foreach.Content->arrayName)){
            throw SemanticalException("The foreach array " + foreach.Content->arrayName  + " has not been declared");
        }

        static int generated = 0;

        foreach.Content->var = foreach.Content->context->addVariable(foreach.Content->variableName, stringToType(foreach.Content->variableType));
        foreach.Content->arrayVar = foreach.Content->context->getVariable(foreach.Content->arrayName);
        foreach.Content->iterVar = foreach.Content->context->addVariable("foreach_iter" + ++generated, stringToType("int"));

        visit_each(*this, foreach.Content->instructions);
    }

    void operator()(ast::Assignment& assignment){
        if (!assignment.Content->context->exists(assignment.Content->variableName)) {
            throw SemanticalException("Variable " + assignment.Content->variableName + " has not  been declared");
        }

        visit(*this, assignment.Content->value);

        assignment.Content->context->getVariable(assignment.Content->variableName)->addReference();
    }

    void operator()(ast::Return& return_){
        visit(*this, return_.Content->value);
    }

    void operator()(ast::ArrayAssignment& assignment){
        if (!assignment.Content->context->exists(assignment.Content->variableName)) {
            throw SemanticalException("Array " + assignment.Content->variableName + " has not  been declared");
        }

        visit(*this, assignment.Content->indexValue);
        visit(*this, assignment.Content->value);

        assignment.Content->context->getVariable(assignment.Content->variableName)->addReference();
    }
    
    void operator()(ast::VariableDeclaration& declaration){
        if (declaration.Content->context->exists(declaration.Content->variableName)) {
            throw SemanticalException("Variable " + declaration.Content->variableName + " has already been declared");
        }

        Type variableType = stringToType(declaration.Content->variableType);
        declaration.Content->context->addVariable(declaration.Content->variableName, variableType);

        visit(*this, *declaration.Content->value);
    }
    
    void operator()(ast::ArrayDeclaration& declaration){
        if (declaration.Content->context->exists(declaration.Content->arrayName)) {
            throw SemanticalException("The variable " + declaration.Content->arrayName + " has already been declared");
        }

        BaseType baseType = stringToBaseType(declaration.Content->arrayType); 
        Type type(baseType, declaration.Content->arraySize);

        declaration.Content->context->addVariable(declaration.Content->arrayName, type);
    }
    
    void operator()(ast::Swap& swap){
        if (swap.Content->lhs == swap.Content->rhs) {
            throw SemanticalException("Cannot swap a variable with itself");
        }

        if (!swap.Content->context->exists(swap.Content->lhs) || !swap.Content->context->exists(swap.Content->rhs)) {
            throw SemanticalException("Variable has not been declared in the swap");
        }

        swap.Content->lhs_var = swap.Content->context->getVariable(swap.Content->lhs);
        swap.Content->rhs_var = swap.Content->context->getVariable(swap.Content->rhs);

        //Reference both variables
        swap.Content->lhs_var->addReference();
        swap.Content->rhs_var->addReference();
    }

    void operator()(ast::VariableValue& variable){
        if (!variable.Content->context->exists(variable.Content->variableName)) {
            throw SemanticalException("Variable " + variable.Content->variableName + " has not been declared");
        }

        //Reference the variable
        variable.Content->var = variable.Content->context->getVariable(variable.Content->variableName);
        variable.Content->var->addReference();
    }

    void operator()(ast::ArrayValue& array){
        if (!array.Content->context->exists(array.Content->arrayName)) {
            throw SemanticalException("Array " + array.Content->arrayName + " has not been declared");
        }
        
        //Reference the variable
        array.Content->var = array.Content->context->getVariable(array.Content->arrayName);
        array.Content->var->addReference();

        visit(*this, array.Content->indexValue);
    }

    void operator()(ast::ComposedValue& value){
        visit(*this, value.Content->first);
        
        for_each(value.Content->operations.begin(), value.Content->operations.end(), 
            [&](boost::tuple<char, ast::Value>& operation){ visit(*this, operation.get<1>()); });
    }

    void operator()(ast::TerminalNode&){
        //Terminal nodes have no need for variable checking    
    }
};

void VariablesAnnotator::annotate(ast::Program& program){
    VariablesVisitor visitor;
    visit_non_variant(visitor, program);
}
