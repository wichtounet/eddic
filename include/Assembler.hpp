//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>

#include "Platform.hpp"

namespace eddic {

/*!
 * \brief Assemble and link the output.asm file and procude the output executable. 
 * \param platform The target platform. 
 * \param output The output file path.  
 * \param debug The debug mode flag. 
 * \param verbose The verbose mode flag. 
 */
void assemble(Platform platform, const std::string& s, const std::string& o, const std::string& output, bool debug, bool verbose);

void verify_dependencies();

} //end of eddic

#endif
