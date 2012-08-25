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

#include "ast/TemplateEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

using namespace eddic;

typedef std::unordered_multimap<std::string, ast::TemplateFunctionDeclaration> TemplateMap;

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

} //end of anonymous namespace

void ast::template_instantiation(ast::SourceFile& program){
    TemplateMap template_functions; 

    Collector collector(template_functions);
    collector(program);

    std::cout << template_functions.size() << std::endl;
}
