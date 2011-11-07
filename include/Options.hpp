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

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

namespace eddic {

extern po::variables_map options;

void parseOptions(int argc, const char* argv[]);
void printHelp();
void printVersion();



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
        static void setDefaults();
        static void set(BooleanOption option);
        static void unset(BooleanOption option);
        static bool isSet(BooleanOption option);
        static void set(ValueOption option, const std::string& value);
        static const std::string& get(ValueOption option);
};

} //end of eddic

#endif
