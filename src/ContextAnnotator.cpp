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
        
        void operator()(ast::Program& program){
            currentContext = program.Content->context = globalContext = std::make_shared<GlobalContext>();

            visit_each(*this, program.Content->blocks);
        }

        void operator()(ast::GlobalVariableDeclaration& declaration){
            declaration.Content->context = currentContext;
        }
        
        void operator()(ast::GlobalArrayDeclaration& declaration){
            declaration.Content->context = currentContext;
        }

        void operator()(ast::FunctionDeclaration& function){
            currentContext = function.Content->context = functionContext = std::make_shared<FunctionContext>(currentContext);

            visit_each(*this, function.Content->instructions);
    
            currentContext = currentContext->parent();
        }

        void operator()(ast::While& while_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
            
            visit(*this, while_.Content->condition);

            visit_each(*this, while_.Content->instructions);
            
            currentContext = currentContext->parent();
        }

        void operator()(ast::For& for_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
          
            visit_optional(*this, for_.Content->start);
            visit_optional(*this, for_.Content->condition);
            visit_optional(*this, for_.Content->repeat);
            
            visit_each(*this, for_.Content->instructions);
            
            currentContext = currentContext->parent();
        }

        void operator()(ast::Foreach& foreach){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);

            foreach.Content->context = currentContext;
            
            visit_each(*this, foreach.Content->instructions);
             
            currentContext = currentContext->parent();
        }
        
        void operator()(ast::ForeachIn& foreach){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);

            foreach.Content->context = currentContext;
            
            visit_each(*this, foreach.Content->instructions);
             
            currentContext = currentContext->parent();
        }

        void operator()(ast::If& if_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);

            visit(*this, if_.Content->condition);
            
            visit_each(*this, if_.Content->instructions);
            
            currentContext = currentContext->parent();
            
            visit_each_non_variant(*this, if_.Content->elseIfs);
            visit_optional_non_variant(*this, if_.Content->else_);
        }

        void operator()(ast::ElseIf& elseIf){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
           
            visit(*this, elseIf.condition);
            
            visit_each(*this, elseIf.instructions);
            
            currentContext = currentContext->parent();
        }
        
        void operator()(ast::Else& else_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
            
            visit_each(*this, else_.instructions);
            
            currentContext = currentContext->parent();
        }
        
        void operator()(ast::VariableDeclaration& declaration){
            declaration.Content->context = currentContext;
            
            visit(*this, *declaration.Content->value);
        }
        
        void operator()(ast::ArrayDeclaration& declaration){
            declaration.Content->context = currentContext;
        }
        
        void operator()(ast::Assignment& assignment){
            assignment.Content->context = currentContext;

            visit(*this, assignment.Content->value);
        }
        
        void operator()(ast::ArrayAssignment& assignment){
            assignment.Content->context = currentContext;

            visit(*this, assignment.Content->indexValue);
            visit(*this, assignment.Content->value);
        }
        
        void operator()(ast::Swap& swap){
            swap.Content->context = currentContext;
        }
        
        void operator()(ast::VariableValue& variable){
            variable.Content->context = currentContext;
        }
        
        void operator()(ast::ArrayValue& array){
            array.Content->context = currentContext;

            visit(*this, array.Content->indexValue);
        }
        
        void operator()(ast::TerminalNode&){
            //A terminal node has no context
        }
};

void ContextAnnotator::annotate(ast::Program& program){
    AnnotateVisitor visitor;
    visitor(program);
}
