//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/pre_alloc_cleanup.hpp"

#include "mtac/Program.hpp"

#include "ltac/Statement.hpp"

using namespace eddic;

template<typename Variant>
bool is_pseudo_reg(Variant& var){
    return boost::get<ltac::PseudoRegister>(&*var);
}

void ltac::pre_alloc_cleanup(std::shared_ptr<mtac::Program> program){
    for(auto& function : program->functions){
        for(auto& bb : function){
            auto it = iterate(bb->l_statements);

            while(it.has_next()){
                auto statement = *it;

                if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                    auto instruction = *ptr;

                    if(instruction->op == ltac::Operator::MOV && is_pseudo_reg(instruction->arg1) && is_pseudo_reg(instruction->arg2)){
                        auto reg1 = boost::get<ltac::PseudoRegister>(*instruction->arg1);
                        auto reg2 = boost::get<ltac::PseudoRegister>(*instruction->arg2);

                        if(reg1 == reg2){
                            it.erase();
                            continue;
                        }
                    }
                }

                ++it;
            }
        }
    }
}
