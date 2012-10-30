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

class basic_block_iterator : public std::iterator<std::bidirectional_iterator_tag, basic_block_p> {
    public:
        basic_block_iterator(basic_block_p current, basic_block_p prev) : current(current), prev(prev) {}
        basic_block_iterator(const basic_block_iterator& it) : current(it.current), prev(it.prev) {}

        basic_block_iterator& operator++() {
            prev = current;
            current = current->next;
            return *this;
        }

        basic_block_iterator operator++(int) {
            basic_block_iterator tmp(*this); 
            operator++(); 
            return tmp;
        }
        
        basic_block_iterator& operator--() {
            current = prev;
            if(current){
                prev = current->prev;
            }
            return *this;
        }

        basic_block_iterator operator--(int) {
            basic_block_iterator tmp(*this); 
            operator--(); 
            return tmp;
        }

        bool operator==(const basic_block_iterator& rhs) const {
            return current == rhs.current;
        }

        bool operator!=(const basic_block_iterator& rhs) const {
            return current != rhs.current;
        }

        basic_block_p& operator*() {
            return current;
        }
            
    private:
        basic_block_p current;
        basic_block_p prev;
};

} //end of mtac

} //end of eddic

#endif
