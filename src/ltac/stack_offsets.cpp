//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <unordered_map>

#include "Type.hpp"
#include "GlobalContext.hpp"

#include "mtac/Program.hpp"

#include "ltac/stack_offsets.hpp"
#include "ltac/Address.hpp"
#include "ltac/Register.hpp"
#include "ltac/Printer.hpp"

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

void ltac::fix_stack_offsets(mtac::Program& program, Platform platform){
    timing_timer timer(program.context->timing(), "stack_offsets");

    for(auto& function : program.functions){
        std::unordered_map<std::string, int> offset_labels;
        int bp_offset = 0;
        
        for(auto& bb : function){
            for(auto& instruction : bb->l_statements){
                if(instruction.is_label()){
                    if(offset_labels.count(instruction.label)){
                        bp_offset = offset_labels[instruction.label];
                        offset_labels.erase(instruction.label);
                    }
                }
                else if(instruction.is_jump()){
                    if(instruction.op != ltac::Operator::CALL && instruction.op != ltac::Operator::ALWAYS){
                        offset_labels[instruction.label] = bp_offset;
                    }
                } else {
                    change_address(instruction.arg1, bp_offset);
                    change_address(instruction.arg2, bp_offset);
                    change_address(instruction.arg3, bp_offset);

                    if(opt_variant_equals(instruction.arg1, ltac::SP)){
                        if(instruction.op == ltac::Operator::ADD){
                            bp_offset -= boost::get<int>(*instruction.arg2);
                        }

                        if(instruction.op == ltac::Operator::SUB){
                            bp_offset += boost::get<int>(*instruction.arg2);
                        }
                    }

                    if(instruction.op == ltac::Operator::PUSH){
                        bp_offset += INT->size(platform);
                    }

                    if(instruction.op == ltac::Operator::POP){
                        bp_offset -= INT->size(platform);
                    }
                }
            } 
        }
    }
}
