//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
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
    mtac::Quadruple& def;
    std::shared_ptr<Variable> i;
    int e;
    int d;
    bool generated;
};

typedef std::map<std::shared_ptr<Variable>, LinearEquation> InductionVariables;

class Loop {
    public:
        typedef std::set<mtac::basic_block_p>::iterator iterator;

        Loop(const std::set<mtac::basic_block_p>& blocks);

        iterator begin();
        iterator end();
        
        std::set<mtac::basic_block_p>& blocks();

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

} //end of mtac

} //end of eddic

#endif
