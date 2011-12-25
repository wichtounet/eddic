//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant.hpp>

#include "tac/Optimizer.hpp"
#include "tac/Program.hpp"

using namespace eddic;

namespace {

struct ArithmeticIdentities : public boost::static_visitor<tac::Statement> {
    template<typename T>
    tac::Statement operator()(T& statement) const { 
        return statement;
    }
};
    
}

template<typename Visitor>
void apply_to_all(Visitor visitor, tac::Program& program){
    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                statement = boost::apply_visitor(visitor, statement);
            }
        }
    }
}

void tac::Optimizer::optimize(tac::Program& program) const {
    ArithmeticIdentities identities;
    apply_to_all(identities, program);
}
