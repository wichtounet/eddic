//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_DATA_FLOW_DOMAIN_H
#define MTAC_DATA_FLOW_DOMAIN_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <map>
#include <set>

#include "Variable.hpp"

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
    
    const DomainValues& values() const {
        return *int_values;
    }

    bool top() const {
        return !int_values;
    }
};

template<typename Key, typename Value, typename Hasher, typename Equals>
struct Domain<std::unordered_map<Key, Value, Hasher, Equals>> {
    typedef std::unordered_map<Key, Value, Hasher, Equals> Values;
    
    boost::optional<Values> int_values;

    Domain(){
        //Nothing to init
    }

    Domain(Values values) : int_values(values){
        //Nothing to init
    }

    Domain(const Domain& rhs) = default;
    Domain& operator=(const Domain& rhs) = default;
    
    Domain(Domain&& rhs) = default;
    Domain& operator=(Domain&& rhs) = default;

    Values& values(){
        return *int_values;
    }
    
    const Values& values() const {
        return *int_values;
    }

    Value& operator[](const Key& key){
        assert(int_values);

        return (*int_values)[key];
    }
    
    typename Values::iterator begin(){
        assert(int_values);

        return (*int_values).begin();
    }

    typename Values::iterator end(){
        assert(int_values);

        return (*int_values).end();
    }

    typename Values::const_iterator begin() const {
        assert(int_values);

        return (*int_values).cbegin();
    }

    typename Values::const_iterator end() const {
        assert(int_values);

        return (*int_values).cend();
    }
    
    auto count(const Key& key) const -> decltype((*int_values).count(key)){
        assert(int_values);

        return (*int_values).count(key);
    }
    
    typename Values::const_iterator find(const Key& key) const {
        assert(int_values);

        return (*int_values).find(key);
    }

    typename Values::iterator erase(typename Values::iterator it){
        assert(int_values);
        
        return (*int_values).erase(it);
    }

    void erase(const Key& key){
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

template<typename Key, typename Hasher, typename Equals>
struct Domain<std::unordered_set<Key, Hasher, Equals>> {
    typedef std::unordered_set<Key, Hasher, Equals> Values;
    
    boost::optional<Values> int_values;

    Domain(){
        //Nothing to init
    }

    Domain(Values values) : int_values(values){
        //Nothing to init
    }

    Domain(const Domain& rhs) : int_values(rhs.int_values) {}
    Domain& operator=(const Domain& rhs){
        int_values = rhs.int_values;

        return *this;
    }

    Domain(Domain&& rhs) : int_values(std::move(rhs.int_values)) {}
    Domain& operator=(Domain&& rhs){
        int_values = std::move(rhs.int_values);

        return *this;
    }

    Values& values(){
        return *int_values;
    }
    
    const Values& values() const {
        return *int_values;
    }

    typename Values::iterator begin(){
        assert(int_values);

        return (*int_values).begin();
    }

    typename Values::iterator end(){
        assert(int_values);

        return (*int_values).end();
    }

    typename Values::const_iterator begin() const {
        assert(int_values);

        return (*int_values).cbegin();
    }

    typename Values::const_iterator end() const {
        assert(int_values);

        return (*int_values).cend();
    }
    
    auto count(const Key& key) const -> decltype((*int_values).count(key)){
        assert(int_values);

        return (*int_values).count(key);
    }
    
    typename Values::const_iterator find(const Key& key) const {
        assert(int_values);

        return (*int_values).find(key);
    }

    typename Values::iterator erase(typename Values::iterator it){
        assert(int_values);
        
        return (*int_values).erase(it);
    }

    std::size_t size(){
        assert(int_values);
        
        return (*int_values).size();
    }

    void erase(const Key& key){
        assert(int_values);

        (*int_values).erase(key);
    }
    
    void insert(const Key& key){
        assert(int_values);

        (*int_values).insert(key);
    }
    
    void clear(){
        assert(int_values);

        (*int_values).clear();
    }

    bool top() const {
        return !int_values;
    }
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& values){
    stream << "vector{";

    for(auto& value : values){
        stream << value << ", ";
    }

    return stream << "}";
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const std::list<T>& values){
    stream << "list{";

    for(auto& value : values){
        stream << value << ", ";
    }

    return stream << "}";
}

template<typename Key, typename Value, typename Hasher>
std::ostream& operator<<(std::ostream& stream, const std::unordered_map<Key, Value, Hasher>& values){
    stream << "map{";

    for(auto& value : values){
        stream << value.first << ":" << value.second << ", ";
    }

    return stream << "}";
}

template<typename Value, typename Hasher>
std::ostream& operator<<(std::ostream& stream, const std::unordered_set<Value, Hasher>& values){
    stream << "set{";

    for(auto& value : values){
        stream << value << ", ";
    }

    return stream << "}";
}

template<typename Value, typename Hasher>
std::ostream& operator<<(std::ostream& stream, const std::set<Value, Hasher>& values){
    stream << "set{";

    for(auto& value : values){
        stream << value << ", ";
    }

    return stream << "}";
}

template<typename DomainValues>
std::ostream& operator<<(std::ostream& stream, const Domain<DomainValues>& domain){
    if(domain.top()){
        return stream << "top";
    }

    return stream << domain.values();
}

} //end of mtac

} //end of eddic

#endif
