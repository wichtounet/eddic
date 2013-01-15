//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONSTANT_PROPAGATION_PROBLEM_H
#define MTAC_CONSTANT_PROPAGATION_PROBLEM_H

#include <unordered_map>
#include <memory>

#include "variant.hpp"
#include "assert.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/DataFlowProblem.hpp"
#include "mtac/EscapeAnalysis.hpp"

namespace eddic {

class Variable;

namespace mtac {

typedef boost::variant<std::string, double, int, std::shared_ptr<Variable>> ConstantValue;

class ConstantPropagationLattice {
    public:
        ConstantPropagationLattice(){}; //NAC
        ConstantPropagationLattice(ConstantValue value) : m_value(value) {}

        ConstantValue value() const {
            return *m_value;
        }

        bool constant() const {
            return m_value;
        }

        bool nac() const {
            return !constant();
        }

        void set_nac(){
            m_value = boost::none;
        }

    private:
        boost::optional<ConstantValue> m_value;
};

typedef std::unordered_map<std::shared_ptr<Variable>, ConstantPropagationLattice> ConstantPropagationValues;

struct ConstantPropagationProblem : public DataFlowProblem<DataFlowType::Forward, ConstantPropagationValues> {
    mtac::EscapedVariables pointer_escaped;
    
    ProblemDomain Boundary(mtac::Function& function) override;
    
    void meet(ProblemDomain& in, const ProblemDomain& out) override;

    ProblemDomain transfer(mtac::basic_block_p basic_block, mtac::Quadruple& statement, ProblemDomain& in) override;
    ProblemDomain transfer(mtac::basic_block_p, ltac::Statement&, ProblemDomain&) override { eddic_unreachable("Not LTAC"); };
    
    bool optimize(mtac::Function& function, std::shared_ptr<DataFlowResults<ProblemDomain>> results);
    bool optimize(ltac::Statement&, std::shared_ptr<DataFlowResults<ProblemDomain>>) override { eddic_unreachable("Not LTAC"); };
};

template<>
struct pass_traits<ConstantPropagationProblem> {
    STATIC_CONSTANT(pass_type, type, pass_type::DATA_FLOW);
    STATIC_STRING(name, "constant_propagation");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

std::ostream& operator<<(std::ostream& stream, const ConstantPropagationLattice& lattice);

} //end of mtac

} //end of eddic

#endif
