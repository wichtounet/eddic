//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_COMPILER_H
#define MTAC_COMPILER_H

#include "ast/source_def.hpp"

#include "mtac/forward.hpp"

namespace eddic {

struct StringPool;

namespace mtac {

struct Compiler {
    void compile(ast::SourceFile& source, std::shared_ptr<StringPool> pool, mtac::Program& program) const ;
};

} //end of mtac

} //end of eddic

#endif
