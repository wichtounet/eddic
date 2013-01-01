//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
