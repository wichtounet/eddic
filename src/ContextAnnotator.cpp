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

using namespace eddic;

class AnnotateVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<GlobalContext> globalContext;
        std::shared_ptr<FunctionContext> functionContext;
        std::shared_ptr<Context> currentContext;

    public:
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

        void operator()(ASTFunctionCall& functionCall){
            visit_each(*this, functionCall.values);
        }
        
        void operator()(ASTDeclaration& declaration){
            declaration.context = currentContext;

            visit(*this, declaration.value);
        }
        
        void operator()(ASTAssignment& assignment){
            assignment.context = currentContext;

            visit(*this, assignment.value);
        }
        
        void operator()(ASTComposedValue& value){
            visit(*this, value.first);
            
            for_each(value.operations.begin(), value.operations.end(), 
                [&](boost::tuple<char, ASTValue>& operation){ visit(*this, operation.get<1>()); });
        }

        void operator()(ASTBinaryCondition& binaryCondition){
            visit(*this, binaryCondition);
        }

        //Find a way to simplify the 6 following operators
        void operator()(ASTEquals& equals){
            visit(*this, equals.lhs);
            visit(*this, equals.rhs);
        }

        void operator()(ASTNotEquals& notEquals){
            visit(*this, notEquals.lhs);
            visit(*this, notEquals.rhs);
        }

        void operator()(ASTLess& less){
            visit(*this, less.lhs);
            visit(*this, less.rhs);
        }

        void operator()(ASTLessEquals& less){
            visit(*this, less.lhs);
            visit(*this, less.rhs);
        }

        void operator()(ASTGreater& greater){
            visit(*this, greater.lhs);
            visit(*this, greater.rhs);
        }

        void operator()(ASTGreaterEquals& greater){
            visit(*this, greater.lhs);
            visit(*this, greater.rhs);
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
