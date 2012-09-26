//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/Pass.hpp"

using namespace eddic;

ast::Pass::Pass(ast::TemplateEngine& template_engine) : template_engine(template_engine) {}
