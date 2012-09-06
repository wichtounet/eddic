//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_JUMP_H
#define TAC_JUMP_H

#include <string>
#include <memory>

namespace eddic {

namespace tac {

template<typename JumpType, typename BasicBlock>
struct Jump {
    std::string label;
    unsigned int depth;

    JumpType type;
    
    //Filled only in later phase replacing the label
    std::shared_ptr<BasicBlock> block;

    Jump();
    Jump(const std::string& label);
    Jump(const std::string& label, JumpType type);
};
    
template<typename JumpType, typename BasicBlock>
tac::Jump<JumpType, BasicBlock>::Jump(){
    //Nothing to init
}

template<typename JumpType, typename BasicBlock>
tac::Jump<JumpType, BasicBlock>::Jump(const std::string& label) : label(label) {
    //Nothing to init
}

template<typename JumpType, typename BasicBlock>
tac::Jump<JumpType, BasicBlock>::Jump(const std::string& label, JumpType type) : label(label), type(type) {
    //Nothing to init
}

} //end of tac

} //end of eddic

#endif
