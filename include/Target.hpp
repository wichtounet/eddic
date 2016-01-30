//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
