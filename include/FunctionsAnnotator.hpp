//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTIONS_ANNOTATOR_H
#define FUNCTIONS_ANNOTATOR_H

#include "ast/Program.hpp"

namespace eddic {

class FunctionTable;

struct FunctionsAnnotator {
   void annotate(ast::Program& program, FunctionTable& functionTable);
};

} //end of eddic

#endif
