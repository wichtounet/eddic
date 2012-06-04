//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>

#include "PlatformDescriptor.hpp"

namespace eddic {

/*!
 * \enum Platform
 * \brief A type of platform. 
 */
enum class Platform : unsigned int {
    INTEL_X86,
    INTEL_X86_64
};

/*!
 * The current platform. 
 */
extern Platform platform;

/*!
 * \brief Return the platform descriptor of the specified platform. 
 * \param platform The platform identifier
 * \return The platform descriptor of the given platform. 
 */
PlatformDescriptor* getPlatformDescriptor(Platform platform);

} //end of eddic

#endif
