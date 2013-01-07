//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_BASIC_BLOCK_H
#define MTAC_BASIC_BLOCK_H

#include <vector>

#include "variant.hpp"

#include "mtac/forward.hpp"

#include "ltac/forward.hpp"

namespace eddic {

class FunctionContext;
class Context;

namespace mtac {

/*!
 * \class basic_block
 * \brief A basic block in the MTAC representation. 
 * The basic blocks of a function are maintained in a doubly linked list. 
 */
class basic_block {
    public:
        typedef std::vector<std::shared_ptr<mtac::Quadruple>>::iterator iterator;

        /*!
         * Create a new basic block with the given index. 
         * \param The index of the basic block
         */
        basic_block(int index);
        
        /*!
         * Return an iterator to the first statement. 
         * \return an iterator to the first statement.
         */
        iterator begin();
        
        /*!
         * Return an iterator one past the last statement. 
         * \return an iterator one past the last statement.
         */
        iterator end();

        /*!
         * Add a new statement to the basic block. 
         * \param statement The statement to add. 
         */
        void add(std::shared_ptr<mtac::Quadruple> statement);

        const int index;    /*!< The index of the block */
        unsigned int depth = 0;
        std::string label;  /*!< The label of the block */
        std::shared_ptr<FunctionContext> context = nullptr;     /*!< The context of the enclosing function. */

        std::vector<std::shared_ptr<mtac::Quadruple>> statements;    /*!< The MTAC statements inside the basic block. */
        
        std::vector<ltac::Statement> l_statements;  /*!< The LTAC statements inside the basic block. */

        /* Doubly-linked list  */

        std::shared_ptr<basic_block> next = nullptr;     /*!< The next basic block in the doubly-linked list. */
        std::shared_ptr<basic_block> prev = nullptr;     /*!< The previous basic block in the doubly-linked list. */

        /* Control Flow Graph */
        std::vector<std::shared_ptr<basic_block>> successors;
        std::vector<std::shared_ptr<basic_block>> predecessors;

        /* Dominance tree */
        
        std::shared_ptr<basic_block> dominator = nullptr;     /*!< The immediate dominator of this basic block. */
};

typedef std::shared_ptr<basic_block> basic_block_p;

std::ostream& operator<<(std::ostream& stream, basic_block& basic_block);
std::ostream& operator<<(std::ostream& stream, basic_block_p& basic_block);

mtac::basic_block::iterator begin(mtac::basic_block_p function);
mtac::basic_block::iterator end(mtac::basic_block_p function);

void pretty_print(mtac::basic_block_p block, std::ostream& stream);

} //end of mtac

} //end of eddic

#endif
