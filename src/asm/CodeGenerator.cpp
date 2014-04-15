//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "asm/CodeGenerator.hpp"

using namespace eddic;

as::CodeGenerator::CodeGenerator(AssemblyFileWriter& w, mtac::Program& program) : writer(w), program(program) {
    //Nothing to init
}
