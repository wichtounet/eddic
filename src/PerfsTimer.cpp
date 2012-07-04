//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "Options.hpp"
#include "PerfsTimer.hpp"

using namespace eddic;

PerfsTimer::PerfsTimer(const std::string& n) : name(n) {}
PerfsTimer::PerfsTimer(const std::string& n, bool precise) : name(n), precise(precise) {}

PerfsTimer::~PerfsTimer(){
    if(option_defined("perfs")){
        if(precise){
            std::cout << name << " took " << timer.micro_elapsed() << "us" << std::endl;
        } else {
            std::cout << name << " took " << timer.elapsed() << "ms" << std::endl;
        }
    }
}
