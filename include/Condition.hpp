//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONDITION_H
#define CONDITION_H

#include <memory>

#include "BooleanCondition.hpp"

namespace eddic {

class Value;

class Condition {
    private:
        BooleanCondition m_condition;
        std::shared_ptr<Value> m_lhs;
        std::shared_ptr<Value> m_rhs;

    public:
        Condition(BooleanCondition condition) : m_condition(condition), m_lhs(NULL), m_rhs(NULL) {}
        Condition(BooleanCondition condition, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : m_condition(condition), m_lhs(lhs), m_rhs(rhs) {}

        std::shared_ptr<Value> lhs() {
            return m_lhs;
        }
        std::shared_ptr<Value> rhs() {
            return m_rhs;
        }
        BooleanCondition condition() const {
            return m_condition;
        }
        bool isOperator() const {
            return m_lhs != NULL || m_rhs != NULL;
        }
};

} //end of eddic

#endif
