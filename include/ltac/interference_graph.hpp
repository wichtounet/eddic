//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_INTERFERENCE_GRAPH_H
#define LTAC_INTERFERENCE_GRAPH_H

#include <memory>
#include <vector>
#include <unordered_map>

#include "ltac/bit_matrix.hpp"

namespace eddic {

namespace ltac {

typedef std::size_t reg;

template<typename Pseudo>
class interference_graph {
    public:
        reg convert(const Pseudo& reg);
        Pseudo convert(const reg& reg);

        void gather(const Pseudo& reg);

        void add_edge(reg i, reg j);
        void remove_node(reg i);
        bool connected(reg i, reg j);

        std::size_t degree(reg i);
        std::size_t& spill_cost(reg i);
        std::vector<reg>& neighbors(reg i);

        void build_graph();
        void build_adjacency_vectors();
        std::size_t size();

    private:
        std::size_t m_size = 0;

        std::shared_ptr<bit_matrix> matrix;
        
        //For each pseudo reg
        std::vector<std::vector<reg>> adjacency_vectors;
        std::vector<std::size_t> degrees;
        std::vector<std::size_t> spill_costs;
        std::vector<Pseudo> index_to_pseudo; //Maps indices to pseudo regs

        std::unordered_map<Pseudo, std::size_t> pseudo_to_index; //Maps pseudo regs to indices
};

template<typename Pseudo>
std::size_t ltac::interference_graph<Pseudo>::size(){
    return m_size;
}
        
template<typename Pseudo>
ltac::reg ltac::interference_graph<Pseudo>::convert(const Pseudo& reg){
    return pseudo_to_index[reg];
}

template<typename Pseudo>
Pseudo ltac::interference_graph<Pseudo>::convert(const ltac::reg& reg){
    return index_to_pseudo[reg];
}

template<typename Pseudo>
void ltac::interference_graph<Pseudo>::gather(const Pseudo& reg){
    if(!pseudo_to_index.count(reg)){
        pseudo_to_index[reg] = index_to_pseudo.size();
        index_to_pseudo.push_back(reg);
        ++m_size;
    }
}

template<typename Pseudo>
void ltac::interference_graph<Pseudo>::build_graph(){
    degrees.resize(size());
    spill_costs.resize(size());

    matrix = std::make_shared<ltac::bit_matrix>(size());
}

template<typename Pseudo>
void ltac::interference_graph<Pseudo>::add_edge(std::size_t i, std::size_t j){
    matrix->set(i, j);
    matrix->set(j, i);
}

template<typename Pseudo>
bool ltac::interference_graph<Pseudo>::connected(reg i, reg j){
    return matrix->is_set(i, j);
}

template<typename Pseudo>
void ltac::interference_graph<Pseudo>::remove_node(std::size_t i){
    auto& matrix = *this->matrix;

    for(auto j : adjacency_vectors[i]){
        matrix.clear(i, j);
        matrix.clear(j, i);

        --degrees[j];
    }

    degrees[i] = 0;
}

template<typename Pseudo>
std::size_t ltac::interference_graph<Pseudo>::degree(std::size_t i){
    return degrees[i];
}

template<typename Pseudo>
std::size_t& ltac::interference_graph<Pseudo>::spill_cost(reg i){
    return spill_costs[i];
}

template<typename Pseudo>
std::vector<std::size_t>& ltac::interference_graph<Pseudo>::neighbors(std::size_t i){
    return adjacency_vectors[i];
}

template<typename Pseudo>
void ltac::interference_graph<Pseudo>::build_adjacency_vectors(){
    adjacency_vectors.resize(size());

    auto& matrix = *this->matrix;

    for(std::size_t i = 0; i < size(); ++i){
        adjacency_vectors[i].clear();

        for(std::size_t j = 0; j < size(); ++j){
            if(i != j && matrix[i][j]){
                adjacency_vectors[i].push_back(j);
            }
        }
        
        degrees[i] = adjacency_vectors[i].size();
    }
}

} //end of ltac

} //end of eddic

#endif
