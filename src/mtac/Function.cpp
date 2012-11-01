//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "logging.hpp"

#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Loop.hpp"

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
        
mtac::basic_block_p mtac::Function::entry_bb(){
    return entry;
}

mtac::basic_block_p mtac::Function::exit_bb(){
    return exit;
}

void mtac::Function::add(Statement statement){
    statements.push_back(statement);
}

mtac::basic_block_p mtac::Function::current_bb(){
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

mtac::basic_block_p mtac::Function::new_bb(){
    auto bb = std::make_shared<mtac::BasicBlock>(++index);
    bb->context = context;
    return bb;
}

mtac::basic_block_p mtac::Function::append_bb(){
    auto new_block = new_bb();
    ++count;
    
    exit->next = new_block;
    new_block->prev = exit;

    exit = new_block;

    return new_block;
}   
        
mtac::basic_block_iterator mtac::Function::insert_before(mtac::basic_block_iterator it, mtac::basic_block_p block){
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

mtac::basic_block_iterator mtac::Function::remove(mtac::basic_block_p block){
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
        if(pred == block->prev){
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

mtac::basic_block_iterator mtac::Function::merge_basic_blocks(basic_block_iterator it, std::shared_ptr<BasicBlock> block){
    auto source = *it; 

    ASSERT(source->next == block || source->prev == block, "Can only merge sibling blocks");

    log::emit<Debug>("CFG") << "Merge " << source->index << " into " << block->index << log::endl;

    if(!source->statements.empty()){
        //B can have some new successors
        for(auto& succ : source->successors){
            if(succ != source->next){
                mtac::make_edge(block, succ);
            }
        }

        //No need to remove the edges, they will be removed by remove call
    }

    //Insert the statements
    if(source->next == block){
        block->statements.insert(block->statements.begin(), source->statements.begin(), source->statements.end());
    } else {
        block->statements.insert(block->statements.end(), source->statements.begin(), source->statements.end());
    }
    
    //Remove the source basic block
    remove(source);

    return at(block);
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

std::size_t mtac::Function::pseudo_registers(){
    return last_pseudo_registers;
}

void mtac::Function::set_pseudo_registers(std::size_t pseudo_registers){
    this->last_pseudo_registers = pseudo_registers;
}
        
std::size_t mtac::Function::pseudo_float_registers(){
    return last_float_pseudo_registers;
}

void mtac::Function::set_pseudo_float_registers(std::size_t pseudo_registers){
    this->last_float_pseudo_registers = pseudo_registers;
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

std::vector<std::shared_ptr<mtac::Loop>>& mtac::Function::loops(){
    return m_loops;
}

mtac::basic_block_iterator mtac::begin(mtac::function_p function){
    return function->begin();
}

mtac::basic_block_iterator mtac::end(mtac::function_p function){
    return function->end();
}
