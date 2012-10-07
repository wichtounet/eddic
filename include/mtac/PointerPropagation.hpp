//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_POINTER_PROPAGATION_H
#define MTAC_POINTER_PROPAGATION_H

#include <memory>
#include <unordered_map>

#include "variant.hpp"

#include "mtac/Quadruple.hpp"
#include "mtac/Param.hpp"
#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

class PointerPropagation : public boost::static_visitor<> {
    public:
        bool optimized = false;

        void clear();

        void operator()(std::shared_ptr<mtac::Quadruple> quadruple);
        void operator()(std::shared_ptr<mtac::Param> param);

        template<typename T>
        void operator()(T&) const { 
            //Nothing to optimize here
        }

    private:
        std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> aliases;
        std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> pointer_copies;
};

template<>
struct pass_traits<PointerPropagation> {
    STATIC_CONSTANT(pass_type, type, pass_type::BB);
    STATIC_STRING(name, "pointer_propagation");
    STATIC_CONSTANT(bool, need_pool, false);
    STATIC_CONSTANT(bool, need_platform, false);
    STATIC_CONSTANT(bool, need_configuration, false);
    STATIC_CONSTANT(unsigned int, todo_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
