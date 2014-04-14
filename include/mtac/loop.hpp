//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
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

/*!
 * \brief Defines a loop in MTAC IR. A loop is made of a set of basic blocks.
 */
class loop {
    public:
        typedef std::set<mtac::basic_block_p>::iterator iterator;

        /*!
         * \brief Construct a new loop
         * \param blocks The set of basic blocks inside the loop
         */
        loop(const std::set<mtac::basic_block_p>& blocks);

        /*!
         * \brief Return an iterator to the first basic block of the loop. 
         * \return an iterator to the first basic block of the loop.
         */
        iterator begin();

        /*!
         * \brief Return an iterator one past the last basic block of the loop. 
         * \return an iterator one past the last basic block of the loop.
         */
        iterator end();
        
        /*!
         * \brief The set of basic blocks forming the loop. 
         * \return The set of basic blocks forming the loop. 
         */
        std::set<mtac::basic_block_p>& blocks();
        
        /*!
         * \brief The set of basic blocks forming the loop. 
         * \return The set of basic blocks forming the loop. 
         */
        const std::set<mtac::basic_block_p>& blocks() const;

        /*!
         * \brief Indicate if the loop analysis has been able to estimate the number of iterations of the loop. 
         * \return true if the estimation of the loop is made, false otherwise.
         */
        bool has_estimate();

        /*!
         * \brief Return the number of times the loop iterates. 
         *
         * If has_estimate() returns false, this function may return any value. 
         * \return The number of iterations of the loop. 
         */
        long& estimate();

        /*!
         * \brief Return the initial value of the basic induction variable that is used to iterate. 
         *
         * If has_estimate() returns false, this function may return any value. 
         * \return The initial value of the basic induction variable.
         */
        long& initial_value();

        /*!
         * \brief Return the list of basic induction variables of the loop. 
         * \return the list of basic induction variables of the loop. 
         */
        InductionVariables& basic_induction_variables();

        /*!
         * \brief Return the list of dependent induction variables of the loop. 
         * \return the list of dependent induction variables of the loop. 
         */
        InductionVariables& dependent_induction_variables();

        /*!
         * \brief Test if the loop has a single exit
         * \return true if the loop has a single exit, false otherwise.
         */
        bool single_exit() const;

        /*!
         * \brief Find the basic block that is the entry to the loop.
         * \return The entry basic block. 
         */
        mtac::basic_block_p find_entry() const ;

        /*!
         * \brief Find the basic block that is the exit to the loop. 
         *
         * Works only if the loop has exactly one exit. 
         *
         * \return The exit basic block. 
         */
        mtac::basic_block_p find_exit() const ;

        /*!
         * \brief Find the preheader of the loop if it exists. 
         * \return The preheader basic block of the loop or nullptr if is has not been found. 
         */
        mtac::basic_block_p find_preheader() const ;

        /*!
         * \brief Find or create a preheader for the given loop. This function finds a preheader that is safe to 
         * move for the loop into. If a regular preheader is need, find_preheader is better suited. 
         *
         * \param function The function the loop is located in.
         * \param create Indicate if the preheader has to be created if it does not exists.
         * \return The preheader basic block of the loop or nullptr if is has not been found neither created. 
         */
        mtac::basic_block_p find_safe_preheader(mtac::Function& function, bool create) const ;

    private:
        InductionVariables biv;
        InductionVariables div;

        std::set<mtac::basic_block_p> m_blocks;

        long m_estimate = -1;
        long m_initial;
};

std::ostream& operator<<(std::ostream& stream, const mtac::loop& loop);

} //end of mtac

} //end of eddic

#endif
