//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include <memory>
#include <boost/variant/variant.hpp>

#include "WarningsEngine.hpp"

#include "GetTypeVisitor.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Types.hpp"
#include "Variable.hpp"

#include "Compiler.hpp"
#include "Options.hpp"
#include "TypeTransformer.hpp"

#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

#include "ast/SourceFile.hpp"

using namespace eddic;

struct Inspector : public boost::static_visitor<> {
    private:
        FunctionTable& functionTable;

    public:
        Inspector(FunctionTable& table) : functionTable(table) {}

        void check(std::shared_ptr<Context> context){
            auto iter = context->begin();
            auto end = context->end();

            for(; iter != end; iter++){
                auto var = iter->second;

                if(var->referenceCount() == 0){
                    if(var->position().isStack()){
                        warn("unused variable '" + var->name() + "'");
                    } else if(var->position().isGlobal()){
                        warn("unused global variable '" + var->name() + "'");
                    } else if(var->position().isParameter()){
                        warn("unused parameter '" + var->name() + "'");
                    }
                }
            }
        }

        void operator()(ast::SourceFile& program){
            check(program.Content->context);

            visit_each(*this, program.Content->blocks);
        }
        
        void operator()(ast::FunctionDeclaration& declaration){
            check(declaration.Content->context);
            
            int references = functionTable.referenceCount(declaration.Content->mangledName);

            if(declaration.Content->functionName != "main" && references == 0){
                warn("unused function '" + declaration.Content->functionName + "'");
            }
        }
        
        void operator()(ast::Import&){
            //Nothing to warn about there
        }

        void operator()(ast::StandardImport&){
            //Nothing to warn about there
        }

        void operator()(ast::GlobalVariableDeclaration&){
            //Nothing to check there
        }

        void operator()(ast::GlobalArrayDeclaration&){
            //Nothing to check there
        }
};

void WarningsEngine::check(ast::SourceFile& program, FunctionTable& table) const {
    if(WarningUnused){
        Inspector inspector(table);
        visit_non_variant(inspector, program);
    }
}
