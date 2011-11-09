//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIONS_H
#define OPTIONS_H

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

namespace eddic {

extern bool OptimizeIntegers;
extern bool OptimizeStrings;
extern po::variables_map options;

void parseOptions(int argc, const char* argv[]);
void printHelp();
void printVersion();

} //end of eddic

#endif
