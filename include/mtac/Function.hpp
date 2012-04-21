//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_FUNCTION_H
#define TAC_FUNCTION_H

#include <string>
#include <memory>
#include <vector>

#include "SymbolTable.hpp"

#include "mtac/BasicBlock.hpp"

namespace eddic {

class FunctionContext;

namespace tac {

typedef std::shared_ptr<tac::BasicBlock> BlockPtr;

class Function {
    public:
        Function(std::shared_ptr<FunctionContext> context, const std::string& name);

        std::shared_ptr<eddic::Function> definition;
        
        std::shared_ptr<FunctionContext> context;

        void add(tac::Statement statement);

        BlockPtr currentBasicBlock();
        BlockPtr newBasicBlock();

        std::string getName() const;

        std::vector<tac::Statement>& getStatements();
        std::vector<BlockPtr>& getBasicBlocks();

    private:
        //Before being partitioned, the function has only statement
        std::vector<tac::Statement> statements;
        
        //There is no basic blocks at the beginning
        std::vector<BlockPtr> blocks;

        std::string name;
};

} //end of tac

} //end of eddic

#endif
