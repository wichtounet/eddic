//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ASM_INTEL_ASSEMBLY_UTILS_H
#define ASM_INTEL_ASSEMBLY_UTILS_H

#include <vector>
#include <string>

namespace eddic {

class AssemblyFileWriter;

namespace as {

/*!
 * Save the given registers on the stack. Works only for int registers. 
 * \param The writer to use to output the instructions
 * \param registers The registers that are to be saved on the stack. 
 */
void save(AssemblyFileWriter& writer, const std::vector<std::string>& registers);

/*!
 * Restore the given registers from the stack. Works only for int registers. They are restored in their reverse 
 * order so that the same list can be used to save and restore the registers. 
 * \param The writer to use to output the instructions
 * \param registers The registers that are to be restored from the stack. 
 */
void restore(AssemblyFileWriter& writer, const std::vector<std::string>& registers);

} //end of as

} //end of eddic

#endif
