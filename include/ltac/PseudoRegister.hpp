//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_PSEUDO_REGISTER_H
#define LTAC_PSEUDO_REGISTER_H

#include <ostream>

namespace eddic {

namespace ltac {

/*!
 * \struct PseudoRegister
 * Represents a pseudo register in the LTAC Representation. 
 */
struct PseudoRegister {
    unsigned short reg;
    bool bound;
    unsigned short binding;

    PseudoRegister();
    PseudoRegister(unsigned short);
    PseudoRegister(unsigned short, unsigned short);
    
    explicit operator int();

    bool operator<(const PseudoRegister& rhs) const;
    bool operator>(const PseudoRegister& rhs) const;

    bool operator==(const PseudoRegister& rhs) const;
    bool operator!=(const PseudoRegister& rhs) const;
};

std::ostream& operator<<(std::ostream& out, const PseudoRegister& reg);

} //end of ltac

} //end of eddic

namespace std {
    template<>
    class hash<eddic::ltac::PseudoRegister> {
    public:
        size_t operator()(const eddic::ltac::PseudoRegister& val) const {
            return val.reg;
        }
    };
}

#endif
