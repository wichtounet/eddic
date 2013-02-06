//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LOOP_H
#define MTAC_LOOP_H

#include <memory>
#include <set>
#include <map>

#include "mtac/forward.hpp"

namespace eddic {

class Variable;

namespace mtac {

struct LinearEquation {
    std::size_t def;
    std::shared_ptr<Variable> i;
    int e;
    int d;
    bool generated;
    bool div;

    LinearEquation() : def(0), i(nullptr), e(0), d(0), generated(false), div(false){
        //Nothing to init
    }

    LinearEquation(std::size_t def, std::shared_ptr<Variable> i, int e, int d, bool generated, bool div = false) : def(def), i(i), e(e), d(d), generated(generated), div(div) {
        //Nothing
    }
};

typedef std::map<std::shared_ptr<Variable>, LinearEquation> InductionVariables;

class Loop {
    public:
        typedef std::set<mtac::basic_block_p>::iterator iterator;

        Loop(const std::set<mtac::basic_block_p>& blocks);

        iterator begin();
        iterator end();
        
        std::set<mtac::basic_block_p>& blocks();
        const std::set<mtac::basic_block_p>& blocks() const;

        bool has_estimate();
        long& estimate();
        long& initial_value();

        InductionVariables& basic_induction_variables();
        InductionVariables& dependent_induction_variables();

    private:
        InductionVariables biv;
        InductionVariables div;

        std::set<mtac::basic_block_p> m_blocks;

        long m_estimate = -1;
        long m_initial;
};

/*!
 * \brief Find the basic block that is the entry to the Loop.
 * \param loop The loop. 
 * \return The entry basic block. 
 */
mtac::basic_block_p find_entry(mtac::Loop& loop);

/*!
 * \brief Find the basic block that is the exit to the Loop. 
 * \param loop The loop.
 * \return The exit basic block. 
 */
mtac::basic_block_p find_exit(mtac::Loop& loop);

/*!
 * \brief Find or create a preheader for the given loop. 
 * \param loop The loop to find the preheader for.
 * \param function The function the loop is located in.
 * \param create Indicate if the preheader has to be created if it does not exists.
 * \return The preheader basic block of the Loop or nullptr if is has not been found neither created. 
 */
mtac::basic_block_p find_pre_header(mtac::Loop& loop, mtac::Function& function, bool create);

std::ostream& operator<<(std::ostream& stream, const mtac::Loop& loop);

} //end of mtac

} //end of eddic

#endif
