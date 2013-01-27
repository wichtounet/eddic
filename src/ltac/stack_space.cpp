//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"
#include "Variable.hpp"

#include "ltac/stack_space.hpp"
#include "ltac/Utils.hpp"
#include "ltac/Address.hpp"
#include "ltac/Instruction.hpp"

using namespace eddic;

namespace {

void optimize_ranges(std::vector<std::pair<int, int>>& memset_ranges){
    std::sort(memset_ranges.begin(), memset_ranges.end(), [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs){ return lhs.first < rhs.first; });

    auto it = memset_ranges.begin();

    while(it != memset_ranges.end()){
        auto& range = *it;

        auto second_it = it;
        ++second_it;

        while(second_it != memset_ranges.end()){
            auto& second_range = *second_it;

            if(second_range.first == range.first + range.second){
                range.second += second_range.second;

                second_it = memset_ranges.erase(second_it);
            } else {
                break;
            }
        }

        it = second_it;
    }
}

} //end of anonymous namespace

void ltac::alloc_stack_space(mtac::Program& program){
    auto platform = program.context->target_platform();

    for(auto& function : program.functions){
        auto bb = function.entry_bb();
        auto int_size = INT->size(platform);

        //Clear the stack variables

        std::vector<std::pair<int, int>> memset_ranges;

        for(auto& var_pair : *function.context){
            auto& var = var_pair.second;

            if(var->position().isStack()){
                auto type = var->type();
                int position = var->position().offset();

                if(type->is_array() && type->has_elements()){
                    memset_ranges.emplace_back(position + int_size, type->data_type()->size(platform) * type->elements());
                } else if(type->is_custom_type()){
                    memset_ranges.emplace_back(position, type->size(platform));
                }
            }
        }

        optimize_ranges(memset_ranges);

        for(auto& range : memset_ranges){
            int size = range.second / int_size;

            if(size <= 4){
                for(int i = 0; i < size; ++i){
                    ltac::add_instruction(bb, ltac::Operator::MOV, ltac::Address(ltac::BP, range.first + i * int_size), 0);
                }
            } else {
                ltac::add_instruction(bb, ltac::Operator::MEMSET, ltac::Address(ltac::BP, range.first), size);
            }
        }

        //Set the sizes of arrays

        for(auto& var_pair : *function.context){
            auto& var = var_pair.second;

            if(var->position().isStack()){
                auto type = var->type();
                int position = var->position().offset();

                if(type->is_array() && type->has_elements()){
                    ltac::add_instruction(bb, ltac::Operator::MOV, ltac::Address(ltac::BP, position), static_cast<int>(type->elements()));
                } else if(type->is_custom_type()){
                    //Set lengths of arrays inside structures
                    auto struct_type = function.context->global()->get_struct(type);
                    auto offset = 0;

                    while(struct_type){
                        for(auto& member : struct_type->members){
                            if(member->type->is_array() && !member->type->is_dynamic_array()){
                                ltac::add_instruction(bb, ltac::Operator::MOV, 
                                        ltac::Address(ltac::BP, position + offset + function.context->global()->member_offset(struct_type, member->name)),
                                        static_cast<int>(member->type->elements()));
                            }
                        }

                        struct_type = function.context->global()->get_struct(struct_type->parent_type);
                    }
                }
            }
        }
    }
}
