//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FunctionsAnnotator.hpp"
#include "FunctionTable.hpp"
#include "GetTypeVisitor.hpp"
#include "SemanticalException.hpp"
#include "ASTVisitor.hpp"
#include "VisitorUtils.hpp"
#include "TypeTransformer.hpp"
#include "mangling.hpp"
#include "Options.hpp"
#include "Compiler.hpp"

#include "ast/SourceFile.hpp"

using namespace eddic;

class FunctionInserterVisitor : public boost::static_visitor<> {
    private:
        FunctionTable& functionTable;

    public:
        FunctionInserterVisitor(FunctionTable& table) : functionTable(table) {}

        AUTO_RECURSE_PROGRAM()
         
        void operator()(ast::FunctionDeclaration& declaration){
            auto signature = std::make_shared<Function>(stringToType(declaration.Content->returnType), declaration.Content->functionName);

            if(signature->returnType.isArray()){
                throw SemanticalException("Cannot return array from function");
            }

            for(auto& param : declaration.Content->parameters){
                Type paramType = visit(TypeTransformer(), param.parameterType);
                signature->parameters.push_back(ParameterType(param.parameterName, paramType));
            }
            
            declaration.Content->mangledName = signature->mangledName = mangle(declaration.Content->functionName, signature->parameters);

            if(functionTable.exists(signature->mangledName)){
                throw SemanticalException("The function " + signature->name + " has already been defined");
            }

            functionTable.addFunction(signature);
        }

        template<typename T>
        void operator()(T&){
            //Stop recursion here
        }
};

class FunctionCheckerVisitor : public boost::static_visitor<> {
    private:
        FunctionTable& functionTable;
        std::shared_ptr<Function> currentFunction;

    public:
        FunctionCheckerVisitor(FunctionTable& table) : functionTable(table) {}

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
            currentFunction = functionTable.getFunction(declaration.Content->mangledName);

            visit_each(*this, declaration.Content->instructions);
        }

        void operator()(ast::FunctionCall& functionCall){
            visit_each(*this, functionCall.Content->values);
            
            std::string name = functionCall.Content->functionName;
            
            if(name == "println" || name == "print"){
                return;
            }

            std::string mangled = mangle(name, functionCall.Content->values);

            if(!functionTable.exists(mangled)){
                throw SemanticalException("The function \"" + unmangle(mangled) + "\" does not exists");
            } 

            functionTable.addReference(mangled);

            functionCall.Content->function = functionTable.getFunction(mangled);
            
            visit_each(*this, functionCall.Content->values);
        }

        void operator()(ast::Return& return_){
            return_.Content->function = currentFunction;

            visit(*this, return_.Content->value);
        }

        template<typename T>        
        void operator()(T&){
            //No function calls there
        }
};

void FunctionsAnnotator::annotate(ast::SourceFile& program, FunctionTable& functionTable) const {
    //First phase : Collect functions
    FunctionInserterVisitor inserterVisitor(functionTable);
    inserterVisitor(program);

    //Second phase : Verify calls
    FunctionCheckerVisitor checkerVisitor(functionTable);
    checkerVisitor(program);
}
