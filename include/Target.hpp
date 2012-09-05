//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TARGET_MACROS_H
#define TARGET_MACROS_H

namespace eddic {

#ifdef __GNUG__
    #ifdef __LP64__
        static const bool Target64 = true;
    #else
        static const bool Target64 = false;
    #endif
    static const bool TargetDetermined = true;
#else
    static const bool Target64 = false;
    static const bool TargetDetermined = false;
#endif

} //end of eddic

#endif
