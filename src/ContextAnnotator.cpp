//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ContextAnnotator.hpp"

#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "BlockContext.hpp"

using namespace eddic;

void ContextAnnotator::annotate(ASTProgram& program){
    //Create the global context
    globalContext = std::make_shared<GlobalContext>();
    currentContext = globalContext;

    //Annotate the program
    //Visit and annotate everything
}
