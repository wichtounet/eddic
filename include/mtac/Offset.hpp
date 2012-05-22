//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_OFFSET_H
#define MTAC_OFFSET_H

namespace eddic {

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

template <class T>
inline void hash_combine(std::size_t& seed, const T& v){
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct OffsetHash : std::unary_function<Offset, std::size_t> {
    std::size_t operator()(const Offset& p) const {
        std::size_t seed = 0;
        hash_combine(seed, p.variable);
        hash_combine(seed, p.offset);
        return seed;
    }
};

} //end of mtac

} //end of eddic

#endif
