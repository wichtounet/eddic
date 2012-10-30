//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_DATA_FLOW_PROBLEM_H
#define MTAC_DATA_FLOW_PROBLEM_H

#include <memory>

#include "mtac/forward.hpp"
#include "mtac/DataFlowDomain.hpp"

#include "ltac/forward.hpp"

namespace eddic {

namespace mtac {

template<typename Domain>
struct DataFlowResults {
    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, Domain> OUT;
    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, Domain> IN;
    
    std::unordered_map<mtac::Statement, Domain> OUT_S;
    std::unordered_map<mtac::Statement, Domain> IN_S;
    
    std::unordered_map<ltac::Statement, Domain> OUT_LS;
    std::unordered_map<ltac::Statement, Domain> IN_LS;
};

enum class DataFlowType : unsigned int {
    Forward,        //Common forward data-flow problem in MTAC
    Backward,       //Common backward data-flow problem in MTAC
    Low_Forward,    //Common forward data-flow problem in LTAC
    Low_Backward    //Common backward data-flow problem in LTAC
};

template<DataFlowType Type, typename DomainValues>
struct DataFlowProblem {
    typedef Domain<DomainValues> ProblemDomain;

    virtual ProblemDomain Boundary(std::shared_ptr<mtac::Function> function);
    virtual ProblemDomain Init(std::shared_ptr<mtac::Function> function);

    virtual ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) = 0;
    virtual ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in) = 0;
    virtual ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, ltac::Statement& statement, ProblemDomain& in) = 0;

    virtual bool optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> results) = 0;
    virtual bool optimize(ltac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> results) = 0;

    ProblemDomain top_element(){
        return ProblemDomain();
    }

    ProblemDomain default_element(){
        return ProblemDomain(DomainValues());
    }
};

template<DataFlowType Type, typename DomainValues>
auto DataFlowProblem<Type, DomainValues>::Boundary(std::shared_ptr<mtac::Function>/* function*/) -> ProblemDomain {
    //By default, return the default element
    return default_element();
}

template<DataFlowType Type, typename DomainValues>
auto DataFlowProblem<Type, DomainValues>::Init(std::shared_ptr<mtac::Function>/* function*/) -> ProblemDomain {
    //By default, return the top element
    return top_element();
}

template<typename ProblemDomain>
ProblemDomain intersection_meet(ProblemDomain& in, ProblemDomain& out){
    //ASSERT(!in.top() || !out.top(), "At least one lattice should not be a top element");

    if(in.top() && out.top()){
        typename ProblemDomain::Values values;
        ProblemDomain result(values);

        return result;
    } else if(in.top()){
        return out;
    } else if(out.top()){
        return in;
    } else {
        typename ProblemDomain::Values values;
        ProblemDomain result(values);

        auto it = in.begin();
        auto end = in.end();

        while(it != end){
            auto var = it->first;

            if(out.find(var) != out.end()){
                auto value_in = it->second;
                auto value_out = out[var];

                if(value_in == value_out){
                    result[var] = value_in;
                }
            }

            ++it;
        }

        return result;
    }
}

} //end of mtac

} //end of eddic

#endif
