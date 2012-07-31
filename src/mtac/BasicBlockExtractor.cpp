//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <unordered_map>

#include "variant.hpp"

#include "mtac/BasicBlockExtractor.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"

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
                if(nextIsLeader || (boost::get<std::shared_ptr<mtac::Call>>(&statement) && !safe(boost::get<std::shared_ptr<mtac::Call>>(statement)))){
                    function->newBasicBlock();
                    nextIsLeader = false;
                }

                if(boost::get<std::shared_ptr<mtac::IfFalse>>(&statement) || boost::get<std::shared_ptr<mtac::If>>(&statement) || 
                        isReturn(statement) || boost::get<std::shared_ptr<mtac::Goto>>(&statement)){
                    nextIsLeader = true;
                } 

                function->currentBasicBlock()->add(statement);
            }
        }

        //Then, replace all the the labels by reference to basic blocks
        for(auto& block : function->getBasicBlocks()){
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

        function->getBasicBlocks().push_front(std::make_shared<mtac::BasicBlock>(-1));
        function->getBasicBlocks().push_back(std::make_shared<mtac::BasicBlock>(-2));

        function->getStatements().clear();
    }
}
