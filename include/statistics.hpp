//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
