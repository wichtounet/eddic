//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIONS_H
#define OPTIONS_H

#include <map>
#include <string>

namespace eddic {

enum class BooleanOption : unsigned int {
    OPTIMIZE_INTEGERS,
    OPTIMIZE_STRINGS,
    ASSEMBLY_ONLY, 

    COUNT
};

enum class ValueOption : unsigned int {
    OUTPUT, 

    COUNT
};

class Options {
    private:
        static bool booleanOptions[(int) BooleanOption::COUNT];
        static std::string valueOptions[(int) ValueOption::COUNT];
        
    public:
        static void setDefaults() {
            set(ValueOption::OUTPUT, "a.out");
        }

        static void set(BooleanOption option) {
            booleanOptions[(int) option] = true;
        }
        static void unset(BooleanOption option) {
            booleanOptions[(int) option] = false;
        }
        static bool isSet(BooleanOption option) {
            return booleanOptions[(int) option];
        }

        static void set(ValueOption option, std::string value) {
            valueOptions[(int) option] = value;
        }
        static std::string get(ValueOption option) {
            return valueOptions[(int) option];
        }
};

} //end of eddic

#endif
