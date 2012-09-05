//=======================================================================
// Copyright Baptiste Wicht 2011.
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

#include "ltac/Program.hpp"

#include "mtac/Program.hpp"

namespace eddic {

namespace ltac {

/*!
 * \class Compiler
 * Low-Level Compiler. Generates LTAC program from the MTAC Program. 
 */
class Compiler {
    public:
        /*!
         * Compile the MTAC Program into an LTAC Program. 
         * \param source The source MTAC Program. 
         * \param target The target LTAC Program. 
         * \param platform The target platform. 
         * \param float_pool The float pool to use. 
         */
        void compile(std::shared_ptr<mtac::Program> source, std::shared_ptr<ltac::Program> target, std::shared_ptr<FloatPool> float_pool, Platform platform);
    
    private:
        void compile(std::shared_ptr<mtac::Function> src_function, std::shared_ptr<ltac::Function> target_function, std::shared_ptr<FloatPool> float_pool, Platform platform);

        std::unordered_set<std::shared_ptr<mtac::BasicBlock>> block_usage;
};

} //end of ltac

} //end of eddic

#endif
