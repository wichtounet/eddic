//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_BASIC_BLOCK_H
#define MTAC_BASIC_BLOCK_H

#include <vector>

#include "variant.hpp"

#include "mtac/forward.hpp"
#include "mtac/Quadruple.hpp"

#include "ltac/forward.hpp"
#include "ltac/Instruction.hpp"

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
        typedef std::vector<mtac::Quadruple>::iterator iterator;
        typedef std::vector<mtac::Quadruple>::reverse_iterator reverse_iterator;

        /*!
         * Create a new basic block with the given index. 
         * \param index The index of the basic block
         */
        basic_block(int index);
        
        /*!
         * Return an iterator to the first statement. 
         * \return an iterator to the first statement.
         */
        iterator begin(){
            return statements.begin();
        }
        
        /*!
         * Return an iterator one past the last statement. 
         * \return an iterator one past the last statement.
         */
        iterator end(){
            return statements.end();
        }

        /*!
         * Add a new MTAC statement to the back of the basic block. 
         * \param statement The statement to add. 
         */
        inline void push_back(mtac::Quadruple&& statement){
            statements.push_back(std::forward<mtac::Quadruple>(statement));
        }
        
        /*!
         * Add a new LTAC statement to the back of the basic block. 
         * \param statement The statement to add. 
         */
        inline void push_back(ltac::Instruction&& statement){
            l_statements.push_back(std::forward<ltac::Instruction>(statement));
        }

        /*!
         * Add a new statement to the back of the basic block. The statements will be created in place.
         * \param args The args to to create the new statement. 
         */
        template< class... Args >
        inline void emplace_back( Args&&... args ){
            statements.emplace_back(std::forward<Args>(args)...);
        }
        
        /*!
         * Add a new LTAC statement to the back of the basic block. The statements will be created in place.
         * \param args The args to to create the new statement. 
         */
        template< class... Args >
        inline void emplace_back_low( Args&&... args ){
            l_statements.emplace_back(std::forward<Args>(args)...);
        }

        /*!
         * \brief Return the MTAC statement with the given UID. 
         *
         * Operation in O(n). The operation will fail if there are no MTAC statement with this UID in the basic block. 
         * \return A reference to the MTAC statement with the given UID in the basic block. 
         */
        mtac::Quadruple& find(std::size_t uid);

        /*!
         * \brief Return the LTAC statement with the given UID. 
         *
         * Operation in O(n). The operation will fail if there are no LTAC statement with this UID in the basic block. 
         * \return A reference to the LTAC statement with the given UID in the basic block. 
         */
        ltac::Instruction& find_low(std::size_t uid);

        /*!
         * \brief Return the number of MTAC statements of the basic blocks. 
         *
         * Operation in O(1)
         * \return the number of MTAC statements of the basic blocks. 
         */
        std::size_t size() const ;

        /*!
         * \brief Return the number of non-NOP MTAC statements of the basic blocks. 
         *
         * Operation in O(n)
         * \return the number of non-NOP MTAC statements of the basic blocks. 
         */
        std::size_t size_no_nop() const ;

        const int index;    /*!< The index of the block */
        unsigned int depth = 0;
        std::string label;  /*!< The label of the block */
        std::shared_ptr<FunctionContext> context = nullptr;     /*!< The context of the enclosing function. */

        std::vector<mtac::Quadruple> statements;    /*!< The MTAC statements inside the basic block. */
        
        std::vector<ltac::Instruction> l_statements;  /*!< The LTAC statements inside the basic block. */

        /* Doubly-linked list  */

        std::shared_ptr<basic_block> next = nullptr;     /*!< The next basic block in the doubly-linked list. */
        std::shared_ptr<basic_block> prev = nullptr;     /*!< The previous basic block in the doubly-linked list. */

        /* Control Flow Graph */
        std::vector<std::shared_ptr<basic_block>> successors;   //!< The basic block's predecessors in the CFG
        std::vector<std::shared_ptr<basic_block>> predecessors; //!< The basic block's successors in the CFG

        /* Dominance tree */
        
        std::shared_ptr<basic_block> dominator = nullptr;     /*!< The immediate dominator of this basic block. */
};

typedef std::shared_ptr<basic_block> basic_block_p;

std::ostream& operator<<(std::ostream& stream, const basic_block& basic_block);
std::ostream& operator<<(std::ostream& stream, const basic_block_p& basic_block);

mtac::basic_block::iterator begin(mtac::basic_block_p function);
mtac::basic_block::iterator end(mtac::basic_block_p function);

mtac::basic_block_p clone(mtac::Function& function, mtac::basic_block_p basic_block);

void pretty_print(std::shared_ptr<const mtac::basic_block> block, std::ostream& stream);

} //end of mtac

} //end of eddic

#endif
