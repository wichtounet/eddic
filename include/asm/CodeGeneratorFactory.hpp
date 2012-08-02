//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CODE_GENERATOR_FACTORY_H
#define CODE_GENERATOR_FACTORY_H

#include <memory>

#include "Platform.hpp"
#include "GlobalContext.hpp"

#include "asm/CodeGenerator.hpp"

namespace eddic {

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
    std::shared_ptr<CodeGenerator> get(Platform platform, AssemblyFileWriter& writer, std::shared_ptr<GlobalContext> context);
};

} //end of as

} //end of eddic

#endif
