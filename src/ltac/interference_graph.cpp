//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/interference_graph.hpp"

using namespace eddic;

ltac::interference_graph::interference_graph(std::size_t size) : m_size(size), matrix(size), degrees(size, 0) {
    //Nothing
}

std::size_t ltac::interference_graph::size(){
    return m_size;
}
        
void ltac::interference_graph::add_edge(std::size_t i, std::size_t j){
    matrix.set(i, j);
    matrix.set(j, i);
}

void ltac::interference_graph::remove_node(std::size_t i){
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
