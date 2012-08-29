//=======================================================================
// Copyright Baptiste Wicht 2011.
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

namespace {

class AnnotateVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<GlobalContext> globalContext;
        std::shared_ptr<FunctionContext> functionContext;
        std::shared_ptr<Context> currentContext;

    public:
        AUTO_RECURSE_UNMARKED_STRUCT()
        AUTO_RECURSE_BINARY_CONDITION()
        AUTO_RECURSE_FUNCTION_CALLS()
        AUTO_RECURSE_BUILTIN_OPERATORS()
        AUTO_RECURSE_UNARY_VALUES()
        AUTO_RECURSE_TERNARY()
        AUTO_RECURSE_PREFIX()
        AUTO_RECURSE_SUFFIX()

        AUTO_IGNORE_TEMPLATE_STRUCT()
        AUTO_IGNORE_TEMPLATE_FUNCTION_DECLARATION()
        AUTO_IGNORE_FALSE()
        AUTO_IGNORE_TRUE()
        AUTO_IGNORE_NULL()
        AUTO_IGNORE_LITERAL()
        AUTO_IGNORE_CHAR_LITERAL()
        AUTO_IGNORE_FLOAT()
        AUTO_IGNORE_INTEGER()
        AUTO_IGNORE_INTEGER_SUFFIX()
        AUTO_IGNORE_IMPORT()
        AUTO_IGNORE_STANDARD_IMPORT()
        
        void operator()(ast::SourceFile& program){
            if(program.Content->context){
                currentContext = globalContext = program.Content->context;
            } else {
                currentContext = program.Content->context = globalContext = std::make_shared<GlobalContext>();
            }

            visit_each(*this, program.Content->blocks);
        }
        
        void operator()(ast::MemberFunctionCall& functionCall){
            functionCall.Content->context = currentContext;

            visit_each(*this, functionCall.Content->values);
        }

        void operator()(ast::GlobalVariableDeclaration& declaration){
            declaration.Content->context = currentContext;
        }
        
        void operator()(ast::GlobalArrayDeclaration& declaration){
            declaration.Content->context = currentContext;
            
            visit(*this, declaration.Content->size);
        }

        void operator()(ast::Constructor& constructor){
            currentContext = constructor.Content->context = functionContext = std::make_shared<FunctionContext>(currentContext, globalContext);

            visit_each(*this, constructor.Content->instructions);
    
            currentContext = currentContext->parent();
        }

        void operator()(ast::Destructor& destructor){
            currentContext = destructor.Content->context = functionContext = std::make_shared<FunctionContext>(currentContext, globalContext);

            visit_each(*this, destructor.Content->instructions);
    
            currentContext = currentContext->parent();
        }

        void operator()(ast::FunctionDeclaration& function){
            if(!function.Content->marked){
                currentContext = function.Content->context = functionContext = std::make_shared<FunctionContext>(currentContext, globalContext);

                visit_each(*this, function.Content->instructions);

                currentContext = currentContext->parent();
            }
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
            visit(*this, assignment.Content->left_value);
            visit(*this, assignment.Content->value);
        }
        
        void operator()(ast::Delete& delete_){
            delete_.Content->context = currentContext;
        }
        
        void operator()(ast::Swap& swap){
            swap.Content->context = currentContext;
        }

        void operator()(ast::Expression& value){
            visit(*this, value.Content->first);
            for_each(value.Content->operations.begin(), value.Content->operations.end(), 
                    [&](ast::Operation& operation){ visit(*this, operation.get<1>()); });
        }
        
        void operator()(ast::VariableValue& variable){
            variable.Content->context = currentContext;
        }
        
        void operator()(ast::DereferenceValue& variable){
            visit(*this, variable.Content->ref);
        }
        
        void operator()(ast::MemberValue& value){
            value.Content->context = currentContext;

            visit(*this, value.Content->location);
        }
        
        void operator()(ast::ArrayValue& array){
            array.Content->context = currentContext;

            visit(*this, array.Content->indexValue);
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
};

} //end of anonymous namespace

void ast::defineContexts(ast::SourceFile& program){
    AnnotateVisitor visitor;
    visitor(program);
}
