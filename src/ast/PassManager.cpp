//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "iterators.hpp"

#include "ast/PassManager.hpp"
#include "ast/Pass.hpp"
#include "ast/SourceFile.hpp"

//The passes
#include "ast/TransformerEngine.hpp"

using namespace eddic;

void ast::PassManager::init_passes(){
    //Clean pass
    passes.push_back(std::make_shared<ast::CleanPass>());
}

void ast::PassManager::run_passes(ast::SourceFile& program){
    auto it = iterate(passes);

    while(it.has_next()){
        auto pass = *it;

        pass->apply_program(program);

        if(!pass->is_simple()){
            applied_passes.push_back(pass);
        }

        ++it;
    }
}
