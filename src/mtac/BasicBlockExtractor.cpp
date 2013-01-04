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
#include "mtac/Statement.hpp"

using namespace eddic;

namespace {

template<typename T>
bool isReturn(T statement){
    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        return (*ptr)->op == mtac::Operator::RETURN;
    }

    return false;
}

template<typename T>
bool is_goto(T statement){
    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        return (*ptr)->op == mtac::Operator::GOTO;
    }

    return false;
}

} //end of anonymous namespace

void mtac::BasicBlockExtractor::extract(mtac::Program& program) const {
    for(auto& function : program.functions){
        std::unordered_map<std::string, std::shared_ptr<basic_block>> labels;
       
        //The first is always a leader 
        bool nextIsLeader = true;

        function.create_entry_bb();

        //First separate the statements into basic blocks
        for(auto& statement : function.get_statements()){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if((*ptr)->op == mtac::Operator::LABEL){
                    function.append_bb();

                    labels[(*ptr)->label()] = function.current_bb();

                    nextIsLeader = false;
                    continue;
                }

                if((*ptr)->op == mtac::Operator::CALL){
                    if(!safe((*ptr)->function().mangled_name())){
                        function.append_bb();
                        nextIsLeader = false;
                    }
                }
                
                if(nextIsLeader){
                    function.append_bb();
                    nextIsLeader = false;
                }

                if((*ptr)->is_if() || (*ptr)->is_if_false() || isReturn(statement) || is_goto(statement)){
                    nextIsLeader = true;
                } 

                function.current_bb()->add(statement);
            } 
        }

        //Then, replace all the the labels by reference to basic blocks
        for(auto& block : function){
            for(auto& statement : block->statements){
                if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                    if((*ptr)->op == mtac::Operator::GOTO || (*ptr)->is_if() || (*ptr)->is_if_false()){
                        (*ptr)->block = labels[(*ptr)->label()];
                    }
                }
            }
        }

        function.create_exit_bb();

        function.release_statements();
    }
}
