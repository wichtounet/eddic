//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
