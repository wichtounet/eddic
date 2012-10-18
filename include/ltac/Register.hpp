//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_REGISTER_H
#define LTAC_REGISTER_H

#include <ostream>

#include <boost/functional/hash.hpp>

namespace eddic {

namespace ltac {

/*!
 * \struct Register
 * Represents a symbolic hard register in the LTAC Representation. 
 */
struct Register {
    unsigned short reg;

    Register();
    Register(unsigned short);
    
    explicit operator int();

    bool operator<(const Register& rhs) const;
    bool operator>(const Register& rhs) const;

    bool operator==(const Register& rhs) const;
    bool operator!=(const Register& rhs) const;
};

std::ostream& operator<<(std::ostream& out, const Register& reg);

/*!
 * Represent the stack pointer. 
 */
static const Register SP(1000);

/*!
 * Represent the base pointer. 
 */
static const Register BP(1001);

struct RegisterHash : std::unary_function<Register, std::size_t> {
    std::size_t operator()(const Register& r) const {
        std::size_t seed = 0;
        boost::hash_combine(seed, r.reg);
        return seed;
    }
};

} //end of ltac

} //end of eddic

#endif
