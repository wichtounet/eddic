//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include <memory>
#include <boost/variant/variant.hpp>

#include "VariableChecker.hpp"

#include "ast/Program.hpp"

#include "IsConstantVisitor.hpp"
#include "GetTypeVisitor.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "FunctionContext.hpp"
#include "Types.hpp"
#include "Variable.hpp"

#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

using namespace eddic;

struct CheckerVisitor : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_CALLS()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
   
    void operator()(ASTFunctionDeclaration& declaration){
        //Add all the parameters to the function context
        for(auto& parameter : declaration.parameters){
            Type type = stringToType(parameter.parameterType);
            
            declaration.context->addParameter(parameter.parameterName, type);    
        }

        visit_each(*this, declaration.instructions);
    }
    
    void operator()(GlobalVariableDeclaration& declaration){
        if (declaration.context->exists(declaration.variableName)) {
            throw SemanticalException("The global Variable " + declaration.variableName + " has already been declared");
        }
    
        if(!boost::apply_visitor(IsConstantVisitor(), declaration.value)){
            throw SemanticalException("The value must be constant");
        }

        Type type = stringToType(declaration.variableType); 

        declaration.context->addVariable(declaration.variableName, type, declaration.value);

        Type valueType = boost::apply_visitor(GetTypeVisitor(), declaration.value);
        if (valueType != type) {
            throw SemanticalException("Incompatible type for global variable " + declaration.variableName);
        }
    }
    
    void operator()(ASTForeach& foreach){
        if(foreach.context->exists(foreach.variableName)){
            throw SemanticalException("The foreach variable " + foreach.variableName  + " has already been declared");
        }

        foreach.context->addVariable(foreach.variableName, stringToType(foreach.variableType));

        visit_each(*this, foreach.instructions);
    }

    void operator()(ASTAssignment& assignment){
        if (!assignment.context->exists(assignment.variableName)) {
            throw SemanticalException("Variable " + assignment.variableName + " has not  been declared");
        }

        visit(*this, assignment.value);

        std::shared_ptr<Variable> var = assignment.context->getVariable(assignment.variableName);

        Type valueType = boost::apply_visitor(GetTypeVisitor(), assignment.value);
        if (valueType != var->type()) {
            throw SemanticalException("Incompatible type in assignment of variable " + assignment.variableName);
        }
    }
    
    void operator()(ASTDeclaration& declaration){
        if (declaration.context->exists(declaration.variableName)) {
            throw SemanticalException("Variable " + declaration.variableName + " has already been declared");
        }

        Type variableType = stringToType(declaration.variableType);
        declaration.context->addVariable(declaration.variableName, variableType);

        visit(*this, declaration.value);

        Type valueType = boost::apply_visitor(GetTypeVisitor(), declaration.value);
        if (valueType != variableType) {
            throw SemanticalException("Incompatible type in declaration of variable " + declaration.variableName);
        }
    }
    
    void operator()(ASTSwap& swap){
        if (swap.lhs == swap.rhs) {
            throw SemanticalException("Cannot swap a variable with itself");
        }

        if (!swap.context->exists(swap.lhs) || !swap.context->exists(swap.rhs)) {
            throw SemanticalException("Variable has not been declared in the swap");
        }

        swap.lhs_var = swap.context->getVariable(swap.lhs);
        swap.rhs_var = swap.context->getVariable(swap.rhs);

        if (swap.lhs_var->type() != swap.rhs_var->type()) {
            throw SemanticalException("Swap of variables of incompatible type");
        }
    }

    void operator()(ASTVariable& variable){
        if (!variable.context->exists(variable.variableName)) {
            throw SemanticalException("Variable " + variable.variableName + " has not been declared");
        }

        variable.var = variable.context->getVariable(variable.variableName);
    }

    void operator()(ASTComposedValue& value){
        visit(*this, value.first);
        
        for_each(value.operations.begin(), value.operations.end(), 
            [&](boost::tuple<char, ASTValue>& operation){ visit(*this, operation.get<1>()); });

        GetTypeVisitor visitor;

        Type type = boost::apply_visitor(visitor, value.first);

        for(auto& operation : value.operations){
            Type operationType = boost::apply_visitor(visitor, operation.get<1>());

            if(type != operationType){
                throw SemanticalException("Incompatible type");
            }
        }
    }

    void operator()(TerminalNode&){
        //Terminal nodes have no need for variable checking    
    }
   
};

void VariableChecker::check(ASTProgram& program){
   CheckerVisitor visitor;
   visitor(program); 
}
