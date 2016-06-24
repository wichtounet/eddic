//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
        AUTO_RECURSE_DELETE()
        AUTO_RECURSE_SCOPE()

        AUTO_IGNORE_BOOLEAN()
        AUTO_IGNORE_NULL()
        AUTO_IGNORE_LITERAL()
        AUTO_IGNORE_CHAR_LITERAL()
        AUTO_IGNORE_FLOAT()
        AUTO_IGNORE_INTEGER()
        AUTO_IGNORE_INTEGER_SUFFIX()

        void operator()(ast::FunctionCall& functionCall){
            functionCall.context = currentContext;

            visit_each(*this, functionCall.values);
        }

        template<typename Loop>
        void annotateWhileLoop(Loop& loop){
            currentContext = loop.context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);

            visit(*this, loop.condition);

            visit_each(*this, loop.instructions);

            currentContext = currentContext->parent();
        }

        void operator()(ast::While& while_){
            annotateWhileLoop(while_);
        }

        void operator()(ast::DoWhile& while_){
            annotateWhileLoop(while_);
        }

        void operator()(ast::Switch& switch_){
            switch_.context = currentContext;

            visit(*this, switch_.value);
            visit_each_non_variant(*this, switch_.cases);
            visit_optional_non_variant(*this, switch_.default_case);
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
            currentContext = for_.context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);

            visit_optional(*this, for_.start);
            visit_optional(*this, for_.condition);
            visit_optional(*this, for_.repeat);

            visit_each(*this, for_.instructions);

            currentContext = currentContext->parent();
        }

        template<typename Loop>
        void annotateSimpleLoop(Loop& loop){
            loop.context = currentContext = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);

            visit_each(*this, loop.instructions);

            currentContext = currentContext->parent();
        }

        void operator()(ast::Foreach& foreach){
            annotateSimpleLoop(foreach);
        }

        void operator()(ast::ForeachIn& foreach){
            annotateSimpleLoop(foreach);
        }

        void operator()(ast::If& if_){
            currentContext = if_.context = std::make_shared<BlockContext>(currentContext, functionContext, globalContext);

            visit(*this, if_.condition);

            visit_each(*this, if_.instructions);

            currentContext = currentContext->parent();

            visit_each_non_variant(*this, if_.elseIfs);
            visit_optional_non_variant(*this, if_.else_);
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
            declaration.context = currentContext;

            visit_each(*this, declaration.values);
        }

        void operator()(ast::VariableDeclaration& declaration){
            declaration.context = currentContext;

            visit_optional(*this, declaration.value);
        }

        void operator()(ast::ArrayDeclaration& declaration){
            declaration.context = currentContext;

            visit(*this, declaration.size);
        }

        void operator()(ast::Assignment& assignment){
            assignment.context = currentContext;

            visit(*this, assignment.left_value);
            visit(*this, assignment.value);
        }

        void operator()(ast::VariableValue& variable){
            variable.context = currentContext;
        }

        void operator()(ast::Return& return_){
            return_.context = functionContext;

            visit(*this, return_.value);
        }

        void operator()(ast::Cast& cast){
            cast.context = currentContext;

            visit(*this, cast.value);
        }

        void operator()(ast::New& new_){
            new_.context = currentContext;

            visit_each(*this, new_.values);
        }

        void operator()(ast::NewArray& new_){
            new_.context = currentContext;

            visit(*this, new_.size);
        }

        void operator()(ast::Expression& expression){
            expression.context = currentContext;

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
        currentContext = globalContext = program.context;
    } else {
        currentContext = globalContext = program.context;

        for(auto& block : program.blocks){
            if(auto* ptr = boost::get<ast::GlobalVariableDeclaration>(&block)){
                ptr->context = currentContext;
            } else if(auto* ptr = boost::get<ast::GlobalArrayDeclaration>(&block)){
                ptr->context = currentContext;
            }
        }
    }
}

#define HANDLE_FUNCTION() \
    currentContext = function.context = functionContext = std::make_shared<FunctionContext>(currentContext, globalContext, platform, configuration); \
    auto visitor = make_visitor(globalContext, functionContext, currentContext); \
    visit_each(visitor, function.instructions); \
    currentContext = currentContext->parent();

void ast::ContextAnnotationPass::apply_function(ast::TemplateFunctionDeclaration& function){
    HANDLE_FUNCTION();
}

void ast::ContextAnnotationPass::apply_struct_function(ast::TemplateFunctionDeclaration& function){
    HANDLE_FUNCTION();
}

void ast::ContextAnnotationPass::apply_struct_constructor(ast::Constructor& function){
    HANDLE_FUNCTION();
}

void ast::ContextAnnotationPass::apply_struct_destructor(ast::Destructor& function){
    HANDLE_FUNCTION();
}
