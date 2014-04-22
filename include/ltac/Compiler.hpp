//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_COMPILER_H
#define LTAC_COMPILER_H

#include <memory>
#include <unordered_set>

#include "FloatPool.hpp"
#include "Platform.hpp"
#include "Options.hpp"

#include "mtac/forward.hpp"

namespace eddic {

namespace ltac {

/*!
 * \class Compiler
 * Low-Level Compiler. Generates LTAC program from the MTAC Program. 
 */
class Compiler {
    public:
        Compiler(Platform platform, std::shared_ptr<Configuration> configuration);

        /*!
         * Compile the MTAC Program into an LTAC Program. 
         * \param source The source MTAC Program. 
         * \param float_pool The float pool to use. 
         */
        void compile(mtac::Program& source, FloatPool& float_pool);
    
    private:
        void compile(mtac::Function& src_function, FloatPool& float_pool);

        std::unordered_set<mtac::basic_block_p> block_usage;
        Platform platform;
        std::shared_ptr<Configuration> configuration;
};

} //end of ltac

} //end of eddic

#endif
