//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef PERFS_TIMER_H
#define PERFS_TIMER_H

#include <string>

#include "StopWatch.hpp"

namespace eddic {

class PerfsTimer {
    public:
        PerfsTimer(const std::string& n);
        PerfsTimer(const std::string& n, bool precise);

        ~PerfsTimer();
    
    private:
        StopWatch timer;
        std::string name;
        bool precise = false;
};

} //end of eddic

#endif
