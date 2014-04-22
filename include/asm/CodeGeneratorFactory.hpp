//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CODE_GENERATOR_FACTORY_H
#define CODE_GENERATOR_FACTORY_H

#include <memory>

#include "Platform.hpp"

#include "asm/CodeGenerator.hpp"

namespace eddic {

struct GlobalContext;

namespace as {

/*!
 * \struct CodeGeneratorFactory
 * A factory to create code generator based on the target platform. 
 */
struct CodeGeneratorFactory {
    /*!
     * Create a code generator for the givne platform. 
     * \param platform The target platform. 
     * \param writer The assembly file writer to use. 
     * \return A pointer to the code generator corresponding to the platform. 
     */
    std::unique_ptr<CodeGenerator> get(Platform platform, AssemblyFileWriter& writer, mtac::Program& program, std::shared_ptr<GlobalContext> context);
};

} //end of as

} //end of eddic

#endif
