//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_TEMPORARY_ALLOCATOR_H
#define MTAC_TEMPORARY_ALLOCATOR_H

#include "Platform.hpp"

#include "mtac/Program.hpp"
#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

struct allocate_temporary {
    Platform platform;

    void set_platform(Platform platform);

    bool operator()(std::shared_ptr<mtac::Program> program);
};

template<>
struct pass_traits<allocate_temporary> {
    STATIC_CONSTANT(pass_type, type, pass_type::IPA);
    STATIC_STRING(name, "allocate_temporary");
    STATIC_CONSTANT(unsigned int, property_flags, PROPERTY_PLATFORM);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
