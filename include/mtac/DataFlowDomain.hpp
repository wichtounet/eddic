//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_DATA_FLOW_DOMAIN_H
#define MTAC_DATA_FLOW_DOMAIN_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>

namespace eddic {

namespace mtac {

template<typename DomainValues>
struct Domain {
    typedef DomainValues Values;

    boost::optional<DomainValues> int_values;

    Domain(){
        //Nothing to init
    }

    Domain(DomainValues values) : int_values(values){
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

    Domain(Values values) : int_values(values){
        //Nothing to init
    }

    Values& values(){
        return *int_values;
    }

    Value& operator[](Key key){
        assert(int_values);

        return (*int_values)[key];
    }

    auto begin() -> decltype((*int_values).begin()) {
        assert(int_values);

        return (*int_values).begin();
    }

    auto end() -> decltype((*int_values).end()) {
        assert(int_values);

        return (*int_values).end();
    }
    
    auto count(Key key) -> decltype((*int_values).count(key)) {
        assert(int_values);

        return (*int_values).count(key);
    }
    
    auto find(Key key) -> decltype((*int_values).find(key)) {
        assert(int_values);

        return (*int_values).find(key);
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

template<typename DomainValues>
std::ostream& operator<<(std::ostream& stream, Domain<DomainValues>& domain){
    if(domain.top()){
        return stream << "top";
    }

    return stream << domain.values();
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, std::vector<T>& values){
    stream << "vector{";

    for(auto& value : values){
        stream << value << ", ";
    }

    return stream << "}";
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, std::list<T>& values){
    stream << "list{";

    for(auto& value : values){
        stream << value << ", ";
    }

    return stream << "}";
}

template<typename Key, typename Value, typename Hasher>
std::ostream& operator<<(std::ostream& stream, std::unordered_map<Key, Value, Hasher>& values){
    stream << "map{";

    for(auto& value : values){
        stream << value.first << ":" << value.second << ", ";
    }

    return stream << "}";
}

template<typename Value, typename Hasher>
std::ostream& operator<<(std::ostream& stream, std::unordered_set<Value, Hasher>& values){
    stream << "set{";

    for(auto& value : values){
        stream << value << ", ";
    }

    return stream << "}";
}

} //end of mtac

} //end of eddic

#endif
