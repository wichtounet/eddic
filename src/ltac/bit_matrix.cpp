//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/bit_matrix.hpp"

using namespace eddic;

ltac::sub_bit_matrix::sub_bit_matrix(bit_matrix& matrix, std::size_t i) : matrix(matrix), i(i) {
    //Nothing
}

bool ltac::sub_bit_matrix::operator[](std::size_t j){
    return matrix.is_set(i, j);
}

ltac::bit_matrix::bit_matrix(std::size_t size) : size(size), bitset(size * size) {
   //Nothing 
}

void ltac::bit_matrix::set(std::size_t i, std::size_t j){
    bitset.set(i * size + j);
}

void ltac::bit_matrix::clear(std::size_t i, std::size_t j){
    bitset.set(i * size + j, false);
}

bool ltac::bit_matrix::is_set(std::size_t i, std::size_t j){
    return bitset[i * size + j];
}

ltac::sub_bit_matrix ltac::bit_matrix::operator[](std::size_t i){
    return {*this, i};
}
