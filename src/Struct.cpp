//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Struct.hpp"

using namespace eddic;

Member::Member(const std::string& n, Type t) : name(n), paramType(t) {}

Struct::Struct(const std::string& n) : name(n) {}
