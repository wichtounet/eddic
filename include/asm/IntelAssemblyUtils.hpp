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

void save(AssemblyFileWriter& writer, const std::vector<std::string>& registers);
void restore(AssemblyFileWriter& writer, const std::vector<std::string>& registers);

} //end of tac

} //end of eddic

#endif
