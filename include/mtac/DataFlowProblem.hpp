//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_DATA_FLOW_PROBLEM_H
#define MTAC_DATA_FLOW_PROBLEM_H

#include <memory>

#include "mtac/BasicBlock.hpp"

namespace eddic {

namespace mtac {

template<typename DomainValues>
struct Domain {
    typedef DomainValues Values;

    boost::optional<DomainValues> int_values;

    Domain(){
        //Nothing to init
    }

    Domain(DomainValues values) : int_values(std::move(values)){
        //Nothing to init
    }

    DomainValues& values(){
        return *int_values;
    }

    bool top() const {
        return !int_values;
    }
};

template<typename Key, typename Value, typename Hasher>
struct Domain<std::unordered_map<Key, Value, Hasher>> {
    typedef std::unordered_map<Key, Value, Hasher> Values;
    
    boost::optional<Values> int_values;

    Domain(){
        //Nothing to init
    }

    Domain(Values values) : int_values(std::move(values)){
        //Nothing to init
    }

    Values& values(){
        return *int_values;
    }

    Value& operator[](Key key){
        assert(int_values);

        return (*int_values)[key];
    }

    typename Values::iterator begin(){
        assert(int_values);

        return (*int_values).begin();
    }
    
    typename Values::iterator find(Key key){
        assert(int_values);

        return (*int_values).find(key);
    }

    typename Values::iterator end(){
        assert(int_values);

        return (*int_values).end();
    }

    void erase(Key key){
        assert(int_values);

        (*int_values).erase(key);
    }
    
    void clear(){
        assert(int_values);

        (*int_values).clear();
    }

    bool top() const {
        return !int_values;
    }
};

template<typename Domain>
struct DataFlowResults {
    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, Domain> OUT;
    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, Domain> IN;
    
    std::unordered_map<mtac::Statement, Domain> OUT_S;
    std::unordered_map<mtac::Statement, Domain> IN_S;
};

template<bool Forward, typename DomainValues>
struct DataFlowProblem {
    typedef Domain<DomainValues> ProblemDomain;

    virtual ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) = 0;
    virtual ProblemDomain transfer(mtac::Statement& statement, ProblemDomain& in) = 0;

    virtual ProblemDomain Boundary();
    virtual ProblemDomain Init();

    virtual bool optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& results) = 0;

    ProblemDomain top_element(){
        return ProblemDomain();
    }

    ProblemDomain default_element(){
        return ProblemDomain(DomainValues());
    }
};

template<bool Forward, typename DomainValues>
typename DataFlowProblem<Forward, DomainValues>::ProblemDomain DataFlowProblem<Forward, DomainValues>::Boundary(){
    //By default, return the default element
    return default_element();
}

template<bool Forward, typename DomainValues>
typename DataFlowProblem<Forward, DomainValues>::ProblemDomain DataFlowProblem<Forward, DomainValues>::Init(){
    //By default, return the top element
    return top_element();
}

template<typename ProblemDomain>
ProblemDomain union_meet(ProblemDomain& in, ProblemDomain& out){
    ASSERT(!in.top() || !out.top(), "At least one lattice should not be a top element");

    if(in.top()){
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
