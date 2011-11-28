//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEBUG_TIMER_H
#define DEBUG_TIMER_H

#include <string>
#include <iostream>

namespace eddic {

template<bool Enabled>
class DebugTimer {
    private:
        Timer timer;
        std::string name;

    public:
        DebugTimer(const std::string& n) : name(n) {}
        
        ~DebugTimer(){
            std::cout << name << " took " << timer.elapsed() << "s" << std::endl;
        }
};

template<>
struct DebugTimer<false> {
    DebugTimer(const std::string&) {}
};

} //end of eddic

#endif
