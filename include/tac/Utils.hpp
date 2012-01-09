//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_UTILS_H
#define TAC_UTILS_H

#include <memory>
#include <unordered_set>

#include <boost/variant.hpp>

#include "tac/Program.hpp"

namespace eddic {

namespace tac {

template<typename V, typename T>
bool equals(T& variant, V value){
    return boost::get<V>(&variant) && boost::get<V>(variant) == value;
}

template<typename T>
bool isInt(T& variant){
    return boost::get<int>(&variant);
}

void computeBlockUsage(std::shared_ptr<tac::Function> function, std::unordered_set<std::shared_ptr<tac::BasicBlock>>& usage);

bool safe(std::shared_ptr<tac::Call> call);

} //end of tac

} //end of eddic

#endif
