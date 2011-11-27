//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLES_ANNOTATOR_H
#define VARIABLES_ANNOTATOR_H

#include "ast/program_def.hpp"

namespace eddic {

struct VariablesAnnotator {
    void annotate(ast::Program& program) const ;
};

} //end of eddic

#endif
