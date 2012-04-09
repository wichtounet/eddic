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
#include "ast/GetTypeVisitor.hpp"
#include "ast/TypeTransformer.hpp"

#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Types.hpp"
#include "Variable.hpp"
#include "Warnings.hpp"
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
        Inspector(SymbolTable& table, Collector& collector) : functionTable(table), collector(collector) {}
    
        AUTO_RECURSE_GLOBAL_DECLARATION() 
        AUTO_RECURSE_FUNCTION_CALLS()
        AUTO_RECURSE_BUILTIN_OPERATORS()
        AUTO_RECURSE_SIMPLE_LOOPS()
        AUTO_RECURSE_FOREACH()
        AUTO_RECURSE_BRANCHES()
        AUTO_RECURSE_BINARY_CONDITION()
        AUTO_RECURSE_COMPOSED_VALUES()
        AUTO_RECURSE_RETURN_VALUES()
        AUTO_RECURSE_ARRAY_VALUES()
        AUTO_RECURSE_VARIABLE_OPERATIONS()
        AUTO_RECURSE_ARRAY_ASSIGNMENT()

        void check(std::shared_ptr<Context> context){
            if(WarningUnused){
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
        }

        void operator()(ast::SourceFile& program){
            check(program.Content->context);

            visit_each(*this, program.Content->blocks);
        }
        
        void operator()(ast::FunctionDeclaration& declaration){
            check(declaration.Content->context);
            
            if(WarningUnused){
                int references = functionTable.referenceCount(declaration.Content->mangledName);

                if(declaration.Content->functionName != "main" && references == 0){
                    warn(declaration.Content->position, "unused function '" + declaration.Content->functionName + "'");
                }
            }
        
            visit_each(*this, declaration.Content->instructions);
        }
    
        void operator()(ast::Cast& cast){
            if(WarningCast){
                eddic::Type srcType = visit(ast::GetTypeVisitor(functionTable), cast.Content->value);
                eddic::Type destType = visit(ast::TypeTransformer(functionTable), cast.Content->type);

                std::cout << "cast " << (int) srcType.base() << " " << (int) destType.base() << std::endl;

                if(srcType == destType){
                    warn(cast.Content->position, "cast is not useful");
                }
            }
        }

        template<typename T>
        void operator()(T&){
            //Nothing to check there
        }
    
    private:
        SymbolTable& functionTable;
        Collector& collector;
};

} //end of anonymous namespace

void ast::checkForWarnings(ast::SourceFile& program, SymbolTable& table){
    Collector collector;
    visit_non_variant(collector, program);

    Inspector inspector(table, collector);
    visit_non_variant(inspector, program);
}
