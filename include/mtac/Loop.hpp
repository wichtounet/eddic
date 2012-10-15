//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LOOP_H
#define MTAC_LOOP_H

#include <memory>
#include <vector>

namespace eddic {

namespace mtac {

class BasicBlock;

class Loop {
    public:
        typedef std::vector<std::shared_ptr<mtac::BasicBlock>>::iterator iterator;

        Loop(std::vector<std::shared_ptr<mtac::BasicBlock>> blocks);

        iterator begin();
        iterator end();

        int estimate();
        void set_estimate(int estimate);

    private:
        std::vector<std::shared_ptr<mtac::BasicBlock>> blocks;

        int m_estimate = -1;
};

Loop::iterator begin(std::shared_ptr<mtac::Loop> loop);
Loop::iterator end(std::shared_ptr<mtac::Loop> loop);

} //end of mtac

} //end of eddic

#endif
