//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Call.hpp"

using namespace eddic;

tac::Call::Call(){}
tac::Call::Call(const std::string& f, std::shared_ptr<Function> function) : function(f), function(function) {}
tac::Call::Call(const std::string& f, std::shared_ptr<Function> function, std::shared_ptr<Variable> r): function(f), function(function), return_(r) {}
tac::Call::Call(const std::string& f, std::shared_ptr<Function> function, std::shared_ptr<Variable> r, std::shared_ptr<Variable> r2): function(f), function(function), return_(r), return2_(r2) {}
