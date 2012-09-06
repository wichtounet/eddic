//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef WARNINGS_ENGINE_H
#define WARNINGS_ENGINE_H

#include <memory>

#include "ast/source_def.hpp"

#include "Options.hpp"

namespace eddic {

namespace ast {

void checkForWarnings(SourceFile& program, std::shared_ptr<Configuration> configuration);

} //end of ast

} //end of eddic

#endif
