//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PERFS_TIMER_H
#define PERFS_TIMER_H

#include <string>
#include <iostream>

#include "StopWatch.hpp"
#include "Options.hpp"

namespace eddic {

//TODO Move the impl stuff in cpp

class PerfsTimer {
    private:
        StopWatch timer;
        std::string name;

    public:
        PerfsTimer(const std::string& n) : name(n) {}
        
        ~PerfsTimer(){
            if(option_defined("perfs")){
                std::cout << name << " took " << timer.elapsed() << "ms" << std::endl;
            }
        }
};

} //end of eddic

#endif
