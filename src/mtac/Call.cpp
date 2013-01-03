//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Call.hpp"

using namespace eddic;

mtac::Call::Call(Function& functionDefinition) : 
    functionDefinition(functionDefinition) {}
mtac::Call::Call(Function& functionDefinition, std::shared_ptr<Variable> r) :
    functionDefinition(functionDefinition), return_(r) {}
mtac::Call::Call(Function& functionDefinition, std::shared_ptr<Variable> r, std::shared_ptr<Variable> r2) :
    functionDefinition(functionDefinition), return_(r), return2_(r2) {}
