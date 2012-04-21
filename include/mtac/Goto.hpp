//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_GOTO_H
#define TAC_GOTO_H

#include <string>
#include <memory>

namespace eddic {

namespace tac {

class BasicBlock;

struct Goto {
    std::string label;
    
    //Filled only in later phase replacing the label
    std::shared_ptr<BasicBlock> block;

    Goto();
    Goto(const std::string& label);
};

} //end of tac

} //end of eddic

#endif
