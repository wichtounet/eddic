//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <boost/variant/variant.hpp>

#include "ContextAnnotator.hpp"

#include "ast/Program.hpp"

#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "BlockContext.hpp"

using namespace eddic;

class AnnotateVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<GlobalContext> globalContext;
        std::shared_ptr<FunctionContext> functionContext;
        std::shared_ptr<Context> currentContext;

    public:
        void operator()(ASTProgram& program){
            currentContext = globalContext = std::make_shared<GlobalContext>();

            program.context = currentContext;

            for(auto& block : program.blocks){
                boost::apply_visitor(*this, block);
            }
        }

        void operator()(FunctionDeclaration& function){
            currentContext = functionContext = std::make_shared<FunctionContext>(currentContext);

            function.context = currentContext;

            for(auto& instruction : function.instructions){
                boost::apply_visitor(*this, instruction);
            }
    
            currentContext = currentContext->parent();
        }

        void operator()(ASTWhile& while_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
            
            boost::apply_visitor(*this, while_.condition);
            
            for(auto& instruction : while_.instructions){
                boost::apply_visitor(*this, instruction);
            }
            
            currentContext = currentContext->parent();
        }

        void operator()(ASTIf& if_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);

            boost::apply_visitor(*this, if_.condition);
            
            for(auto& instruction : if_.instructions){
                boost::apply_visitor(*this, instruction);
            }
            
            for(auto& elseIf : if_.elseIfs){
                (*this)(elseIf);
            }

            if(if_.else_){
                (*this)(*if_.else_);
            }

            currentContext = currentContext->parent();
        }

        void operator()(ASTElseIf& elseIf){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
           
            boost::apply_visitor(*this, elseIf.condition);
            
            for(auto& instruction : elseIf.instructions){
                boost::apply_visitor(*this, instruction);
            }
            
            currentContext = currentContext->parent();
        }
        
        void operator()(ASTElse& else_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
           
            for(auto& instruction : else_.instructions){
                boost::apply_visitor(*this, instruction);
            }
            
            currentContext = currentContext->parent();
        }

        void operator()(ASTBinaryCondition& binaryCondition){
            boost::apply_visitor(*this, binaryCondition);
        }

        void operator()(Node& node){
            node.context = currentContext;
        }
        
        //TODO : Make some AST Nodes a ConstantNode with no context
        void operator()(ASTFalse& false_){
            //Nothing
        }

        void operator()(ASTTrue& true_){
            //Nothing
        }
};

void ContextAnnotator::annotate(ASTProgram& program){
    AnnotateVisitor visitor;
    visitor(program);
}
