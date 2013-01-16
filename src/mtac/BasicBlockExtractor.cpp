//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <unordered_map>

#include "variant.hpp"
#include "Variable.hpp"
#include "Function.hpp"

#include "mtac/BasicBlockExtractor.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

void mtac::BasicBlockExtractor::extract(mtac::Program& program) const {
    for(auto& function : program.functions){
        std::unordered_map<std::string, std::shared_ptr<basic_block>> labels;
       
        //The first is always a leader 
        bool nextIsLeader = true;

        function.create_entry_bb();

        //First separate the statements into basic blocks
        for(auto& quadruple : function.get_statements()){
            if(quadruple.op == mtac::Operator::LABEL){
                function.append_bb();

                labels[quadruple.label()] = function.current_bb();

                nextIsLeader = false;
                continue;
            }

            if(quadruple.op == mtac::Operator::CALL){
                if(!safe(quadruple.function().mangled_name())){
                    function.append_bb();
                    nextIsLeader = false;
                }
            }

            if(nextIsLeader){
                function.append_bb();
                nextIsLeader = false;
            }

            if(quadruple.is_if() || quadruple.is_if_false() || quadruple.op == mtac::Operator::GOTO || quadruple.op == mtac::Operator::RETURN){
                nextIsLeader = true;
            } 

            function.current_bb()->emplace_back(std::move(quadruple));
        }

        //Then, replace all the labels by reference to basic blocks
        for(auto& block : function){
            for(auto& quadruple : block->statements){
                if(quadruple.op == mtac::Operator::GOTO || quadruple.is_if() || quadruple.is_if_false()){
                    quadruple.block = labels[quadruple.label()];
                }
            }
        }

        function.create_exit_bb();

        function.release_statements();
    }
}
