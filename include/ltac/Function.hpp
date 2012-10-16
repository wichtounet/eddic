//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_FUNCTION_H
#define LTAC_FUNCTION_H

#include "ltac/BasicBlock.hpp"
#include "ltac/Statement.hpp"

namespace eddic {

class Function;
class FunctionContext;

namespace ltac {

class Function {
    public:
        typedef std::shared_ptr<BasicBlock> BlockPtr;
        typedef std::list<BlockPtr> BlockList;
        typedef BlockList::iterator BlockIterator;

    public:
        std::shared_ptr<eddic::Function> definition;
        std::shared_ptr<FunctionContext> context;

        Function(std::shared_ptr<FunctionContext> context, const std::string& name);

        void add(Statement statement);

        std::string getName() const;

        BlockPtr current_bb();
        BlockPtr new_bb();
        BlockPtr new_bb_in_front();

        BlockList& basic_blocks();

    private:
        BlockList _basic_blocks;

        std::string name;
};

} //end of mtac

} //end of eddic

#endif
