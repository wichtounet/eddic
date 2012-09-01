//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "logging.hpp"

#ifdef LOGGING_DISABLE

void eddic::configure_logging(int){}

#else

void eddic::configure_logging(int level){
    if(level == 0){
        ::logging::detail::Logger<Level>::logging()._level.l = Level::disable;
    } else if(level == 1){
        ::logging::detail::Logger<Level>::logging()._level.l = Level::info;
    } else if(level == 2){
        ::logging::detail::Logger<Level>::logging()._level.l = Level::trace;
    } else if(level == 3){
        ::logging::detail::Logger<Level>::logging()._level.l = Level::debug;
    } else {
        ASSERT_PATH_NOT_TAKEN("Invalid log level");
    }
}

#endif
