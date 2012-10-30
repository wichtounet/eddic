//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <unordered_map>
#include <memory>

#include <boost/any.hpp>

namespace eddic {

struct ConfigValue {
    bool defined;
    boost::any value;
};

struct Configuration {
    std::unordered_map<std::string, ConfigValue> values;

    /*!
     * Indicates if the given option has been defined. 
     * \param option_name The name of the option to test. 
     * \return true if the option has been defined, otherwise false. 
     */
    bool option_defined(const std::string& option_name);

    /*!
     * Return the value of the defined option. 
     * \param option_name The name of the option to search. 
     * \return the value of the given option. 
     */
    std::string option_value(const std::string& option_name);

    /*!
     * Return the int value of the defined option. 
     * \param option_name The name of the option to search. 
     * \return the int value of the given option. 
     */
    int option_int_value(const std::string& option_name);
};

/*!
 * \brief Parse the options of the command line filling the options. 
 * \param argc The number of argument
 * \param argv The arguments
 * \return the Configuration of the compilation. nullptr is returned if the options failed. 
 */
std::shared_ptr<Configuration> parseOptions(int argc, const char* argv[]);

/*!
 * \brief Print the help.
 */
void print_help();

/*!
 * \brief Print the usage.
 */
void print_version();

} //end of eddic

#endif
