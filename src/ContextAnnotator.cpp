//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include <memory>

#include <boost/variant/variant.hpp>

#include "ContextAnnotator.hpp"

#include "ast/Program.hpp"

#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "BlockContext.hpp"

#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

using namespace eddic;

class AnnotateVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<GlobalContext> globalContext;
        std::shared_ptr<FunctionContext> functionContext;
        std::shared_ptr<Context> currentContext;

    public:
        AUTO_RECURSE_BINARY_CONDITION()
        AUTO_RECURSE_FUNCTION_CALLS()
        AUTO_RECURSE_COMPOSED_VALUES()
        
        void operator()(ASTProgram& program){
            currentContext = globalContext = std::make_shared<GlobalContext>();

            visit_each(*this, program.blocks);
        }

        void operator()(ASTFunctionDeclaration& function){
            function.context = currentContext = functionContext = std::make_shared<FunctionContext>(currentContext);

            visit_each(*this, function.instructions);
    
            currentContext = currentContext->parent();
        }

        void operator()(ASTWhile& while_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
            
            visit(*this, while_.condition);

            visit_each(*this, while_.instructions);
            
            currentContext = currentContext->parent();
        }

        void operator()(ASTFor& for_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
          
            visit_optional(*this, for_.start);
            visit_optional(*this, for_.condition);
            visit_optional(*this, for_.repeat);
            
            visit_each(*this, for_.instructions);
            
            currentContext = currentContext->parent();
        }

        void operator()(ASTForeach& foreach){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);

            foreach.context = currentContext;
            
            visit_each(*this, foreach.instructions);
             
            currentContext = currentContext->parent();
        }

        void operator()(ASTIf& if_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);

            visit(*this, if_.condition);
            
            visit_each(*this, if_.instructions);
            visit_each_non_variant(*this, if_.elseIfs);
            visit_optional_non_variant(*this, if_.else_);

            currentContext = currentContext->parent();
        }

        void operator()(ASTElseIf& elseIf){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
           
            visit(*this, elseIf.condition);
            
            visit_each(*this, elseIf.instructions);
            
            currentContext = currentContext->parent();
        }
        
        void operator()(ASTElse& else_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
           
            visit_each(*this, else_.instructions);
            
            currentContext = currentContext->parent();
        }
        
        void operator()(ASTDeclaration& declaration){
            declaration.context = currentContext;

            visit(*this, declaration.value);
        }
        
        void operator()(ASTAssignment& assignment){
            assignment.context = currentContext;

            visit(*this, assignment.value);
        }
        
        void operator()(Node& node){
            node.context = currentContext;
        }
        
        void operator()(TerminalNode&){
            //A terminal node has no context
        }
};

void ContextAnnotator::annotate(ASTProgram& program){
    AnnotateVisitor visitor;
    visitor(program);
}
