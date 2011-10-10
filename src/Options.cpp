//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Options.hpp"

using namespace eddic;

bool Options::booleanOptions[(int) BooleanOption::COUNT];
std::string Options::valueOptions[(int) ValueOption::COUNT];
        
void Options::setDefaults() {
    set(ValueOption::OUTPUT, "a.out");
}

void Options::set(BooleanOption option) {
    booleanOptions[(int) option] = true;
}

void Options::unset(BooleanOption option) {
    booleanOptions[(int) option] = false;
}

bool Options::isSet(BooleanOption option) {
    return booleanOptions[(int) option];
}

void Options::set(ValueOption option, const std::string& value) {
    valueOptions[(int) option] = value;
}

const std::string& Options::get(ValueOption option) {
    return valueOptions[(int) option];
}
