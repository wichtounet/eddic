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

} //end of anonymous namespace

void mtac::BasicBlockExtractor::extract(std::shared_ptr<mtac::Program> program) const {
    for(auto& function : program->functions){
        std::unordered_map<std::string, std::shared_ptr<basic_block>> labels;
       
        //The first is always a leader 
        bool nextIsLeader = true;

        function->create_entry_bb();

        //First separate the statements into basic blocks
        for(auto& statement : function->getStatements()){
            if(auto* ptr = boost::get<std::string>(&statement)){
                function->append_bb();

                labels[*ptr] = function->current_bb();

                nextIsLeader = false;
            } else {
                if(nextIsLeader || (boost::get<std::shared_ptr<mtac::Call>>(&statement) && !safe(boost::get<std::shared_ptr<mtac::Call>>(statement)))){
                    function->append_bb();
                    nextIsLeader = false;
                }

                if(boost::get<std::shared_ptr<mtac::IfFalse>>(&statement) || boost::get<std::shared_ptr<mtac::If>>(&statement) || 
                        isReturn(statement) || boost::get<std::shared_ptr<mtac::Goto>>(&statement)){
                    nextIsLeader = true;
                } 

                function->current_bb()->add(statement);
            }
        }

        //Then, replace all the the labels by reference to basic blocks
        for(auto& block : function){
            for(auto& statement : block->statements){
                if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                   (*ptr)->block = labels[(*ptr)->label];
                } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                   (*ptr)->block = labels[(*ptr)->label];
                } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&statement)){
                   (*ptr)->block = labels[(*ptr)->label];
                }
            }
        }

        function->create_exit_bb();

        function->getStatements().clear();
    }
}
