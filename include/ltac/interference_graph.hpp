//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_INTERFERENCE_GRAPH_H
#define LTAC_INTERFERENCE_GRAPH_H

#include "ltac/bit_matrix.hpp"

namespace eddic {

namespace ltac {

class interference_graph {
    public:
        interference_graph(std::size_t size);

        void add_edge(std::size_t i, std::size_t j);
        void remove_node(std::size_t i);

        std::size_t degree(std::size_t i);
        std::vector<std::size_t>& neighbors(std::size_t i);

        void build_adjacency_vectors();
        std::size_t size();

    private:
        std::size_t m_size;

        bit_matrix matrix;
        std::vector<std::vector<std::size_t>> adjacency_vectors;
        std::vector<std::size_t> degrees;
};

} //end of ltac

} //end of eddic

#endif
