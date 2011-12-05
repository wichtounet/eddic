//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEFAULT_VALUES_H
#define DEFAULT_VALUES_H

#include "ast/source_def.hpp"

namespace eddic {

struct DefaultValues {
    void fill(ast::SourceFile& program) const ;
};

} //end of eddic

#endif
