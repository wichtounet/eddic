//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Call.hpp"

using namespace eddic;

tac::Call::Call(){}
tac::Call::Call(const std::string& f, int p) : function(f), params(p) {}
tac::Call::Call(const std::string& f, int p, std::shared_ptr<Variable> r): function(f), params(p), return_(r) {}
