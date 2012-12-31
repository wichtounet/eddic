//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STATISTICS_H
#define STATISTICS_H

#include <unordered_map>
#include <string>

namespace eddic {

class statistics {
    public:
        typedef std::unordered_map<std::string, std::size_t> Counters;
        typedef Counters::const_iterator iterator;

        void inc_counter(const std::string& a);
        std::size_t counter(const std::string& a) const;

        iterator begin() const;
        iterator end() const;

    private:
        Counters counters;
};

} //end of eddic

#endif
