//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Struct.hpp"

using namespace eddic;

Member::Member(const std::string& n, std::shared_ptr<const Type> t) : name(n), type(t) {}

void Member::add_reference(){
    ++references;
}

unsigned int Member::get_references(){
    return references;
}

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

    eddic_unreachable("This member is not contained in the struct");
}
    
void Struct::add_reference(){
    ++references;
}

unsigned int Struct::get_references(){
    return references;
}
