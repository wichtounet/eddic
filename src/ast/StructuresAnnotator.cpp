//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include "SemanticalException.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "mangling.hpp"

#include "ast/StructuresAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

namespace {

struct StructuresCollector : public boost::static_visitor<> {
    std::shared_ptr<GlobalContext> context;

    StructuresCollector(std::shared_ptr<GlobalContext> context) : context(context) {}

    AUTO_RECURSE_PROGRAM()

    void operator()(ast::Struct& struct_){
        if(!struct_.Content->marked){
            if(struct_.Content->template_types.empty()){
                std::shared_ptr<const Type> struct_type = new_type(context, struct_.Content->name, false);

                struct_.Content->mangled_name = mangle(struct_type);
            } else {
                std::vector<std::shared_ptr<const Type>> template_types;

                ast::TypeTransformer transformer(context);

                for(auto& type : struct_.Content->template_types){
                    template_types.push_back(visit(transformer, type));
                }
                
                std::shared_ptr<const Type> struct_type = new_template_type(struct_.Content->name, template_types);

                struct_.Content->mangled_name = mangle(struct_type);
            }

            if(context->struct_exists(struct_.Content->mangled_name)){
                throw SemanticalException("The structure " + struct_.Content->mangled_name + " has already been defined", struct_.Content->position);
            }

            auto signature = std::make_shared<Struct>(struct_.Content->mangled_name);
            context->add_struct(signature);
        }
    }

    AUTO_IGNORE_OTHERS()
};

struct StructureMembersCollector : public boost::static_visitor<> {
    std::shared_ptr<GlobalContext> context;

    StructureMembersCollector(std::shared_ptr<GlobalContext> context) : context(context) {}

    AUTO_RECURSE_PROGRAM()

    void operator()(ast::Struct& struct_){
        if(!struct_.Content->marked){
            auto signature = context->get_struct(struct_.Content->name);
            std::vector<std::string> names;

            signature->members.clear();

            for(auto& member : struct_.Content->members){
                if(std::find(names.begin(), names.end(), member.Content->name) != names.end()){
                    throw SemanticalException("The member " + member.Content->name + " has already been defined", member.Content->position);
                }

                names.push_back(member.Content->name);

                auto member_type = visit(ast::TypeTransformer(context), member.Content->type);

                if(member_type->is_array()){
                    throw SemanticalException("Arrays inside structures are not supported", member.Content->position);
                }

                signature->members.push_back(std::make_shared<Member>(member.Content->name, member_type));
            }
        }
    }

    AUTO_IGNORE_OTHERS()
};

struct StructuresVerifier : public boost::static_visitor<> {
    std::shared_ptr<GlobalContext> context;

    StructuresVerifier(std::shared_ptr<GlobalContext> context) : context(context) {}
    
    AUTO_RECURSE_PROGRAM()

    void operator()(ast::Struct& struct_){
        if(!struct_.Content->marked){
            auto struct_type = context->get_struct(struct_.Content->name);

            for(auto& member : struct_.Content->members){
                auto type = (*struct_type)[member.Content->name]->type;

                if(type->is_custom_type()){
                    auto struct_name = type->type();

                    if(!context->struct_exists(struct_name)){
                        throw SemanticalException("Invalid member type " + struct_name, member.Content->position);
                    }
                }
            }
        }
    }

    AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace

void ast::defineStructures(ast::SourceFile& program){
    StructuresCollector collector(program.Content->context);
    collector(program);
    
    StructureMembersCollector member_collector(program.Content->context);
    member_collector(program);
    member_collector(program);
    
    StructuresVerifier verify(program.Content->context);
    verify(program);
}
