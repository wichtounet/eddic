//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef BACK_ENDS_H
#define BACK_ENDS_H

#include <memory>

namespace eddic {

class BackEnd;

/*!
 * \enum Output
 * \brief Output type of the Compiler. 
 * This output type allows to determine which backend has to be used. 
 */
enum class Output : unsigned int {
    NATIVE_EXECUTABLE
};

/*!
 * Return the appropriate back end for the given Output type. 
 * \param output The output type. 
 * \return The appropriate back end. 
 */
std::shared_ptr<BackEnd> get_back_end(Output output);

}

#endif
