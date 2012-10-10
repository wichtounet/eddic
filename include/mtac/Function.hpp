//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_FUNCTION_H
#define MTAC_FUNCTION_H

#include "iterators.hpp"

#include "mtac/BasicBlock.hpp"
#include "mtac/Statement.hpp"
#include "mtac/basic_block_iterator.hpp"

namespace eddic {

class FunctionContext;

namespace mtac {

class Function {
    public:
        typedef std::shared_ptr<BasicBlock> BlockPtr;
        typedef std::list<BlockPtr> BlockList;
        typedef BlockList::iterator BlockIterator;

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
        basic_block_iterator remove(basic_block_iterator it);
        basic_block_iterator remove(std::shared_ptr<BasicBlock> bb);

        std::pair<basic_block_iterator, basic_block_iterator> blocks();

        std::size_t bb_count();
        std::size_t size();

    private:
        //Before being partitioned, the function has only statement
        std::vector<Statement> statements;
        
        //There is no basic blocks at the beginning
        std::size_t count = 0;
        std::shared_ptr<BasicBlock> entry;
        std::shared_ptr<BasicBlock> exit;

        std::string name;
};

basic_block_iterator begin(std::shared_ptr<mtac::Function> function);
basic_block_iterator end(std::shared_ptr<mtac::Function> function);

} //end of mtac

template<>
struct Iterators<std::shared_ptr<mtac::Function>> {
    std::shared_ptr<mtac::Function> container;

    mtac::basic_block_iterator it;
    mtac::basic_block_iterator end;

    Iterators(std::shared_ptr<mtac::Function> container) : container(container), it(container->begin()), end(container->end()) {}

    auto operator*() -> decltype(*it) {
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
    }

    void erase(){
        it = container->remove(it);
    }

    bool has_next(){
        return it != end;
    }
};

} //end of eddic

#endif
