//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "iterators.hpp"
#include "logging.hpp"
#include "GlobalContext.hpp"

#include "mtac/remove_unused_functions.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Statement.hpp"

using namespace eddic;

namespace {

void remove_references(mtac::Function& function){
    for(auto& bb : function){
        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if((*ptr)->op == mtac::Operator::CALL){
                    --(*ptr)->function().references();
                }
            }
        }
    }
}

} //end of anonymous namespace

bool mtac::remove_unused_functions::operator()(mtac::Program& program){
    auto it = iterate(program.functions);

    while(it.has_next()){
        auto& function = *it;

        if(program.context->referenceCount(function.get_name()) == 0){
            remove_references(function);
            LOG<Debug>("Optimizer") << "Remove unused function " << function.get_name() << log::endl;
            it.erase();
            continue;
        } else if(program.context->referenceCount(function.get_name()) == 1 && mtac::is_recursive(function)){
            remove_references(function);
            LOG<Debug>("Optimizer") << "Remove unused recursive function " << function.get_name() << log::endl;
            it.erase();
            continue;
        } 

        ++it;
    }

    //Not necessary to restart the other passes
    return false;
}
