//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIONS_H
#define OPTIONS_H

#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

namespace eddic {

/*!
 * \brief Bolean flag indicating if warnings are outputted for unused declarations. 
 */
extern bool WarningUnused;

/*!
 * \brief Bolean flag indicating if warnings are outputted for casts.
 */
extern bool WarningCast;

/*!
 * \brief The program options. 
 */
extern po::variables_map options;

/*!
 * \brief Parse the options of the command line filling the options. 
 * \param argc The number of argument
 * \param argv The arguments
 * \return A boolean indicating if the options are valid (\c true) or not. 
 */
bool parseOptions(int argc, const char* argv[]);

/*!
 * \brief Print the help.
 */
void printHelp();

/*!
 * \brief Print the usage.
 */
void printVersion();

} //end of eddic

#endif
