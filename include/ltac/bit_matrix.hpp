//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_BIT_MATRIX_H
#define LTAC_BIT_MATRIX_H

#include <boost/dynamic_bitset.hpp>

namespace eddic {

namespace ltac {

class bit_matrix;

struct sub_bit_matrix {
    unsigned int i;
    bit_matrix& matrix;

    bool operator[](std::size_t j);
};

class bit_matrix {
    public:
        bit_matrix(std::size_t size);

        bool set(int i, int j);
        bool clear(int i, int j);
        bool is_set(int i, int j);

        sub_bit_matrix operator[](std::size_t i);
        
    private:
        std::size_t size;
        boost::dynamic_bitset<> bitset;
};

} //end of ltac

} //end of eddic

#endif
