//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GlobalContext.hpp"

#include "mtac/Program.hpp"

#include "ltac/pre_alloc_cleanup.hpp"

using namespace eddic;

template<typename Variant>
bool is_pseudo_reg(Variant& var){
    return boost::get<ltac::PseudoRegister>(&*var);
}

template<typename Variant>
bool is_float_pseudo_reg(Variant& var){
    return boost::get<ltac::PseudoFloatRegister>(&*var);
}

void ltac::pre_alloc_cleanup(mtac::Program& program){
    timing_timer timer(program.context->timing(), "pre_alloc_cleanup");

    for(auto& function : program.functions){
        for(auto& bb : function){
            auto it = iterate(bb->l_statements);

            while(it.has_next()){
                auto& instruction = *it;

                if(instruction.op == ltac::Operator::MOV && is_pseudo_reg(instruction.arg1) && is_pseudo_reg(instruction.arg2)){
                    auto reg1 = boost::get<ltac::PseudoRegister>(*instruction.arg1);
                    auto reg2 = boost::get<ltac::PseudoRegister>(*instruction.arg2);

                    if(reg1 == reg2){
                        it.erase();
                        continue;
                    }
                }

                if(instruction.op == ltac::Operator::FMOV && is_float_pseudo_reg(instruction.arg1) && is_float_pseudo_reg(instruction.arg2)){
                    auto reg1 = boost::get<ltac::PseudoFloatRegister>(*instruction.arg1);
                    auto reg2 = boost::get<ltac::PseudoFloatRegister>(*instruction.arg2);

                    if(reg1 == reg2){
                        it.erase();
                        continue;
                    }
                }

                ++it;
            }
        }
    }
}
