//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LOGGING_H
#define LOGGING_H

#ifdef LOGGING_DISABLE

#define LOG if(false) log::emit

#else

#define LOG log::emit

#endif

#include "logging/logging.h"

using namespace ::logging;

namespace eddic {

void configure_logging(int level);

} //end of eddic

#endif
