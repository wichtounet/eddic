//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Type.hpp"
#include "VisitorUtils.hpp"
#include "GlobalContext.hpp"
#include "mangling.hpp"
#include "Variable.hpp"

#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

bool mtac::is_single_int_register(std::shared_ptr<const Type> type){
   return type == INT || type == BOOL || type == CHAR || type->is_pointer(); 
}

bool mtac::is_single_float_register(std::shared_ptr<const Type> type){
    return type == FLOAT;
}

bool mtac::is_recursive(mtac::Function& function){
    for(auto& basic_block : function){
        for(auto& quadruple : basic_block->statements){
            if(quadruple.op == mtac::Operator::CALL && quadruple.function().mangled_name() == function.definition().mangled_name()){
                return true;
            }
        }
    }

    return false;
}

void eddic::mtac::computeBlockUsage(mtac::Function& function, std::unordered_set<mtac::basic_block_p>& usage){
    for(auto& block : function){
        for(auto& quadruple : block->statements){
            usage.insert(quadruple.block);
        }
    }
}

bool eddic::mtac::safe(const std::string& function){
    //These functions are considered as safe because they save/restore all the registers and does not return anything 
    return function == "_F5printS" || function == "_F5printC"; 
}

bool eddic::mtac::erase_result(mtac::Operator op){
   return 
           op != mtac::Operator::DOT_ASSIGN 
        && op != mtac::Operator::DOT_FASSIGN 
        && op != mtac::Operator::DOT_PASSIGN 
        && op != mtac::Operator::RETURN
        && op != mtac::Operator::GOTO
        && op != mtac::Operator::NOP
        && op != mtac::Operator::PARAM
        && op != mtac::Operator::PPARAM
        && op != mtac::Operator::CALL
        && op != mtac::Operator::LABEL
        && !(op >= mtac::Operator::IF_UNARY && op <= mtac::Operator::IF_FALSE_FL);
}

unsigned int eddic::mtac::compute_member_offset(std::shared_ptr<const GlobalContext> context, std::shared_ptr<const Type> type, const std::string& member){
    return compute_member(context, type, member).first;
}

std::pair<unsigned int, std::shared_ptr<const Type>> eddic::mtac::compute_member(std::shared_ptr<const GlobalContext> context, std::shared_ptr<const Type> type, const std::string& member){
    auto struct_type = context->get_struct(type);
    std::shared_ptr<const Type> member_type;
    unsigned int offset = 0;

    do {
        if(struct_type->member_exists(member)){
            member_type = (*struct_type)[member]->type;
            break;
        }

        offset += context->self_size_of_struct(struct_type);

        struct_type = context->get_struct(struct_type->parent_type);
    } while(struct_type);

    eddic_assert(member_type, "The member must exist");

    offset += context->member_offset(struct_type, member);

    return std::make_pair(offset, member_type);
}

mtac::Quadruple mtac::copy(const mtac::Quadruple& quadruple){
    return quadruple;
}
