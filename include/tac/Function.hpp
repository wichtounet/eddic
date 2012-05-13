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
#include <list>

#include "SymbolTable.hpp"

namespace eddic {

class FunctionContext;

namespace tac {

template<typename BasicBlock, typename Statement>
class Function {
    public:
        Function(std::shared_ptr<FunctionContext> context, const std::string& name);

        typedef std::shared_ptr<BasicBlock> BlockPtr;

        std::shared_ptr<eddic::Function> definition;
        
        std::shared_ptr<FunctionContext> context;

        void add(Statement statement);

        BlockPtr currentBasicBlock();
        BlockPtr newBasicBlock();

        std::string getName() const;

        std::vector<Statement>& getStatements();
        std::list<BlockPtr>& getBasicBlocks();

    private:
        //Before being partitioned, the function has only statement
        std::vector<Statement> statements;
        
        //There is no basic blocks at the beginning
        std::list<BlockPtr> blocks;

        std::string name;
};

template<typename BasicBlock, typename Statement>
tac::Function<BasicBlock, Statement>::Function(std::shared_ptr<FunctionContext> c, const std::string& n) : context(c), name(n) {
    //Nothing to do   
}
        
template<typename BasicBlock, typename Statement>
void tac::Function<BasicBlock, Statement>::add(Statement statement){
    statements.push_back(statement);
}

template<typename BasicBlock, typename Statement>
std::shared_ptr<BasicBlock> tac::Function<BasicBlock, Statement>::currentBasicBlock(){
    assert(!blocks.empty());

    return blocks.back();
}

template<typename BasicBlock, typename Statement>
std::shared_ptr<BasicBlock> tac::Function<BasicBlock, Statement>::newBasicBlock(){
    blocks.push_back(std::make_shared<BasicBlock>(blocks.size() + 1));
    return blocks.back();
}   

template<typename BasicBlock, typename Statement>
std::string tac::Function<BasicBlock, Statement>::getName() const {
    return name;
}

template<typename BasicBlock, typename Statement>
std::vector<Statement>& tac::Function<BasicBlock, Statement>::getStatements(){
    return statements;
}

template<typename BasicBlock, typename Statement>
std::list<std::shared_ptr<BasicBlock>>& tac::Function<BasicBlock, Statement>::getBasicBlocks(){
    return blocks;
}

} //end of tac

} //end of eddic

#endif
