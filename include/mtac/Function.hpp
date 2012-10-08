//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_FUNCTION_H
#define MTAC_FUNCTION_H

#include <memory>

#include "mtac/BasicBlock.hpp"
#include "mtac/Statement.hpp"
#include "mtac/ControlFlowGraph.hpp"

namespace eddic {

class FunctionContext;

namespace mtac {

class Function : public std::enable_shared_from_this<Function> {
    public:
        typedef std::shared_ptr<BasicBlock> BlockPtr;
        typedef std::list<BlockPtr> BlockList;
        typedef BlockList::iterator BlockIterator;

    public:
        Function(std::shared_ptr<FunctionContext> context, const std::string& name);

        std::shared_ptr<eddic::Function> definition;
        
        std::shared_ptr<FunctionContext> context;

        void add(Statement statement);

        BlockPtr currentBasicBlock();
        BlockPtr newBasicBlock();

        std::string getName() const;

        std::vector<Statement>& getStatements();
        BlockList& getBasicBlocks();

        std::pair<BlockIterator, BlockIterator> blocks();

        std::size_t size();
        
        std::shared_ptr<ControlFlowGraph> cfg();
        void invalidate_cfg();

    private:
        //Before being partitioned, the function has only statement
        std::vector<Statement> statements;
        
        //There is no basic blocks at the beginning
        BlockList basic_blocks;

        std::string name;

        std::shared_ptr<ControlFlowGraph> _cfg;
};

} //end of mtac

} //end of eddic

#endif
