//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Call.hpp"

using namespace eddic;

mtac::Call::Call(){}
mtac::Call::Call(const std::string& f, std::shared_ptr<Function> functionDefinition) : 
    function(f), functionDefinition(functionDefinition) {}
mtac::Call::Call(const std::string& f, std::shared_ptr<Function> functionDefinition, std::shared_ptr<Variable> r) :
    function(f), functionDefinition(functionDefinition), return_(r) {}
mtac::Call::Call(const std::string& f, std::shared_ptr<Function> functionDefinition, std::shared_ptr<Variable> r, std::shared_ptr<Variable> r2) :
    function(f), functionDefinition(functionDefinition), return_(r), return2_(r2) {}
