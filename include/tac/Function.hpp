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
#include <utility>

namespace eddic {

class FunctionContext;

namespace tac {

template<typename BasicBlock, typename Statement>
class Function {
    public:
        typedef typename std::shared_ptr<BasicBlock> BlockPtr;
        typedef typename std::list<BlockPtr> BlockList;
        typedef typename BlockList::iterator BlockIterator;

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

    private:
        //Before being partitioned, the function has only statement
        std::vector<Statement> statements;
        
        //There is no basic blocks at the beginning
        BlockList basic_blocks;

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

template<typename BB, typename S>
typename tac::Function<BB, S>::BlockPtr tac::Function<BB, S>::currentBasicBlock(){
    assert(!basic_blocks.empty());

    return basic_blocks.back();
}

template<typename BB, typename S>
typename tac::Function<BB, S>::BlockPtr tac::Function<BB, S>::newBasicBlock(){
    basic_blocks.push_back(std::make_shared<BB>(basic_blocks.size() + 1));
    basic_blocks.back()->context = context;
    return basic_blocks.back();
}   

template<typename BB, typename S>
std::string tac::Function<BB, S>::getName() const {
    return name;
}

template<typename BB, typename S>
std::vector<S>& tac::Function<BB, S>::getStatements(){
    return statements;
}

template<typename BB, typename S>
std::size_t tac::Function<BB, S>::size(){
    std::size_t size = 0;

    for(auto block : getBasicBlocks()){
        size += block->statements.size();
    }

    return size;
}

template<typename BB, typename S>
typename tac::Function<BB, S>::BlockList& tac::Function<BB, S>::getBasicBlocks(){
    return basic_blocks;
}

template<typename BB, typename S>
std::pair<typename tac::Function<BB,S>::BlockIterator, typename tac::Function<BB,S>::BlockIterator> tac::Function<BB,S>::blocks(){
    return std::make_pair(basic_blocks.begin(), basic_blocks.end());
}

} //end of tac

} //end of eddic

#endif
