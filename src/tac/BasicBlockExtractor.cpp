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

using namespace eddic;

namespace {


} //end of anonymous namespace

void tac::BasicBlockExtractor::extract(tac::Program& program) const {
    std::unordered_map<std::string, std::shared_ptr<BasicBlock>> labels;

    for(auto& function : program.functions){
        auto current = function->newBasicBlock();

        bool nextIsLeader = false;

        for(auto& statement : function->getStatements()){
            if(auto* ptr = boost::get<std::string>(&statement)){
                function->newBasicBlock();

                labels[*ptr] = function->currentBasicBlock();

                nextIsLeader = false;
            } else {
                if(nextIsLeader){
                    function->newBasicBlock();
                    nextIsLeader = false;
                }

                if(boost::get<tac::IfFalse>(&statement) || boost::get<tac::Return>(&statement) || boost::get<tac::Goto>(&statement)){
                    nextIsLeader = true;
                }

                function->currentBasicBlock()->add(statement);
            }
        }

        function->getStatements().clear();
    }
}
