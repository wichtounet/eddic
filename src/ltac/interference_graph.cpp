//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/interference_graph.hpp"

using namespace eddic;

std::size_t ltac::interference_graph::size(){
    return m_size;
}
        
ltac::reg ltac::interference_graph::convert(const ltac::PseudoRegister& reg){
    return pseudo_to_index[reg];
}

ltac::PseudoRegister ltac::interference_graph::convert(const ltac::reg& reg){
    return index_to_pseudo[reg];
}

void ltac::interference_graph::gather(const ltac::PseudoRegister& reg){
    if(!pseudo_to_index.count(reg)){
        pseudo_to_index[reg] = index_to_pseudo.size();
        index_to_pseudo.push_back(reg);
        ++m_size;
    }
}

void ltac::interference_graph::build_graph(){
    degrees.resize(size());
    matrix = std::make_shared<ltac::bit_matrix>(size());
}

void ltac::interference_graph::add_edge(std::size_t i, std::size_t j){
    matrix->set(i, j);
    matrix->set(j, i);
}

void ltac::interference_graph::remove_node(std::size_t i){
    auto& matrix = *this->matrix;

    for(auto j : adjacency_vectors[i]){
        matrix.clear(i, j);
        matrix.clear(j, i);

        --degrees[j];
    }

    degrees[i] = 0;
}

std::size_t ltac::interference_graph::degree(std::size_t i){
    return adjacency_vectors[i].size();
}

std::vector<std::size_t>& ltac::interference_graph::neighbors(std::size_t i){
    return adjacency_vectors[i];
}

void ltac::interference_graph::build_adjacency_vectors(){
    adjacency_vectors.resize(size());

    auto& matrix = *this->matrix;

    for(std::size_t i = 0; i < size(); ++i){
        for(std::size_t j = 0; j < size(); ++j){
            if(i != j && matrix[i][j]){
                adjacency_vectors[i].push_back(j);
            }
        }
    }

    for(std::size_t i = 0; i < size(); ++i){
        degrees[i] = adjacency_vectors[i].size();
    }
}
