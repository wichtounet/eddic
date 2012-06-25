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
#include "ast/GetTypeVisitor.hpp"

#include "SymbolTable.hpp"
#include "SemanticalException.hpp"
#include "VisitorUtils.hpp"
#include "mangling.hpp"
#include "Options.hpp"
#include "Type.hpp"

using namespace eddic;

class FunctionInserterVisitor : public boost::static_visitor<> {
    public:
        AUTO_RECURSE_PROGRAM()
         
        void operator()(ast::FunctionDeclaration& declaration){
            auto return_type = visit(ast::TypeTransformer(), declaration.Content->returnType);
            auto signature = std::make_shared<Function>(return_type, declaration.Content->functionName);

            if(return_type->is_array()){
                throw SemanticalException("Cannot return array from function", declaration.Content->position);
            }

            if(return_type->is_custom_type()){
                throw SemanticalException("Cannot return struct from function", declaration.Content->position);
            }

            for(auto& param : declaration.Content->parameters){
                auto paramType = visit(ast::TypeTransformer(), param.parameterType);
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

        std::vector<std::vector<std::shared_ptr<const Type>>> permutations(std::vector<std::shared_ptr<const Type>> types){
            std::vector<std::vector<std::shared_ptr<const Type>>> perms;

            //TODO Enhance to test all possibilities, not only a change of a single type
            for(std::size_t i = 0; i < types.size(); ++i){
                if(!types[i]->is_pointer() && !types[i]->is_array()){
                    std::vector<std::shared_ptr<const Type>> copy = types;

                    copy[i] = new_pointer_type(types[i]);

                    perms.push_back(copy);

                    for(std::size_t j = i + 1; j < types.size(); ++j){
                        if(!types[j]->is_pointer() && !types[j]->is_array()){
                            copy[j] = new_pointer_type(types[j]);
                            perms.push_back(copy);
                            copy[j] = types[j];
                        }
                    }
                }
            }

            return perms;
        }

        void operator()(ast::FunctionCall& functionCall){
            visit_each(*this, functionCall.Content->values);
            
            std::string name = functionCall.Content->functionName;
            
            if(name == "println" || name == "print" || name == "duration"){
                return;
            }

            std::vector<std::shared_ptr<const Type>> types;

            ast::GetTypeVisitor visitor;
            for(auto& value : functionCall.Content->values){
                auto type = visit(visitor, value);
                types.push_back(type);
            }

            std::string mangled = mangle(name, types);

            if(symbols.exists(mangled)){
                symbols.addReference(mangled);

                functionCall.Content->mangled_name = mangled;
                functionCall.Content->function = symbols.getFunction(mangled);
            } else {
                auto perms = permutations(types);

                for(auto& perm : perms){
                    mangled = mangle(name, perm);

                    if(symbols.exists(mangled)){
                        symbols.addReference(mangled);

                        functionCall.Content->mangled_name = mangled;
                        functionCall.Content->function = symbols.getFunction(mangled);

                        return;
                    }
                }
            
                throw SemanticalException("The function \"" + unmangle(mangled) + "\" does not exists", functionCall.Content->position);
            }
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
