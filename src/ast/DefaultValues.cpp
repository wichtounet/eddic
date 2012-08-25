//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "variant.hpp"

#include "ast/DefaultValues.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/TypeTransformer.hpp"

#include "Type.hpp"
#include "VisitorUtils.hpp"

using namespace eddic;

namespace {

struct SetDefaultValues : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_DECLARATION()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_FOREACH()
    AUTO_RECURSE_BRANCHES()

    template<typename T>
    void setDefaultValue(T& declaration){
        if(!declaration.Content->value){
            auto type = visit(ast::TypeTransformer(declaration.Content->context->global()), declaration.Content->variableType);

            if(type->is_standard_type()){
                if(type == INT){
                    ast::Integer integer;
                    integer.value = 0;

                    declaration.Content->value = integer;
                } else if(type == STRING){
                    ast::Literal literal;
                    literal.value = "\"\"";
                    literal.label = "S3";

                    declaration.Content->value = literal;
                } else if(type == FLOAT){
                    ast::Float float_;
                    float_.value = 0.0;

                    declaration.Content->value = float_;
                } else if(type == BOOL){
                    ast::False false_;

                    declaration.Content->value = false_;
                } else {
                    ASSERT_PATH_NOT_TAKEN("Unhandled type");
                }
            }
        }
    }

    void operator()(ast::GlobalVariableDeclaration& declaration){
        setDefaultValue(declaration);
    }

    void operator()(ast::VariableDeclaration& declaration){
        setDefaultValue(declaration);
    }

    AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace

void ast::defineDefaultValues(ast::SourceFile& program){
    SetDefaultValues visitor;
    visitor(program);
}
