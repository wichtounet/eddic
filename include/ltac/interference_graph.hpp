//=======================================================================
// Copyright Baptiste Wicht 2011.
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
#include "ltac/PseudoRegister.hpp"

namespace eddic {

namespace ltac {

typedef std::size_t reg;

class interference_graph {
    public:
        reg convert(const ltac::PseudoRegister& reg);
        ltac::PseudoRegister convert(const reg& reg);

        void gather(const ltac::PseudoRegister& reg);

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
        std::vector<ltac::PseudoRegister> index_to_pseudo; //Maps indices to pseudo regs

        std::unordered_map<ltac::PseudoRegister, std::size_t> pseudo_to_index; //Maps pseudo regs to indices
};

} //end of ltac

} //end of eddic

#endif
