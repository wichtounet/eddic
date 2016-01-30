//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_FUNCTION_H
#define MTAC_FUNCTION_H

#include <memory>
#include <vector>
#include <utility>
#include <set>
#include <ostream>

#include "iterators.hpp"

#include "mtac/forward.hpp"
#include "mtac/basic_block.hpp"
#include "mtac/basic_block_iterator.hpp"
#include "mtac/Quadruple.hpp"

#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"

namespace eddic {

class Function;
class FunctionContext;

namespace mtac {

class loop;

/*!
 * \class Function
 * \brief A function of an EDDI program represented in intermediate representation. 
 *
 * A Function can hold either MTAC of LTAC statements. Before basic block extraction, 
 * the Function can directly contains statements. After that, only the basic blocks have
 * statements.
 *
 * The basic_block of the Function are stored in a doubly-linked list. The entry and exit basic
 * blocks can be obtained with the entry_bb() and exit_bb() functions. The basic blocks can be
 * iterated using begin() and end() iterators. 
 *
 * \see mtac::basic_block
 */
class Function : public std::enable_shared_from_this<Function> {
    public:
        Function(std::shared_ptr<FunctionContext> context, std::string name, eddic::Function& definition);

        //Function cannot be copied
        Function(const Function& rhs) = delete;
        Function& operator=(const Function& rhs) = delete;

        //Function can be moved
        Function(Function&& rhs);
        Function& operator=(Function&& rhs);

        ~Function();

        std::string get_name() const;

        template< class... Args >
        inline void emplace_back( Args&&... args ){
            statements.emplace_back(std::forward<Args>(args)...);
        }
        
        inline void push_back(mtac::Quadruple&& quadruple){
            statements.push_back(std::forward<mtac::Quadruple>(quadruple));
        }

        mtac::Quadruple& find(std::size_t uid);
        ltac::Instruction& find_low(std::size_t uid);

        std::vector<mtac::Quadruple>& get_statements();
        const std::vector<mtac::Quadruple>& get_statements() const;

        void release_statements();

        void create_entry_bb();
        void create_exit_bb();

        basic_block_p current_bb();
        basic_block_p append_bb();
        basic_block_p new_bb();
        
        basic_block_p entry_bb();
        basic_block_p exit_bb();

        /*!
         * \brief Return an iterator to the beginning of the doubly-linked list of basic blocks. 
         * \return iterator to the beginning of the doubly-linked list of basic blocks. 
         */
        basic_block_iterator begin(){
            return basic_block_iterator(entry, nullptr);
        }

        /*!
         * \brief Return an iterator to the end of the doubly-linked list of basic blocks. 
         * \return iterator to the end of the doubly-linked list of basic blocks. 
         */
        basic_block_iterator end(){
            return basic_block_iterator(nullptr, exit);    
        }

        /*!
         * \brief Return an iterator to the beginning of the doubly-linked list of basic blocks. 
         * \return iterator to the beginning of the doubly-linked list of basic blocks. 
         */
        basic_block_const_iterator begin() const {
            return basic_block_const_iterator(entry, nullptr);
        }

        /*!
         * \brief Return an iterator to the end of the doubly-linked list of basic blocks. 
         * \return iterator to the end of the doubly-linked list of basic blocks. 
         */
        basic_block_const_iterator end() const {
            return basic_block_const_iterator(nullptr, exit);
        }

        /*!
         * \brief Return the position of the basic block inside the instruction stream of the function. 
         *
         * The basic block must be part of the function. This runs in O(n).
         *
         * \param bb The basic block to search
         * \return the position of the basic block inside the instruction stream of the function.
         */
        std::size_t position(const basic_block_p& bb) const;
        
        basic_block_iterator at(basic_block_p bb);

        basic_block_iterator insert_before(basic_block_iterator it, basic_block_p block);
        basic_block_iterator insert_after(basic_block_iterator it, basic_block_p block);
        basic_block_iterator merge_basic_blocks(basic_block_iterator it, basic_block_p block);
        basic_block_iterator remove(basic_block_iterator it);
        basic_block_iterator remove(basic_block_p bb);

        std::pair<basic_block_iterator, basic_block_iterator> blocks();

        std::vector<mtac::loop>& loops();

        std::size_t bb_count() const;
        std::size_t size() const;
        std::size_t size_no_nop() const;
        
        std::size_t pseudo_registers() const ;
        void set_pseudo_registers(std::size_t pseudo_registers);
        
        std::size_t pseudo_float_registers() const;
        void set_pseudo_float_registers(std::size_t pseudo_registers);
        
        const std::set<ltac::Register>& use_registers() const;
        const std::set<ltac::FloatRegister>& use_float_registers() const;
        
        const std::set<ltac::Register>& variable_registers() const;
        const std::set<ltac::FloatRegister>& variable_float_registers() const;
        
        void use(ltac::Register reg);
        void use(ltac::FloatRegister reg);
        
        void variable_use(ltac::Register reg);
        void variable_use(ltac::FloatRegister reg);

        /*!
         * \brief Indicate if this function is the main function.
         * \return true if it is the main function, false otherwise. 
         */
        bool is_main() const;

        /*!
         * \brief Indicate if the function is pure. 
         *
         * A pure function cannot modify value of global variables, edit pointers or call unpure functions. 
         * \return true if the function is pure, false otherise. 
         */
        bool& pure();

        /*!
         * \brief Indicate if the function is pure. 
         *
         * A pure function cannot modify value of global variables, edit pointers or call unpure functions. 
         * \return true if the function is pure, false otherise. 
         */
        bool pure() const;

        /*!
         * \brief Indicate if the function comes from the standard library or is a user function. 
         * \return true if the function comes from the standard library, false otherwise. 
         */
        bool& standard();

        /*!
         * \brief Indicate if the function comes from the standard library or is a user function. 
         * \return true if the function comes from the standard library, false otherwise. 
         */
        bool standard() const;

        /*!
         * \brief Return the function definition for this MTAC function. 
         * \return the function definition of this function.
         */
        eddic::Function& definition();

        /*!
         * \brief Return the function definition for this MTAC function. 
         * \return the function definition of this function.
         */
        const eddic::Function& definition() const;

        std::shared_ptr<FunctionContext> context;

    private:
        eddic::Function* _definition;

        //Before being partitioned, the function has only statement
        std::vector<mtac::Quadruple> statements;

        bool _pure = false;
        bool _standard = false;
        
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

        std::vector<mtac::loop> m_loops;

        std::string name;
};

bool operator==(const mtac::Function& lhs, const mtac::Function& rhs);

std::ostream& operator<<(std::ostream& stream, const mtac::Function& function);

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
     */
    void merge_to(mtac::basic_block_p bb){
        it = container.merge_basic_blocks(it, bb);
        end = container.end();
    }
    
    /*!
     * \brief Merge the specified block into the current one. 
     * The specified block will be removed.
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
