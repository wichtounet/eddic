//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

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
    for(auto& pass : passes){
        //A simple pass is only applied once to the whole program
        if(pass->is_simple()){
            pass->apply_program(program);
        } 
        //Normal pass are applied until all function and structures have been handled
        else {
            //The next passes will have to apply it again to fresh functions
            applied_passes.push_back(pass);


        }
    }
}
