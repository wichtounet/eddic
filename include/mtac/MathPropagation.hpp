//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_MATH_PROPAGATION_H
#define MTAC_MATH_PROPAGATION_H

#include <memory>
#include <unordered_map>

#include "variant.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/Pass.hpp"
#include "mtac/Quadruple.hpp"

namespace eddic {

namespace mtac {

class MathPropagation {
    public:
        bool optimized = false;
        Pass pass;
        
        void clear();

        void operator()(mtac::Quadruple& quadruple);
    
        void collect(mtac::Argument* arg);
        void collect(boost::optional<mtac::Argument>& arg);

    private:
        std::unordered_map<std::shared_ptr<Variable>, std::reference_wrapper<mtac::Quadruple>> assigns;
        std::unordered_map<std::shared_ptr<Variable>, int> usage;
};

template<>
struct pass_traits<MathPropagation> {
    STATIC_CONSTANT(pass_type, type, pass_type::BB_TWO_PASS);
    STATIC_STRING(name, "math_propagation");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
