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

            for_each(program.blocks.begin(), program.blocks.end(), 
                [&](FirstLevelBlock& block){ boost::apply_visitor(*this, block); });
        }

        void operator()(ASTFunctionDeclaration& function){
            currentContext = functionContext = std::make_shared<FunctionContext>(currentContext);

            function.context = currentContext;

            for_each(function.instructions.begin(), function.instructions.end(), 
                [&](ASTInstruction& instruction){ boost::apply_visitor(*this, instruction); });
    
            currentContext = currentContext->parent();
        }

        void operator()(ASTWhile& while_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
            
            boost::apply_visitor(*this, while_.condition);
            
            for_each(while_.instructions.begin(), while_.instructions.end(), 
                [&](ASTInstruction& instruction){ boost::apply_visitor(*this, instruction); });
            
            currentContext = currentContext->parent();
        }

        void operator()(ASTFor& for_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
          
            visit(for_.start);
            visit(for_.condition);
            visit(for_.repeat);
            
            for_each(for_.instructions.begin(), for_.instructions.end(), 
                [&](ASTInstruction& instruction){ boost::apply_visitor(*this, instruction); });
            
            currentContext = currentContext->parent();
        }

        void operator()(ASTForeach& foreach){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);

            foreach.context = currentContext;
            
            for_each(foreach.instructions.begin(), foreach.instructions.end(), 
                [&](ASTInstruction& instruction){ boost::apply_visitor(*this, instruction); });
             
            currentContext = currentContext->parent();
        }

        void operator()(ASTIf& if_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);

            boost::apply_visitor(*this, if_.condition);
            
            for_each(if_.instructions.begin(), if_.instructions.end(), 
                [&](ASTInstruction& instruction){ boost::apply_visitor(*this, instruction); });
            
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
            
            for_each(elseIf.instructions.begin(), elseIf.instructions.end(), 
                [&](ASTInstruction& instruction){ boost::apply_visitor(*this, instruction); });
            
            currentContext = currentContext->parent();
        }
        
        void operator()(ASTElse& else_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
           
            for_each(else_.instructions.begin(), else_.instructions.end(), 
                [&](ASTInstruction& instruction){ boost::apply_visitor(*this, instruction); });
            
            currentContext = currentContext->parent();
        }

        void operator()(ASTFunctionCall& functionCall){
            for_each(functionCall.values.begin(), functionCall.values.end(), 
                [&](ASTValue& value){ boost::apply_visitor(*this, value); });
        }
        
        void operator()(ASTDeclaration& declaration){
            declaration.context = currentContext;

            boost::apply_visitor(*this, declaration.value);
        }
        
        void operator()(ASTAssignment& assignment){
            assignment.context = currentContext;

            boost::apply_visitor(*this, assignment.value);
        }
        
        void operator()(ASTComposedValue& value){
            boost::apply_visitor(*this, value.first);
            
            for_each(value.operations.begin(), value.operations.end(), 
                [&](boost::tuple<char, ASTValue>& operation){ boost::apply_visitor(*this, operation.get<1>()); });
        }

        void operator()(ASTBinaryCondition& binaryCondition){
            boost::apply_visitor(*this, binaryCondition);
        }

        //Find a way to simplify the 6 following operators
        void operator()(ASTEquals& equals){
            boost::apply_visitor(*this, equals.lhs);
            boost::apply_visitor(*this, equals.rhs);
        }

        void operator()(ASTNotEquals& notEquals){
            boost::apply_visitor(*this, notEquals.lhs);
            boost::apply_visitor(*this, notEquals.rhs);
        }

        void operator()(ASTLess& less){
            boost::apply_visitor(*this, less.lhs);
            boost::apply_visitor(*this, less.rhs);
        }

        void operator()(ASTLessEquals& less){
            boost::apply_visitor(*this, less.lhs);
            boost::apply_visitor(*this, less.rhs);
        }

        void operator()(ASTGreater& greater){
            boost::apply_visitor(*this, greater.lhs);
            boost::apply_visitor(*this, greater.rhs);
        }

        void operator()(ASTGreaterEquals& greater){
            boost::apply_visitor(*this, greater.lhs);
            boost::apply_visitor(*this, greater.rhs);
        }

        void operator()(Node& node){
            node.context = currentContext;
        }
        
        void operator()(TerminalNode&){
            //A terminal node has no context
        }
      
        //Utility operators
        template<typename T>
        void visit(boost::optional<T> optional){
            if(optional){
                boost::apply_visitor(*this, *optional);
            }
        }
};

void ContextAnnotator::annotate(ASTProgram& program){
    AnnotateVisitor visitor;
    visitor(program);
}
