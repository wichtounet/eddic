//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include <memory>
#include <boost/variant/variant.hpp>

#include "VariableChecker.hpp"

#include "ast/Program.hpp"

using namespace eddic;

class CheckerVisitor : public boost::static_visitor<> {
    public:
        void operator()(ASTProgram& program){
            for_each(program.blocks.begin(), program.blocks.end(), 
                [&](FirstLevelBlock& block){ boost::apply_visitor(*this, block); });
        }
   
        void operator()(ASTFunctionDeclaration& function){
//            for_each(function.instructions.begin(), function.instructions.end(), 
//                [&](ASTInstruction& instruction){ boost::apply_visitor(*this, instruction); });
        }

        void operator()(GlobalVariableDeclaration& function){
            
        } 
};

void VariableChecker::check(ASTProgram& program){
   CheckerVisitor visitor;
   visitor(program); 
}
