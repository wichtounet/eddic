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

#include "IsConstantVisitor.hpp"
#include "GetTypeVisitor.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "Types.hpp"
#include "VisitorUtils.hpp"

#include "ast/Program.hpp"

#include "ASTVisitor.hpp"

using namespace eddic;

struct CheckerVisitor : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_DECLARATION() 
    AUTO_RECURSE_FUNCTION_CALLS()
    AUTO_RECURSE_COMPOSED_VALUES()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
    
    void operator()(GlobalVariableDeclaration& declaration){
        if (declaration.context->exists(declaration.variableName)) {
            throw SemanticalException("Variable has already been declared");
        }
    
        if(!boost::apply_visitor(IsConstantVisitor(), declaration.value)){
            throw SemanticalException("The value must be constant");
        }

        Type type = stringToType(declaration.variableType); 

        declaration.context->addVariable(declaration.variableName, type, declaration.value);

        Type valueType = boost::apply_visitor(GetTypeVisitor(), declaration.value);
        if (valueType != type) {
            throw SemanticalException("Incompatible type");
        }
    }
    
    void operator()(ASTForeach& for_){
        //TODO

        visit_each(*this, for_.instructions);
    }

    void operator()(ASTAssignment& assignment){
        //TODO
    }
    
    void operator()(ASTDeclaration& declaration){
        //TODO
    }
    
    void operator()(ASTSwap& swap){
        //TODO
    }

    void operator()(ASTVariable& variable){
        //TODO
    }

    void operator()(TerminalNode&){
        //Terminal nodes have no need for variable checking    
    }
   
};

void VariableChecker::check(ASTProgram& program){
   CheckerVisitor visitor;
   visitor(program); 
}
