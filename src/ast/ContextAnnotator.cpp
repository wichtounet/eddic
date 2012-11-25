//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <memory>

#include "variant.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "BlockContext.hpp"
#include "VisitorUtils.hpp"

#include "ast/ContextAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

using namespace eddic;

//TODO This design is far from perfect, the Pass should be a static_visitor itself, but that 
//would mean exposing all the AST node in the header

namespace {

struct AnnotateVisitor : public boost::static_visitor<> {
        std::shared_ptr<GlobalContext> globalContext;
        std::shared_ptr<FunctionContext> functionContext;
        std::shared_ptr<Context> currentContext;

        AUTO_RECURSE_BUILTIN_OPERATORS()
        AUTO_RECURSE_TERNARY()
        AUTO_RECURSE_PREFIX()

        AUTO_IGNORE_FALSE()
        AUTO_IGNORE_TRUE()
        AUTO_IGNORE_NULL()
        AUTO_IGNORE_LITERAL()
        AUTO_IGNORE_CHAR_LITERAL()
        AUTO_IGNORE_FLOAT()
        AUTO_IGNORE_INTEGER()
        AUTO_IGNORE_INTEGER_SUFFIX()

        void operator()(ast::FunctionCall& functionCall){
            functionCall.Content->context = currentContext;

            visit_each(*this, functionCall.Content->values);
        }

        template<typename Loop>            
        void annotateWhileLoop(Loop& loop){
            currentContext = loop.Content->context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);
            
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

        void operator()(ast::Switch& switch_){
            switch_.Content->context = currentContext;

            visit(*this, switch_.Content->value);
            visit_each_non_variant(*this, switch_.Content->cases);
            visit_optional_non_variant(*this, switch_.Content->default_case);
        }
        
        void operator()(ast::SwitchCase& switch_case){
            visit(*this, switch_case.value);

            currentContext = switch_case.context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);
           
            visit_each(*this, switch_case.instructions);

            currentContext = currentContext->parent();
        }
        
        void operator()(ast::DefaultCase& default_case){
            currentContext = default_case.context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);
           
            visit_each(*this, default_case.instructions);

            currentContext = currentContext->parent();
        }

        void operator()(ast::For& for_){
            currentContext = for_.Content->context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);
          
            visit_optional(*this, for_.Content->start);
            visit_optional(*this, for_.Content->condition);
            visit_optional(*this, for_.Content->repeat);
            
            visit_each(*this, for_.Content->instructions);
            
            currentContext = currentContext->parent();
        }

        template<typename Loop>
        void annotateSimpleLoop(Loop& loop){
            loop.Content->context = currentContext = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);

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
            currentContext = if_.Content->context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);

            visit(*this, if_.Content->condition);
            
            visit_each(*this, if_.Content->instructions);
            
            currentContext = currentContext->parent();
            
            visit_each_non_variant(*this, if_.Content->elseIfs);
            visit_optional_non_variant(*this, if_.Content->else_);
        }

        void operator()(ast::ElseIf& elseIf){
            currentContext = elseIf.context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);

            visit(*this, elseIf.condition);
            
            visit_each(*this, elseIf.instructions);
            
            currentContext = currentContext->parent();
        }
        
        void operator()(ast::Else& else_){
            currentContext = else_.context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);
            
            visit_each(*this, else_.instructions);
            
            currentContext = currentContext->parent();
        }
        
        void operator()(ast::StructDeclaration& declaration){
            declaration.Content->context = currentContext;
            
            visit_each(*this, declaration.Content->values);
        }
        
        void operator()(ast::VariableDeclaration& declaration){
            declaration.Content->context = currentContext;
            
            visit_optional(*this, declaration.Content->value);
        }
        
        void operator()(ast::ArrayDeclaration& declaration){
            declaration.Content->context = currentContext;
            
            visit(*this, declaration.Content->size);
        }
        
        void operator()(ast::Assignment& assignment){
            assignment.Content->context = currentContext;

            visit(*this, assignment.Content->left_value);
            visit(*this, assignment.Content->value);
        }
        
        void operator()(ast::Delete& delete_){
            delete_.Content->context = currentContext;
        }
        
        void operator()(ast::Swap& swap){
            swap.Content->context = currentContext;
        }
        
        void operator()(ast::VariableValue& variable){
            variable.Content->context = currentContext;
        }
        
        void operator()(ast::Return& return_){
            return_.Content->context = functionContext;

            visit(*this, return_.Content->value);
        }
        
        void operator()(ast::Cast& cast){
            cast.Content->context = currentContext;

            visit(*this, cast.Content->value);
        }
        
        void operator()(ast::New& new_){
            new_.Content->context = currentContext;
            
            visit_each(*this, new_.Content->values);
        }
        
        void operator()(ast::NewArray& new_){
            new_.Content->context = currentContext;
            
            visit(*this, new_.Content->size);
        }
        
        void operator()(ast::Expression& expression){
            expression.Content->context = currentContext;

            VISIT_COMPOSED_VALUE(expression);
        }
};

inline AnnotateVisitor make_visitor(std::shared_ptr<GlobalContext> globalContext, std::shared_ptr<FunctionContext> functionContext, std::shared_ptr<Context> currentContext){
    AnnotateVisitor visitor;
    visitor.globalContext = globalContext;
    visitor.currentContext = currentContext;
    visitor.functionContext = functionContext;
    return visitor;
}

} //end of anonymous namespace

void ast::ContextAnnotationPass::apply_program(ast::SourceFile& program, bool indicator){
    if(indicator){
        currentContext = globalContext = program.Content->context;
    } else {
        currentContext = program.Content->context = globalContext = std::make_shared<GlobalContext>(platform);

        for(auto& block : program.Content->blocks){
            if(auto* ptr = boost::get<ast::GlobalVariableDeclaration>(&block)){
                ptr->Content->context = currentContext;
            } else if(auto* ptr = boost::get<ast::GlobalArrayDeclaration>(&block)){
                ptr->Content->context = currentContext;
            }
        }
    }
}

#define HANDLE_FUNCTION() \
    currentContext = function.Content->context = functionContext = std::make_shared<FunctionContext>(currentContext, globalContext, platform, configuration); \
    auto visitor = make_visitor(globalContext, functionContext, currentContext); \
    visit_each(visitor, function.Content->instructions); \
    currentContext = currentContext->parent();

void ast::ContextAnnotationPass::apply_function(ast::FunctionDeclaration& function){
    HANDLE_FUNCTION();
}

void ast::ContextAnnotationPass::apply_struct_function(ast::FunctionDeclaration& function){
    HANDLE_FUNCTION();
}

void ast::ContextAnnotationPass::apply_struct_constructor(ast::Constructor& function){
    HANDLE_FUNCTION();
}

void ast::ContextAnnotationPass::apply_struct_destructor(ast::Destructor& function){
    HANDLE_FUNCTION();
}
