//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <sstream>

#include "cpp_utils/assert.hpp"

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
    cpp_unreachable("The float label does not exists");
}

std::unordered_map<double, std::string> FloatPool::get_pool() const {
    return pool;
}
