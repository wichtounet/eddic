//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Call.hpp"

using namespace eddic;

ltac::Call::Call(){
    //Nothing to init    
}

ltac::Call::Call(const std::string& f, std::shared_ptr<Function> functionDefinition) : function(f), functionDefinition(functionDefinition) {
    //Nothing to init    
}
