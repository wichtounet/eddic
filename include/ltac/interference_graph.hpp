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

    private:
        bit_matrix matrix;
};

} //end of ltac

} //end of eddic

#endif
