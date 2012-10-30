//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEPENDENCIES_RESOLVER_H
#define DEPENDENCIES_RESOLVER_H

#include "ast/source_def.hpp"

namespace eddic {

namespace parser {
    class SpiritParser;
}

namespace ast {

void resolveDependencies(ast::SourceFile& program, parser::SpiritParser& parser);

} //end of ast

} //end of eddic

#endif
