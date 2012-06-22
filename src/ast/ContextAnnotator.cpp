//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <memory>
#include <boost/variant/variant.hpp>

#include "ast/ContextAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

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
        AUTO_RECURSE_BINARY_CONDITION()
        AUTO_RECURSE_FUNCTION_CALLS()
        AUTO_RECURSE_BUILTIN_OPERATORS()
        AUTO_RECURSE_MINUS_PLUS_VALUES()
        AUTO_RECURSE_CAST_VALUES()

        AUTO_IGNORE_FALSE()
        AUTO_IGNORE_TRUE()
        AUTO_IGNORE_NULL()
        AUTO_IGNORE_LITERAL()
        AUTO_IGNORE_FLOAT()
        AUTO_IGNORE_INTEGER()
        AUTO_IGNORE_INTEGER_SUFFIX()
        AUTO_IGNORE_IMPORT()
        AUTO_IGNORE_STANDARD_IMPORT()
        AUTO_IGNORE_STRUCT()
        
        void operator()(ast::SourceFile& program){
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
            
        template<typename Loop>            
        void annotateWhileLoop(Loop& loop){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
            
            visit(*this, loop.Content->condition);

            visit_each(*this, loop.Content->instructions);
            
            currentContext = currentContext->parent();
        }

        void operator()(ast::While& while_){
            annotateWhileLoop(while_);
        }

        void operator()(ast::DoWhile& while_){
            annotateWhileLoop(while_);
        }

        void operator()(ast::For& for_){
            currentContext = std::make_shared<BlockContext>(currentContext, functionContext);
          
            visit_optional(*this, for_.Content->start);
            visit_optional(*this, for_.Content->condition);
            visit_optional(*this, for_.Content->repeat);
            
            visit_each(*this, for_.Content->instructions);
            
            currentContext = currentContext->parent();
        }

        template<typename Loop>
        void annotateSimpleLoop(Loop& loop){
            loop.Content->context = currentContext = std::make_shared<BlockContext>(currentContext, functionContext);

            visit_each(*this, loop.Content->instructions);
             
            currentContext = currentContext->parent();
        }

        void operator()(ast::Foreach& foreach){
            annotateSimpleLoop(foreach);
        }
        
        void operator()(ast::ForeachIn& foreach){
            annotateSimpleLoop(foreach);
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
            
            visit_optional(*this, declaration.Content->value);
        }
        
        void operator()(ast::ArrayDeclaration& declaration){
            declaration.Content->context = currentContext;
        }
        
        void operator()(ast::Assignment& assignment){
            visit(*this, assignment.Content->left_value);
            visit(*this, assignment.Content->value);
        }
        
        void operator()(ast::Swap& swap){
            swap.Content->context = currentContext;
        }
        
        void operator()(ast::SuffixOperation& operation){
            operation.Content->context = currentContext;
        }
        
        void operator()(ast::PrefixOperation& operation){
            operation.Content->context = currentContext;
        }

        void operator()(ast::Expression& value){
            value.Content->context = currentContext;

            visit(*this, value.Content->first);
            for_each(value.Content->operations.begin(), value.Content->operations.end(), 
                    [&](ast::Operation& operation){ visit(*this, operation.get<1>()); });
        }
        
        void operator()(ast::VariableValue& variable){
            variable.Content->context = currentContext;
        }
        
        void operator()(ast::DereferenceVariableValue& variable){
            variable.Content->context = currentContext;
        }
        
        void operator()(ast::ArrayValue& array){
            array.Content->context = currentContext;

            visit(*this, array.Content->indexValue);
        }
       
        void operator()(ast::Return& return_){
            return_.Content->context = functionContext;

            visit(*this, return_.Content->value);
        }
};

void ast::defineContexts(ast::SourceFile& program){
    AnnotateVisitor visitor;
    visitor(program);
}
