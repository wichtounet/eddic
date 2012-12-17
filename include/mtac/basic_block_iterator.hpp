//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_BASIC_BLOCK_ITERATOR_H
#define MTAC_BASIC_BLOCK_ITERATOR_H

#include <iterator>

#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

template<typename BB>
class basic_block_base_iterator : public std::iterator<std::bidirectional_iterator_tag, BB> {
    public:
        basic_block_base_iterator(BB current, BB prev) : current(current), prev(prev) {}
        basic_block_base_iterator(const basic_block_base_iterator<BB>& it) : current(it.current), prev(it.prev) {}

        basic_block_base_iterator<BB>& operator++() {
            prev = current;
            current = current->next;
            return *this;
        }

        basic_block_base_iterator<BB> operator++(int) {
            basic_block_base_iterator<BB> tmp(*this); 
            operator++(); 
            return tmp;
        }
        
        basic_block_base_iterator<BB>& operator--() {
            current = prev;
            if(current){
                prev = current->prev;
            }
            return *this;
        }

        basic_block_base_iterator<BB> operator--(int) {
            basic_block_base_iterator tmp(*this); 
            operator--(); 
            return tmp;
        }

        bool operator==(const basic_block_base_iterator<BB>& rhs) const {
            return current == rhs.current;
        }

        bool operator!=(const basic_block_base_iterator<BB>& rhs) const {
            return current != rhs.current;
        }

        BB& operator*() {
            return current;
        }
            
    private:
        BB current;
        BB prev;
};

typedef basic_block_base_iterator<mtac::basic_block_p> basic_block_iterator;
typedef basic_block_base_iterator<mtac::basic_block_cp> basic_block_const_iterator;

} //end of mtac

} //end of eddic

#endif
