//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Call.hpp"

using namespace eddic;

tac::Call::Call(){}
tac::Call::Call(const std::string& f, std::shared_ptr<Function> functionDefinition) : 
    function(f), functionDefinition(functionDefinition) {}
tac::Call::Call(const std::string& f, std::shared_ptr<Function> functionDefinition, std::shared_ptr<Variable> r) :
    function(f), functionDefinition(functionDefinition), return_(r) {}
tac::Call::Call(const std::string& f, std::shared_ptr<Function> functionDefinition, std::shared_ptr<Variable> r, std::shared_ptr<Variable> r2) :
    function(f), functionDefinition(functionDefinition), return_(r), return2_(r2) {}
