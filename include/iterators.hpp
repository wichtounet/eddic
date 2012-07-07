//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef UTILS_H
#define UTILS_H

namespace eddic {
    
template<typename Container>
struct Iterators {
    typedef typename Container::iterator Iterator;

    Iterator it;
    Iterator end;

    auto operator*() -> decltype(*it) {
        return *it;
    }

    void operator++(){
        ++it;
    }

    bool has_next(){
        return it != end;
    }
};

template<typename Container>
Iterators<Container> iterate(Container& container){
    Iterators<Container> iterators;

    iterators.it = container.begin();
    iterators.end = container.end();

    return iterators;
}

}

#endif
