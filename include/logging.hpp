//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
