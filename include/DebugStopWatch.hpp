//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEBUG_STOP_WATCH_H
#define DEBUG_STOP_WATCH_H

#include <string>
#include <iostream>

#include "StopWatch.hpp"

namespace eddic {

/*!
 * \class DebugStopWatch
 * \brief Simple stopwatch to keep track of elapsed time between instantiation and destruction.
 * 
 * If debug template argument is set to false, this class transforms to a no-op.  
 * 
 * \see StopWatch
 */
template<bool Enabled>
class DebugStopWatch {
    private:
        StopWatch timer;
        std::string name;

    public:
        DebugStopWatch(const std::string& n) : name(n) {}
        
        ~DebugStopWatch(){
            std::cout << name << " took " << timer.elapsed() << "s" << std::endl;
        }
};

template<>
struct DebugStopWatch<false> {
    DebugStopWatch(const std::string&) {}
};

} //end of eddic

#endif
