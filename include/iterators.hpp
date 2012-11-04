//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ITERATORS_H
#define ITERATORS_H

namespace eddic {
    
template<typename Container>
struct Iterators {
    typedef typename Container::iterator Iterator;

    Container& container;

    Iterator it;
    Iterator end;

    Iterators(Container& container) : container(container), it(container.begin()), end(container.end()) {}

    auto operator*() -> decltype(*it) {
        return *it;
    }

    void operator++(){
        ++it;
    }
    
    void operator--(){
        --it;
    }

    template<typename T>
    void insert(T&& value){
        it = container.insert(it, value);
        end = container.end();
    }

    void erase(){
        it = container.erase(it);
        end = container.end();
    }

    bool has_next(){
        return it != end;
    }
    
    bool has_previous(){
        return it != container.begin();
    }

    void update(){
        end = container.end();
    }
};

template<typename Container>
Iterators<Container> iterate(Container& container){
    Iterators<Container> iterators(container);

    return iterators;
}

}

#endif
