//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_ARGUMENT_H
#define MTAC_ARGUMENT_H

#include <memory>
#include <string>

#include "variant.hpp"

namespace eddic {

class Variable;

namespace mtac {

typedef boost::variant<std::shared_ptr<eddic::Variable>, double, int, std::string> Argument;

} //end of mtac

//Needs to be declared in eddic to be used in ltac as well

inline bool operator==(const mtac::Argument& a, int b){
    if(auto* ptr = boost::get<int>(&a)){
        return *ptr == b;
    }

    return false;
}

inline bool operator==(const mtac::Argument& a, double b){
    if(auto* ptr = boost::get<int>(&a)){
        return *ptr == b;
    }

    return false;
}

inline bool operator==(const mtac::Argument& a, const std::shared_ptr<Variable>& b){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&a)){
        return *ptr == b;
    }

    return false;
}

inline bool operator==(const mtac::Argument& a, const std::string& b){
    if(auto* ptr = boost::get<std::string>(&a)){
        return *ptr == b;
    }

    return false;
}

} //end of eddic

#endif
