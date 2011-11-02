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

using namespace eddic;

struct CheckerVisitor : public boost::static_visitor<> {
    void operator()(ASTProgram& program){
        visit_each(*this, program.blocks);
    }

    void operator()(ASTFunctionDeclaration& function){
        //TODO visit_each(*this, function.instructions);
    }

    void operator()(GlobalVariableDeclaration& declaration){
        if (declaration.context->exists(declaration.variableName)) {
            throw SemanticalException("Variable has already been declared");
        }
    
        //TODO visit(*this, value);
         
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
};

void VariableChecker::check(ASTProgram& program){
   CheckerVisitor visitor;
   visitor(program); 
}
