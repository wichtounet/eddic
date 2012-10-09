//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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

} //end of eddic

#endif
