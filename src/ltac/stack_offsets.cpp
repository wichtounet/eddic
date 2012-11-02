//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "Type.hpp"

#include "ltac/stack_offsets.hpp"
#include "ltac/Address.hpp"
#include "ltac/Register.hpp"
#include "ltac/Statement.hpp"

using namespace eddic;

namespace {

template<typename Type, typename Variant>
bool opt_variant_equals(Variant& opt_variant, Type& value){
    if(opt_variant){
        if(auto* ptr = boost::get<Type>(&*opt_variant)){
            if(*ptr == value){
                return true;
            }
        }
    }

    return false;
}

template<typename Arg>
void change_address(Arg& arg, int bp_offset){
    if(arg){
        if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            auto& address = *ptr;

            if(opt_variant_equals(address.base_register, ltac::BP)){
                address.base_register = ltac::SP; 
                
                if(address.displacement){
                    *address.displacement += bp_offset;
                } else {
                    address.displacement = bp_offset;
                }
            }
        }
    }
}

}

void ltac::fix_stack_offsets(std::shared_ptr<mtac::Program> program, Platform platform){
    int bp_offset = 0;

    std::unordered_map<std::string, int> offset_labels;

    for(auto& function : program->functions){
        for(auto& bb : function){
            for(auto& statement : bb->l_statements){
                if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                    auto instruction = *ptr;

                    change_address(instruction->arg1, bp_offset);
                    change_address(instruction->arg2, bp_offset);
                    change_address(instruction->arg3, bp_offset);

                    if(opt_variant_equals(instruction->arg1, ltac::BP)){
                        if(instruction->op == ltac::Operator::ADD){
                            bp_offset -= boost::get<int>(*instruction->arg2);
                        }

                        if(instruction->op == ltac::Operator::SUB){
                            bp_offset += boost::get<int>(*instruction->arg2);
                        }
                    }

                    if(instruction->op == ltac::Operator::PUSH){
                        bp_offset += INT->size(platform);
                    }

                    if(instruction->op == ltac::Operator::POP){
                        bp_offset -= INT->size(platform);
                    }
                } else if(auto* ptr = boost::get<std::string>(&statement)){
                    if(offset_labels.count(*ptr)){
                        bp_offset = offset_labels[*ptr];
                        offset_labels.erase(*ptr);
                    }
                } else if(auto* ptr = boost::get<std::shared_ptr<ltac::Jump>>(&statement)){
                    auto jump = *ptr;
                    if(jump->type != ltac::JumpType::CALL && jump->type != ltac::JumpType::ALWAYS){
                        offset_labels[jump->label] = bp_offset;
                    }
                }
            }
        }
    }
}
