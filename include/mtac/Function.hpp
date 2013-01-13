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
#include <set>

#include "iterators.hpp"

#include "mtac/forward.hpp"
#include "mtac/basic_block.hpp"
#include "mtac/basic_block_iterator.hpp"
#include "mtac/Loop.hpp"
#include "mtac/Quadruple.hpp"

#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"

namespace eddic {

class Function;
class FunctionContext;

namespace mtac {

class Function : public std::enable_shared_from_this<Function> {
    public:
        Function(std::shared_ptr<FunctionContext> context, const std::string& name, eddic::Function& definition);

        //Function cannot be copied
        Function(const Function& rhs) = delete;
        Function& operator=(const Function& rhs) = delete;

        //Function can be moved
        Function(Function&& rhs);
        Function& operator=(Function&& rhs);

        std::string get_name() const;

        void add(mtac::Quadruple statement);
        
        template< class... Args >
        void emplace_back( Args&&... args ){
            statements.emplace_back(std::forward<Args>(args)...);
        }
        
        void push_back(const mtac::Quadruple& quadruple){
            statements.push_back(quadruple);
        }

        void push_back(mtac::Quadruple&& quadruple){
            statements.push_back(quadruple);
        }

        mtac::Quadruple& find(std::size_t uid);

        std::vector<mtac::Quadruple>& get_statements();
        void release_statements();

        void create_entry_bb();
        void create_exit_bb();

        basic_block_p current_bb();
        basic_block_p append_bb();
        basic_block_p new_bb();
        
        basic_block_p entry_bb();
        basic_block_p exit_bb();

        basic_block_iterator begin();
        basic_block_iterator end();
        
        basic_block_const_iterator begin() const ;
        basic_block_const_iterator end() const ;
        
        basic_block_iterator at(basic_block_p bb);

        basic_block_iterator insert_before(basic_block_iterator it, basic_block_p block);
        basic_block_iterator insert_after(basic_block_iterator it, basic_block_p block);
        basic_block_iterator merge_basic_blocks(basic_block_iterator it, basic_block_p block);
        basic_block_iterator remove(basic_block_iterator it);
        basic_block_iterator remove(basic_block_p bb);

        std::pair<basic_block_iterator, basic_block_iterator> blocks();

        std::vector<mtac::Loop>& loops();

        std::size_t bb_count() const;
        std::size_t size() const;
        
        std::size_t pseudo_registers();
        void set_pseudo_registers(std::size_t pseudo_registers);
        
        std::size_t pseudo_float_registers();
        void set_pseudo_float_registers(std::size_t pseudo_registers);
        
        const std::set<ltac::Register>& use_registers() const;
        const std::set<ltac::FloatRegister>& use_float_registers() const;
        
        const std::set<ltac::Register>& variable_registers() const;
        const std::set<ltac::FloatRegister>& variable_float_registers() const;
        
        void use(ltac::Register reg);
        void use(ltac::FloatRegister reg);
        
        void variable_use(ltac::Register reg);
        void variable_use(ltac::FloatRegister reg);

        bool is_main() const;

        bool& pure();

        eddic::Function& definition();

        std::shared_ptr<FunctionContext> context;

    private:
        eddic::Function* _definition;

        //Before being partitioned, the function has only statement
        std::vector<mtac::Quadruple> statements;

        bool _pure = false;
        
        //There is no basic blocks at the beginning
        std::size_t count = 0;
        std::size_t index = 0;
        basic_block_p entry = nullptr;
        basic_block_p exit = nullptr;

        std::set<ltac::Register> _use_registers;
        std::set<ltac::FloatRegister> _use_float_registers;
        
        std::set<ltac::Register> _variable_registers;
        std::set<ltac::FloatRegister> _variable_float_registers;
        
        std::size_t last_pseudo_registers = 0;
        std::size_t last_float_pseudo_registers = 0;

        std::vector<mtac::Loop> m_loops;

        std::string name;
};

bool operator==(const mtac::Function& lhs, const mtac::Function& rhs);

} //end of mtac

template<>
struct Iterators<mtac::Function> {
    mtac::Function& container;

    mtac::basic_block_iterator it;
    mtac::basic_block_iterator end;

    Iterators(mtac::Function& container) : container(container), it(container.begin()), end(container.end()) {}

    mtac::basic_block_p& operator*(){
        return *it;
    }

    void operator++(){
        ++it;
    }
    
    void operator--(){
        --it;
    }

    void insert(mtac::basic_block_p bb){
        it = container.insert_before(it, bb);
        end = container.end();
    }

    void erase(){
        it = container.remove(it);
        end = container.end();
    }

    /*!
     * \brief Merge the current block into the specified one. 
     * The current block will be removed.
     * \return an iterator to the merged block 
     */
    void merge_to(mtac::basic_block_p bb){
        it = container.merge_basic_blocks(it, bb);
        end = container.end();
    }
    
    /*!
     * \brief Merge the specified block into the current one. 
     * The specified block will be removed.
     * \return an iterator to the merged block 
     */
    void merge_in(mtac::basic_block_p bb){
        it = container.merge_basic_blocks(container.at(bb), *it);
        end = container.end();
    }

    bool has_next(){
        return it != end;
    }
};

} //end of eddic

#endif
