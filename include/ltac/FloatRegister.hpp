//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_FLOAT_REGISTER_H
#define LTAC_FLOAT_REGISTER_H

#include <ostream>

namespace eddic {

namespace ltac {

/*!
 * \struct FloatRegister
 * Represents a symbolic hard float register in the LTAC Representation. 
 */
struct FloatRegister {
    unsigned short reg;

    FloatRegister();
    FloatRegister(unsigned short);

    operator int();

    bool operator<(const FloatRegister& rhs) const;
    bool operator>(const FloatRegister& rhs) const;

    bool operator==(const FloatRegister& rhs) const;
    bool operator!=(const FloatRegister& rhs) const;
};

std::ostream& operator<<(std::ostream& out, const FloatRegister& reg);

} //end of ltac

} //end of eddic

namespace std {
    template<>
    class hash<eddic::ltac::FloatRegister> {
    public:
        size_t operator()(const eddic::ltac::FloatRegister& val) const {
            return val.reg;
        }
    };
}

#endif
