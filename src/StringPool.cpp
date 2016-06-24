//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <sstream>

#include "cpp_utils/assert.hpp"

#include "StringPool.hpp"

using namespace eddic;

StringPool::StringPool() : currentString(0) {
    label("");      //- special label for default string value
}

std::string StringPool::label(const std::string& value) {
//    std::lock_guard<std::mutex> lock(mutex);

    if (pool.find(value) == pool.end()) {
        std::stringstream ss;
        ss << "S";
        ss << ++currentString;
        pool[value] = ss.str();
    }

    return pool[value];
}

std::string StringPool::value(const std::string& label) const {
//    std::lock_guard<std::mutex> lock(mutex);

    for (auto it : pool){
        if(it.second == label){
            return it.first;
        }
    }

    //This method should not be called on not-existing label
    cpp_unreachable("The label does not exists");
}

std::unordered_map<std::string, std::string> StringPool::getPool() const {
    return pool;
}
