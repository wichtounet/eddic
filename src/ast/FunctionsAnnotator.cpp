//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/FunctionsAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/ASTVisitor.hpp"

#include "SymbolTable.hpp"
#include "SemanticalException.hpp"
#include "VisitorUtils.hpp"
#include "mangling.hpp"
#include "Options.hpp"
#include "Compiler.hpp"

using namespace eddic;

class FunctionInserterVisitor : public boost::static_visitor<> {
    public:
        AUTO_RECURSE_PROGRAM()
         
        void operator()(ast::FunctionDeclaration& declaration){
            if(!is_standard_type(declaration.Content->returnType)){
                throw SemanticalException("A function can only returns standard types for now", declaration.Content->position);
            }

            auto signature = std::make_shared<Function>(newType(declaration.Content->returnType), declaration.Content->functionName);

            if(signature->returnType.isArray()){
                throw SemanticalException("Cannot return array from function", declaration.Content->position);
            }

            for(auto& param : declaration.Content->parameters){
                Type paramType = visit(ast::TypeTransformer(), param.parameterType);
                signature->parameters.push_back(ParameterType(param.parameterName, paramType));
            }
            
            declaration.Content->mangledName = signature->mangledName = mangle(declaration.Content->functionName, signature->parameters);

            if(symbols.exists(signature->mangledName)){
                throw SemanticalException("The function " + signature->name + " has already been defined", declaration.Content->position);
            }

            symbols.addFunction(signature);
            symbols.getFunction(signature->mangledName)->context = declaration.Content->context;
        }

        AUTO_IGNORE_OTHERS()
};

class FunctionCheckerVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<Function> currentFunction;

    public:
        AUTO_RECURSE_PROGRAM()
        AUTO_RECURSE_GLOBAL_DECLARATION() 
        AUTO_RECURSE_SIMPLE_LOOPS()
        AUTO_RECURSE_FOREACH()
        AUTO_RECURSE_BRANCHES()
        AUTO_RECURSE_BINARY_CONDITION()
        AUTO_RECURSE_BUILTIN_OPERATORS()
        AUTO_RECURSE_COMPOSED_VALUES()
        AUTO_RECURSE_MINUS_PLUS_VALUES()
        AUTO_RECURSE_VARIABLE_OPERATIONS()

        void operator()(ast::FunctionDeclaration& declaration){
            currentFunction = symbols.getFunction(declaration.Content->mangledName);

            visit_each(*this, declaration.Content->instructions);
        }

        void operator()(ast::FunctionCall& functionCall){
            visit_each(*this, functionCall.Content->values);
            
            std::string name = functionCall.Content->functionName;
            
            if(name == "println" || name == "print" || name == "duration"){
                return;
            }

            std::string mangled = mangle(name, functionCall.Content->values);

            if(!symbols.exists(mangled)){
                throw SemanticalException("The function \"" + unmangle(mangled) + "\" does not exists", functionCall.Content->position);
            } 

            symbols.addReference(mangled);

            functionCall.Content->function = symbols.getFunction(mangled);
            
            visit_each(*this, functionCall.Content->values);
        }

        void operator()(ast::Return& return_){
            return_.Content->function = currentFunction;

            visit(*this, return_.Content->value);
        }

        AUTO_IGNORE_OTHERS()
};

void ast::defineFunctions(ast::SourceFile& program){
    //First phase : Collect functions
    FunctionInserterVisitor inserterVisitor;
    inserterVisitor(program);

    //Second phase : Verify calls
    FunctionCheckerVisitor checkerVisitor;
    checkerVisitor(program);
}
