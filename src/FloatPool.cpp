//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <sstream>

#include "assert.hpp"
#include "FloatPool.hpp"

using namespace eddic;

FloatPool::FloatPool() : currentString(0) {
    //No labels are inserted by default
}

std::string FloatPool::label(double value) {
    if (pool.find(value) == pool.end()) {
        std::stringstream ss;
        ss << "F";
        ss << ++currentString;
        pool[value] = ss.str();
    }

    return pool[value];
}

double FloatPool::value(const std::string& label) const {
    for (auto it : pool){
        if(it.second == label){
            return it.first;
        }
    }

    //This method should not be called on not-existing label
    ASSERT_PATH_NOT_TAKEN("The float label does not exists");
}

std::unordered_map<double, std::string> FloatPool::get_pool() const {
    return pool;
}
