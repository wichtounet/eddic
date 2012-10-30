//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_COMPILER_H
#define LTAC_COMPILER_H

#include <memory>
#include <unordered_set>

#include "FloatPool.hpp"
#include "Platform.hpp"
#include "Options.hpp"

#include "mtac/Program.hpp"

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
         * \param target The target LTAC Program. 
         * \param float_pool The float pool to use. 
         */
        void compile(std::shared_ptr<mtac::Program> source, std::shared_ptr<FloatPool> float_pool);
    
    private:
        void compile(std::shared_ptr<mtac::Function> src_function, std::shared_ptr<FloatPool> float_pool);

        std::unordered_set<std::shared_ptr<mtac::BasicBlock>> block_usage;
        Platform platform;
        std::shared_ptr<Configuration> configuration;
};

} //end of ltac

} //end of eddic

#endif
