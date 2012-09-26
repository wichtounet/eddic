//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TEMPLATE_ENGINE_H
#define TEMPLATE_ENGINE_H

#include <memory>
#include <vector>
#include <unordered_map>

#include "ast/source_def.hpp"

namespace eddic {

namespace ast {

class TemplateEngine {
    public:
        typedef std::unordered_multimap<std::string, ast::TemplateFunctionDeclaration> LocalFunctionTemplateMap;
        typedef std::unordered_map<std::string, LocalFunctionTemplateMap> FunctionTemplateMap;

        typedef std::unordered_multimap<std::string, std::vector<ast::Type>> LocalFunctionInstantiationMap;
        typedef std::unordered_map<std::string, LocalFunctionInstantiationMap> FunctionInstantiationMap;

        typedef std::unordered_multimap<std::string, ast::TemplateStruct> ClassTemplateMap;
        typedef std::unordered_multimap<std::string, std::vector<ast::Type>> ClassInstantiationMap;

        void check_function(ast::FunctionCall& function_call);
        void check_member_function(ast::MemberFunctionCall& member_function_call);
        void check_type(ast::Type& type, ast::Position& position);

        void template_instantiation(SourceFile& program);
   
        FunctionTemplateMap function_templates;
        FunctionInstantiationMap function_template_instantiations;
        
        ClassTemplateMap class_templates;
        ClassInstantiationMap class_template_instantiations;
};

} //end of ast

} //end of eddic

#endif
