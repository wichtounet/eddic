//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/StructuresAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

#include "SymbolTable.hpp"
#include "SemanticalException.hpp"
#include "Types.hpp"

using namespace eddic;

struct StructuresCollector : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()

    void operator()(ast::Struct& struct_){
        if(symbols.struct_exists(struct_.Content->name)){
            throw SemanticalException("The structure " + struct_.Content->name + " has already been defined", struct_.Content->position);
        }

        auto signature = std::make_shared<Struct>(struct_.Content->name);

        for(auto& member : struct_.Content->members){
            if(!is_standard_type(member.Content->type)){
                throw SemanticalException("For now, only standard type are supporterd as members of structures", member.Content->position);
            }

            Type memberType = newType(member.Content->type);
            signature->members.push_back({member.Content->name, memberType});
        }

        symbols.add_struct(signature);
    }

    template<typename T>
    void operator()(T&){
        //Stop recursion here
    }
};

void ast::defineStructures(ast::SourceFile& program){
    StructuresCollector collector;
    collector(program);
}
