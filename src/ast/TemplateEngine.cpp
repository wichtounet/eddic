//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "variant.hpp"
#include "assert.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"
#include "iterators.hpp"
#include "Type.hpp"
#include "SemanticalException.hpp"

#include "ast/TemplateEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

using namespace eddic;

typedef std::unordered_multimap<std::string, ast::TemplateFunctionDeclaration> TemplateMap;
typedef std::unordered_multimap<std::string, std::vector<std::string>> InstantiationMap;

namespace {

struct Collector : public boost::static_visitor<> {
    TemplateMap& template_functions; 

    Collector(TemplateMap& template_functions) : template_functions(template_functions) {}

    AUTO_RECURSE_PROGRAM()

    void operator()(ast::TemplateFunctionDeclaration& declaration){
        template_functions.insert(TemplateMap::value_type(declaration.Content->functionName, declaration));
    }

    AUTO_IGNORE_OTHERS()
};

struct Instantiator : public boost::static_visitor<> {
    TemplateMap& template_functions;
    InstantiationMap instantiations;

    Instantiator(TemplateMap& template_functions) : template_functions(template_functions) {}

    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_DECLARATION()
    AUTO_RECURSE_GLOBAL_DECLARATION() 
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_FOREACH()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
    AUTO_RECURSE_BUILTIN_OPERATORS()
    AUTO_RECURSE_COMPOSED_VALUES()
    AUTO_RECURSE_MEMBER_VALUE()
    AUTO_RECURSE_UNARY_VALUES()
    AUTO_RECURSE_VARIABLE_OPERATIONS()
    AUTO_RECURSE_STRUCT()
    AUTO_RECURSE_CONSTRUCTOR()
    AUTO_RECURSE_DESTRUCTOR()
    AUTO_RECURSE_SWITCH()
    AUTO_RECURSE_SWITCH_CASE()
    AUTO_RECURSE_DEFAULT_CASE()
    AUTO_RECURSE_RETURN_VALUES()

    bool are_equals(const std::vector<std::string>& template_types, const std::vector<std::string>& types){
        if(types.size() != template_types.size()){
            return false;
        }

        for(std::size_t i = 0; i < template_types.size(); ++i){
            if(template_types[i] != types[i]){
                return false;
            }
        }

        return true;
    }

    bool is_instantiated(const std::string& name, const std::vector<std::string>& template_types){
        auto it = instantiations.find(name);

        while(it != instantiations.end()){
            auto types = it->second;
           
            if(are_equals(types, template_types)){
                return true;
            }
        }

        return false;
    }

    std::vector<ast::FunctionParameter> copy(const std::vector<ast::FunctionParameter>& source){
        std::vector<ast::FunctionParameter> destination;   
        destination.reserve(source.size());

        return destination;
    }
    
    std::vector<ast::Instruction> copy(const std::vector<ast::Instruction>& source){
        std::vector<ast::Instruction> destination;   
        destination.reserve(source.size());

        return destination;
    }

    void operator()(ast::FunctionCall& functionCall){
        visit_each(*this, functionCall.Content->values);
        
        auto template_types = functionCall.Content->template_types;

        if(!template_types.empty()){
            std::string name = functionCall.Content->functionName;
            
            auto it = template_functions.find(name);

            if(it == template_functions.end()){
                throw SemanticalException("There are no template function named " + name, functionCall.Content->position);
            }

            while(it != template_functions.end()){
                auto function_declaration = it->second;
                
                if(function_declaration.Content->template_types.size() == template_types.size()){
                    if(!is_instantiated(name, template_types)){
                        //Instantiate the function 
                        ast::FunctionDeclaration declaration;
                        declaration.Content->instantiated = true;
                        declaration.Content->returnType = function_declaration.Content->returnType;
                        declaration.Content->functionName = function_declaration.Content->functionName;
                        declaration.Content->parameters = copy(function_declaration.Content->parameters);
                        declaration.Content->instructions = copy(function_declaration.Content->instructions);


                        //Mark it as instantiated
                        instantiations.insert(InstantiationMap::value_type(name, template_types));
                    }

                    return;
                }
                
                ++it;
            }
            
            throw SemanticalException("No matching function " + name, functionCall.Content->position);
        }
    }

    void operator()(ast::MemberFunctionCall& functionCall){
        visit_each(*this, functionCall.Content->values);

        //TODO
    }

    AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace

void ast::template_instantiation(ast::SourceFile& program){
    TemplateMap template_functions; 

    Collector collector(template_functions);
    collector(program);

    Instantiator instantiator(template_functions);
    instantiator(program);

    std::cout << template_functions.size() << std::endl;
}
