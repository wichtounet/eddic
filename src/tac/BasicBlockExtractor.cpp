//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <unordered_map>

#include <boost/variant.hpp>

#include "tac/BasicBlockExtractor.hpp"
#include "tac/Program.hpp"
#include "tac/Utils.hpp"

using namespace eddic;

void tac::BasicBlockExtractor::extract(tac::Program& program) const {
    for(auto& function : program.functions){
        std::unordered_map<std::string, std::shared_ptr<BasicBlock>> labels;
       
        //The first is always a leader 
        bool nextIsLeader = true;

        //First separate the statements into basic blocks
        for(auto& statement : function->getStatements()){
            if(auto* ptr = boost::get<std::string>(&statement)){
                function->newBasicBlock();

                labels[*ptr] = function->currentBasicBlock();

                nextIsLeader = false;
            } else {
                if(nextIsLeader || (boost::get<std::shared_ptr<tac::Call>>(&statement) && !safe(boost::get<std::shared_ptr<tac::Call>>(statement)))){
                    function->newBasicBlock();
                    nextIsLeader = false;
                }

                if(boost::get<std::shared_ptr<tac::IfFalse>>(&statement) || boost::get<std::shared_ptr<tac::Return>>(&statement) || boost::get<std::shared_ptr<tac::Goto>>(&statement)){
                    nextIsLeader = true;
                } 

                function->currentBasicBlock()->add(statement);
            }
        }

        //Then, replace all the the labels by reference to basic blocks
        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
                   (*ptr)->block = labels[(*ptr)->label];
                } else if(auto* ptr = boost::get<std::shared_ptr<tac::Goto>>(&statement)){
                   (*ptr)->block = labels[(*ptr)->label];
                }
            }
        }

        function->getStatements().clear();
    }
}
