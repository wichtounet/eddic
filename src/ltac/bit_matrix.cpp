//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
