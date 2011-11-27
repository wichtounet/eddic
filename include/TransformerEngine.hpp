//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TRANSFORMER_ENGINE_H
#define TRANSFORMER_ENGINE_H

#include "ast/program_def.hpp"

namespace eddic {

struct TransformerEngine {
    void transform(ast::Program& program) const;
};

} //end of eddic

#endif
