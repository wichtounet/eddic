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
    std::unordered_map<mtac::basic_block_p, Domain> OUT;
    std::unordered_map<mtac::basic_block_p, Domain> IN;
    
    std::unordered_map<std::size_t, Domain> OUT_S;
    std::unordered_map<std::size_t, Domain> IN_S;
    
    std::unordered_map<ltac::Statement, Domain> OUT_LS;
    std::unordered_map<ltac::Statement, Domain> IN_LS;
};

enum class DataFlowType : unsigned int {
    Forward,        //Common forward data-flow problem in MTAC
    Backward,       //Common backward data-flow problem in MTAC
    Low_Forward,    //Common forward data-flow problem in LTAC
    Low_Backward    //Common backward data-flow problem in LTAC
};

template<typename ProblemDomain>
void intersection_meet(ProblemDomain& in, const ProblemDomain& out){
    //eddic_assert(!in.top() || !out.top(), "At least one lattice should not be a top element");

    if(in.top() && out.top()){
        typename ProblemDomain::Values values;
        ProblemDomain result(values);

        in = result;
    } else if(in.top()){
        in = out;
    } else if(out.top()){
        //in does not change
    } else {
        typename ProblemDomain::Values values;
        ProblemDomain result(values);

        auto it = in.begin();
        auto end = in.end();

        while(it != end){
            auto var = it->first;

            auto out_it = out.find(var);
            if(out_it != out.end()){
                auto value_in = it->second;
                auto value_out = out_it->second;

                if(!(value_in == value_out)){
                    it = in.erase(it);
                    continue;
                }
            }

            ++it;
        }
    }
}

} //end of mtac

} //end of eddic

#endif
