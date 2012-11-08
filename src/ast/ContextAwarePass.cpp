//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/ContextAwarePass.hpp"
#include "ast/SourceFile.hpp"

using namespace eddic;

void ast::ContextAwarePass::apply_program(ast::SourceFile& program, bool){
    context = program.Content->context;
}
