//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ast/ContextAwarePass.hpp"
#include "ast/SourceFile.hpp"

using namespace eddic;

void ast::ContextAwarePass::apply_program(ast::SourceFile& program, bool){
    context = program.Content->context;
}
