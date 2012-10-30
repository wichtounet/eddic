//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_OFFSET_H
#define MTAC_OFFSET_H

#include <memory>
#include <iostream>

#include <boost/functional/hash.hpp>

namespace eddic {

class Variable;

namespace mtac {

struct Offset {
    std::shared_ptr<Variable> variable;
    int offset;

    Offset(std::shared_ptr<Variable> variable, int offset) : variable(variable), offset(offset){
        //Nothing else to init
    }

    bool operator==(const Offset& b) const {
        return variable == b.variable && offset == b.offset;
    }
};

std::ostream& operator<<(std::ostream& stream, const Offset& offset);

struct OffsetHash : std::unary_function<Offset, std::size_t> {
    std::size_t operator()(const Offset& p) const {
        std::size_t seed = 0;

        std::hash<std::shared_ptr<Variable>> hasher;
        boost::hash_combine(seed, hasher(p.variable));
        boost::hash_combine(seed, p.offset);
        return seed;
    }
};

} //end of mtac

} //end of eddic

#endif
