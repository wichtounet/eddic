//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LOGGING_H
#define LOGGING_H

#include "logging/logging.h"

using namespace ::logging;

#ifdef LOGGING_DISABLE

void configure_logging(){}

#else

void configure_logging(){
    ::logging::detail::Logger<Level>::logging()._level.l = Level::trace;
}

#endif

#endif
