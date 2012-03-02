//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include <memory>
#include <boost/variant/variant.hpp>

#include "ast/WarningsEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/Position.hpp"

#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Types.hpp"
#include "Variable.hpp"
#include "Compiler.hpp"
#include "Options.hpp"
#include "VisitorUtils.hpp"
#include "Utils.hpp"

using namespace eddic;

namespace {

typedef std::unordered_map<std::shared_ptr<Variable>, ast::Position> Positions;  

struct Collector : public boost::static_visitor<> {
    public:
        AUTO_RECURSE_PROGRAM()

        void operator()(ast::FunctionDeclaration& function){
            for(auto& param : function.Content->parameters){
                positions[function.Content->context->getVariable(param.parameterName)] = function.Content->position;
            }
            
            visit_each(*this, function.Content->instructions);
        }
        
        void operator()(ast::GlobalVariableDeclaration& declaration){
            positions[declaration.Content->context->getVariable(declaration.Content->variableName)] = declaration.Content->position;
        }

        void operator()(ast::GlobalArrayDeclaration& declaration){
            positions[declaration.Content->context->getVariable(declaration.Content->arrayName)] = declaration.Content->position;
        }
        
        void operator()(ast::ArrayDeclaration& declaration){
            positions[declaration.Content->context->getVariable(declaration.Content->arrayName)] = declaration.Content->position;
        }
        
        void operator()(ast::VariableDeclaration& declaration){
            positions[declaration.Content->context->getVariable(declaration.Content->variableName)] = declaration.Content->position;
        }
        
        template<typename T>
        void operator()(T&){
            //Nothing
        }

        const ast::Position& getPosition(std::shared_ptr<Variable> var){
            return positions[var];   
        }

    private:
        Positions positions;  
};

struct Inspector : public boost::static_visitor<> {
    public:
        Inspector(FunctionTable& table, Collector& collector) : functionTable(table), collector(collector) {}

        void check(std::shared_ptr<Context> context){
            auto iter = context->begin();
            auto end = context->end();

            for(; iter != end; iter++){
                auto var = iter->second;

                if(var->referenceCount() == 0){
                    if(var->position().isStack()){
                        warn(collector.getPosition(var), "unused variable '" + var->name() + "'");
                    } else if(var->position().isGlobal()){
                        warn(collector.getPosition(var), "unused global variable '" + var->name() + "'");
                    } else if(var->position().isParameter()){
                        warn(collector.getPosition(var), "unused parameter '" + var->name() + "'");
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
                warn(declaration.Content->position, "unused function '" + declaration.Content->functionName + "'");
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
    
    private:
        FunctionTable& functionTable;
        Collector& collector;
};

} //end of anonymous namespace

void ast::WarningsEngine::check(ast::SourceFile& program, FunctionTable& table) const {
    if(WarningUnused){
        Collector collector;
        visit_non_variant(collector, program);

        Inspector inspector(table, collector);
        visit_non_variant(inspector, program);
    }
}
