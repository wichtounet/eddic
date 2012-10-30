//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_FUNCTION_H
#define MTAC_FUNCTION_H

#include <memory>
#include <vector>
#include <utility>

#include "iterators.hpp"

#include "mtac/forward.hpp"
#include "mtac/BasicBlock.hpp"
#include "mtac/basic_block_iterator.hpp"

namespace eddic {

class Function;
class FunctionContext;

namespace mtac {

class Loop;

class Function : public std::enable_shared_from_this<Function> {
    public:
        Function(std::shared_ptr<FunctionContext> context, const std::string& name);

        std::shared_ptr<eddic::Function> definition;
        
        std::shared_ptr<FunctionContext> context;

        std::string getName() const;

        void add(Statement statement);
        std::vector<Statement>& getStatements();

        void create_entry_bb();
        void create_exit_bb();

        std::shared_ptr<BasicBlock> current_bb();
        std::shared_ptr<BasicBlock> append_bb();
        std::shared_ptr<BasicBlock> new_bb();
        
        std::shared_ptr<BasicBlock> entry_bb();
        std::shared_ptr<BasicBlock> exit_bb();

        basic_block_iterator begin();
        basic_block_iterator end();
        basic_block_iterator at(std::shared_ptr<BasicBlock> bb);

        basic_block_iterator insert_before(basic_block_iterator it, std::shared_ptr<BasicBlock> block);
        basic_block_iterator merge_basic_blocks(basic_block_iterator it, std::shared_ptr<BasicBlock> block);
        basic_block_iterator remove(basic_block_iterator it);
        basic_block_iterator remove(std::shared_ptr<BasicBlock> bb);

        std::pair<basic_block_iterator, basic_block_iterator> blocks();

        std::vector<std::shared_ptr<Loop>>& loops();

        std::size_t bb_count();
        std::size_t size();
        
        std::size_t pseudo_registers();
        void set_pseudo_registers(std::size_t pseudo_registers);

    private:
        //Before being partitioned, the function has only statement
        std::vector<Statement> statements;
        
        //There is no basic blocks at the beginning
        std::size_t count = 0;
        std::size_t index = 0;
        std::size_t last_pseudo_registers = 0;
        std::shared_ptr<BasicBlock> entry = nullptr;
        std::shared_ptr<BasicBlock> exit = nullptr;

        std::vector<std::shared_ptr<mtac::Loop>> m_loops;

        std::string name;
};

typedef std::shared_ptr<mtac::Function> function_p;

basic_block_iterator begin(std::shared_ptr<mtac::Function> function);
basic_block_iterator end(std::shared_ptr<mtac::Function> function);

} //end of mtac

template<>
struct Iterators<std::shared_ptr<mtac::Function>> {
    std::shared_ptr<mtac::Function> container;

    mtac::basic_block_iterator it;
    mtac::basic_block_iterator end;

    Iterators(std::shared_ptr<mtac::Function> container) : container(container), it(container->begin()), end(container->end()) {}

    std::shared_ptr<mtac::BasicBlock>& operator*(){
        return *it;
    }

    void operator++(){
        ++it;
    }
    
    void operator--(){
        --it;
    }

    void insert(std::shared_ptr<mtac::BasicBlock> bb){
        it = container->insert_before(it, bb);
        end = container->end();
    }

    void erase(){
        it = container->remove(it);
        end = container->end();
    }

    /*!
     * \brief Merge the current block into the specified one. 
     * The current block will be removed.
     * \return an iterator to the merged block 
     */
    void merge_to(std::shared_ptr<mtac::BasicBlock> bb){
        it = container->merge_basic_blocks(it, bb);
        end = container->end();
    }
    
    /*!
     * \brief Merge the specified block into the current one. 
     * The specified block will be removed.
     * \return an iterator to the merged block 
     */
    void merge_in(std::shared_ptr<mtac::BasicBlock> bb){
        it = container->merge_basic_blocks(container->at(bb), *it);
        end = container->end();
    }

    bool has_next(){
        return it != end;
    }
};

} //end of eddic

#endif
