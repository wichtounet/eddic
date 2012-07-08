//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>

#include "SymbolTable.hpp"
#include "Options.hpp"

#include "mtac/inlining.hpp"

using namespace eddic;

std::size_t size_of(std::shared_ptr<mtac::Function> function){
    std::size_t size = 0;

    for(auto block : function->getBasicBlocks()){
        size += block->statements.size();
    }

    return size;
}

bool mtac::inline_functions(std::shared_ptr<mtac::Program> program){
    if(option_defined("fno-inline")){
        return false;
    }

    bool optimized = false;

    std::vector<std::shared_ptr<mtac::Function>> inlined;

    for(auto function : program->functions){
        //The main function cannot be inlined
        if(function->getName() == "main"){
            continue;
        }

        //function called once
        if(symbols.referenceCount(function->getName()) == 1){
            inlined.push_back(function); 
        } else {
            auto size = size_of(function);

            //Inline little functions
            if(size < 10){
                inlined.push_back(function);
            }
        }
    }

    for(auto function : inlined){
        std::cout << "inline " << function->getName() << std::endl;
    }

    return optimized;
}
