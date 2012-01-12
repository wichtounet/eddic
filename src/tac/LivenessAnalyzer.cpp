//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant.hpp>

#include "tac/LivenessAnalyzer.hpp"

using namespace eddic;

namespace {

//Set the default properties of the variable
void updateLive(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, tac::Argument arg){
    if(auto* variable = boost::get<std::shared_ptr<Variable>>(&arg)){
        if(liveness.find(*variable) == liveness.end()){
            if((*variable)->position().isTemporary()){
                liveness[*variable] = false;
            } else {
                liveness[*variable] = true;
            }
        }
    }
}

//Set the variable as live
void setLive(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, tac::Argument arg){
    if(auto* variable = boost::get<std::shared_ptr<Variable>>(&arg)){
        liveness[*variable] = true;
    }
}

void computeLiveness(std::shared_ptr<tac::Function> function){
    std::vector<std::shared_ptr<tac::BasicBlock>>::reverse_iterator bit = function->getBasicBlocks().rbegin();
    std::vector<std::shared_ptr<tac::BasicBlock>>::reverse_iterator bend = function->getBasicBlocks().rend(); 
    
    std::unordered_map<std::shared_ptr<Variable>, bool> liveness;

    while(bit != bend){
        std::vector<tac::Statement>::reverse_iterator sit = (*bit)->statements.rbegin();
        std::vector<tac::Statement>::reverse_iterator send = (*bit)->statements.rend(); 
    
        liveness.clear();

        while(sit != send){
            auto statement = *sit;

            if(auto* ptr = boost::get<std::shared_ptr<tac::Return>>(&statement)){
                if((*ptr)->arg1){
                    updateLive(liveness, (*(*ptr)->arg1));
                }
                
                if((*ptr)->arg2){
                    updateLive(liveness, (*(*ptr)->arg2));
                }
                
                if((*ptr)->arg1){
                    setLive(liveness, (*(*ptr)->arg1));
                }
                
                if((*ptr)->arg2){
                    setLive(liveness, (*(*ptr)->arg2));
                }
               
                (*ptr)->liveness = liveness;
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
                updateLive(liveness, (*ptr)->arg1);
                if((*ptr)->arg2){
                    updateLive(liveness, *(*ptr)->arg2);
                }
                
                setLive(liveness, (*ptr)->arg1);
                if((*ptr)->arg2){
                    setLive(liveness, *(*ptr)->arg2);
                }
                
                (*ptr)->liveness = liveness;
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&statement)){
                updateLive(liveness, (*ptr)->arg1);
                
                if((*ptr)->arg2){
                    updateLive(liveness, (*(*ptr)->arg2));
                }
                
                setLive(liveness, (*ptr)->arg1);
                
                if((*ptr)->arg2){
                    setLive(liveness, (*(*ptr)->arg2));
                }
                
                (*ptr)->liveness = liveness;

                if((*ptr)->result){
                    //TODO Take into account that the the result is overriden, so should no be live (warning with a = a + b)
                }
            }

            sit++;
        }

        bit++;
    }
}

}

void tac::LivenessAnalyzer::compute(tac::Program& program){
    for(auto& function : program.functions){
        computeLiveness(function);
    }
}
