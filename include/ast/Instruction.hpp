//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_INSTRUCTION_H
#define AST_INSTRUCTION_H

#include <boost/variant/variant.hpp>

#include "ast/Deferred.hpp"
#include "ast/Swap.hpp"
#include "ast/FunctionCall.hpp"
#include "ast/Assignment.hpp"
#include "ast/Declaration.hpp"

namespace eddic {

struct While;
typedef Deferred<While> ASTWhile; 

struct For;
typedef Deferred<For> ASTFor; 

struct If;
typedef Deferred<If> ASTIf;

struct Foreach;
typedef Deferred<Foreach> ASTForeach;

typedef boost::variant<
            ASTFunctionCall, 
            ASTSwap, 
            ASTDeclaration,
            ASTAssignment, 
            ASTIf, 
            ASTWhile, 
            ASTForeach, 
            ASTFor>
        ASTInstruction;

} //end of eddic

#include "ast/If.hpp"
#include "ast/While.hpp"
#include "ast/For.hpp"
#include "ast/Foreach.hpp"

#endif
