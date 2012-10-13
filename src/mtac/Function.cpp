//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "logging.hpp"

#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"

using namespace eddic;

mtac::Function::Function(std::shared_ptr<FunctionContext> c, const std::string& n) : context(c), name(n) {
    //Nothing to do   
}

mtac::basic_block_iterator mtac::Function::begin(){
    return basic_block_iterator(entry, nullptr);
}

mtac::basic_block_iterator mtac::Function::end(){
    return basic_block_iterator(nullptr, exit);    
}

mtac::basic_block_iterator mtac::Function::at(std::shared_ptr<BasicBlock> bb){
    if(bb){
        return basic_block_iterator(bb, bb->prev);
    } else {
        return basic_block_iterator(nullptr, nullptr);
    }
}
        
std::shared_ptr<mtac::BasicBlock> mtac::Function::entry_bb(){
    return entry;
}

std::shared_ptr<mtac::BasicBlock> mtac::Function::exit_bb(){
    return exit;
}

void mtac::Function::add(Statement statement){
    statements.push_back(statement);
}

std::shared_ptr<mtac::BasicBlock> mtac::Function::current_bb(){
    return exit;
}
        
void mtac::Function::create_entry_bb(){
    ++count;

    auto new_block = std::make_shared<mtac::BasicBlock>(-1);
    new_block->context = context;

    entry = exit = new_block;
}

void mtac::Function::create_exit_bb(){
    ++count;

    auto new_block = std::make_shared<mtac::BasicBlock>(-2);
    new_block->context = context;
    
    exit->next = new_block;
    new_block->prev = exit;

    exit = new_block;
}

std::shared_ptr<mtac::BasicBlock> mtac::Function::new_bb(){
    auto bb = std::make_shared<mtac::BasicBlock>(++index);
    bb->context = context;
    return bb;
}

std::shared_ptr<mtac::BasicBlock> mtac::Function::append_bb(){
    auto new_block = new_bb();
    ++count;
    
    exit->next = new_block;
    new_block->prev = exit;

    exit = new_block;

    return new_block;
}   
        
mtac::basic_block_iterator mtac::Function::insert_before(mtac::basic_block_iterator it, std::shared_ptr<mtac::BasicBlock> block){
    auto bb = *it;

    ASSERT(block, "Cannot add null block"); 
    ASSERT(it != begin(), "Cannot add before entry");

    block->context = context;
    
    ++count;

    block->prev = bb->prev;
    block->next = bb;
    bb->prev->next = block;
    bb->prev = block;
    
    return at(block);
}

mtac::basic_block_iterator mtac::Function::remove(std::shared_ptr<mtac::BasicBlock> block){
    ASSERT(block, "Cannot remove null block"); 
    ASSERT(block != exit, "Cannot remove exit"); 

    log::emit<Debug>("CFG") << "Remove basic block B" << block->index << log::endl;

    auto& next = block->next;

    --count;

    for(auto& succ : block->successors){
        auto it = iterate(succ->predecessors);

        while(it.has_next()){
            auto pred = *it;

            if(pred == block){
                it.erase();
            } else {
                ++it;
            }
        }
    }
    
    for(auto& pred : block->predecessors){
        auto it = iterate(pred->successors);

        while(it.has_next()){
            auto succ = *it;

            if(succ == block){
                it.erase();
            } else {
                ++it;
            }
        }

        //If there is a Fall through edge, redirect it
        if(pred = block->prev){
            mtac::make_edge(pred, block->next);
        }
    }

    block->successors.clear();
    block->predecessors.clear();

    block->prev->next = next;
    next->prev = block->prev;

    block->prev = nullptr;
    block->next = nullptr;

    return at(next);
}

mtac::basic_block_iterator mtac::Function::remove(mtac::basic_block_iterator it){
    return remove(*it);
}

std::string mtac::Function::getName() const {
    return name;
}

std::vector<mtac::Statement>& mtac::Function::getStatements(){
    return statements;
}

std::size_t mtac::Function::bb_count(){
    return count;
}

std::size_t mtac::Function::size(){
    std::size_t size = 0;

    for(auto& block : *this){
        size += block->statements.size();
    }

    return size;
}

std::pair<mtac::basic_block_iterator, mtac::basic_block_iterator> mtac::Function::blocks(){
    return std::make_pair(begin(), end());
}

mtac::basic_block_iterator mtac::begin(std::shared_ptr<mtac::Function> function){
    return function->begin();
}

mtac::basic_block_iterator mtac::end(std::shared_ptr<mtac::Function> function){
    return function->end();
}
