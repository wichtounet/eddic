//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "Struct.hpp"

using namespace eddic;

Member::Member(const std::string& n, Type t) : name(n), type(t) {}

Struct::Struct(const std::string& n) : name(n) {}

bool Struct::member_exists(const std::string& n){
    for(auto member : members){
        if(member->name == n){
            return true;
        }
    }

    return false;
}

std::shared_ptr<Member> Struct::operator[](const std::string& n){
    for(auto member : members){
        if(member->name == n){
            return member;
        }
    }

    assert(false && "This member is not contained in the struct");
}
