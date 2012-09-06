//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONTEXT_ANNOTATOR_H
#define CONTEXT_ANNOTATOR_H

#include <memory>

#include "Options.hpp"
#include "Platform.hpp"

#include "ast/source_def.hpp"

namespace eddic {

namespace ast {

void defineContexts(ast::SourceFile& program, Platform platform, std::shared_ptr<Configuration> configuration);

} //end of ast

} //end of eddic

#endif
