//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_INLINING_H
#define MTAC_INLINING_H

#include <memory>

#include "Options.hpp"

#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

class Program;
    
struct inline_functions {
    std::shared_ptr<Configuration> configuration;

    void set_configuration(std::shared_ptr<Configuration> configuration);

    bool operator()(std::shared_ptr<mtac::Program> program);
};

template<>
struct pass_traits<inline_functions> {
    STATIC_CONSTANT(pass_type, type, pass_type::IPA);
    STATIC_STRING(name, "inline_functions");
    STATIC_CONSTANT(unsigned int, property_flags, PROPERTY_CONFIGURATION);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
