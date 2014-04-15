//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "statistics.hpp"

using namespace eddic;

void statistics::inc_counter(const std::string& a){
    ++counters[a];
}

std::size_t statistics::counter(const std::string& a) const {
    return counters.at(a);
}

statistics::iterator statistics::begin() const {
    return counters.cbegin();
}

statistics::iterator statistics::end() const {
    return counters.cend();
}
