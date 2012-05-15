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
    boost::optional<DomainValues> int_values;

    Domain(){
        //Nothing to init
    }

    Domain(DomainValues values) : int_values(values){ //TODO Not sure if it is really useful to have rvalues references here
        //Nothing to init
    }

    DomainValues& values(){
        return *int_values;
    }

    bool top() const {
        return !int_values;
    }
};

template<typename Key, typename Value>
struct Domain<std::unordered_map<Key, Value>> {
    boost::optional<std::unordered_map<Key, Value>> int_values;

    Domain(){
        //Nothing to init
    }

    Domain(std::unordered_map<Key, Value> values) : int_values(values){ //TODO Not sure if it is really useful to have rvalues references here
        //Nothing to init
    }

    std::unordered_map<Key, Value>& values(){
        return *int_values;
    }

    Value& operator[](Key key){
        assert(int_values);

        return (*int_values)[key];
    }

    typename std::unordered_map<Key, Value>::iterator begin(){
        assert(int_values);

        return (*int_values).begin();
    }
    
    typename std::unordered_map<Key, Value>::iterator find(Key key){
        assert(int_values);

        return (*int_values).find(key);
    }

    typename std::unordered_map<Key, Value>::iterator end(){
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

    virtual ProblemDomain Boundary() = 0;
    virtual ProblemDomain Init() = 0;

    virtual bool optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& results) = 0;

    ProblemDomain top_element(){
        return ProblemDomain();
    }

    ProblemDomain default_element(){
        return ProblemDomain(DomainValues());
    }
};

} //end of mtac

} //end of eddic

#endif
