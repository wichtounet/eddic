//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_BIT_MATRIX_H
#define LTAC_BIT_MATRIX_H

#include <boost/dynamic_bitset.hpp>

namespace eddic {

namespace ltac {

class bit_matrix;

class sub_bit_matrix {
    public:
        sub_bit_matrix(bit_matrix& matrix, std::size_t i);
        bool operator[](std::size_t j);

    private:
        bit_matrix& matrix;
        std::size_t i;
};

class bit_matrix {
    public:
        bit_matrix(std::size_t size);

        void set(std::size_t i, std::size_t j);
        void clear(std::size_t i, std::size_t j);
        bool is_set(std::size_t i, std::size_t j);

        sub_bit_matrix operator[](std::size_t i);
        
    private:
        std::size_t size;
        boost::dynamic_bitset<> bitset;
};

} //end of ltac

} //end of eddic

#endif
