//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_STATEMENT_COMPILER_H
#define LTAC_STATEMENT_COMPILER_H

#include <memory>
#include <boost/variant.hpp>

#include "FloatPool.hpp"

#include "mtac/Program.hpp"

#include "ltac/Program.hpp"
#include "ltac/RegisterManager.hpp"

namespace eddic {

namespace ltac {

class StatementCompiler : public boost::static_visitor<> {
    public:
        StatementCompiler(std::vector<ltac::Register> registers, std::vector<ltac::FloatRegister> float_registers, 
                std::shared_ptr<ltac::Function> function, std::shared_ptr<FloatPool> float_pool);
    
        /*!
         * Deleted copy constructor
         */
        StatementCompiler(const StatementCompiler& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        StatementCompiler& operator=(const StatementCompiler& rhs) = delete;

        void set_current(mtac::Statement statement);
        void reset();
        void end_basic_block();
        void collect_parameters(std::shared_ptr<eddic::Function> definition);

        void operator()(std::shared_ptr<mtac::IfFalse>& if_false);
        void operator()(std::shared_ptr<mtac::If>& if_);
        void operator()(std::shared_ptr<mtac::Goto>& goto_);
        void operator()(std::shared_ptr<mtac::Param>& param);
        void operator()(std::shared_ptr<mtac::Call>& call);
        void operator()(std::shared_ptr<mtac::Quadruple>& quadruple);
        void operator()(std::shared_ptr<mtac::NoOp>&);
        void operator()(std::string& str);

        bool ended = false;     //Is the basic block ended ?
   
    private:
        //The function being compiled
        std::shared_ptr<ltac::Function> function;

        ltac::RegisterManager manager;

        std::shared_ptr<FloatPool> float_pool;
        
        PlatformDescriptor* descriptor;
        
        void pass_in_int_register(mtac::Argument& argument, int position);
        void pass_in_float_register(mtac::Argument& argument, int position);

        void compare_binary(mtac::Argument& arg1, mtac::Argument& arg2);
        void compare_float_binary(mtac::Argument& arg1, mtac::Argument& arg2);
        void compare_unary(mtac::Argument arg1);

        void mul(std::shared_ptr<Variable> result, mtac::Argument& arg2);
        void div_eax(std::shared_ptr<mtac::Quadruple> quadruple);
        void div(std::shared_ptr<mtac::Quadruple> quadruple);
        void mod(std::shared_ptr<mtac::Quadruple> quadruple);
        void set_if_cc(ltac::Operator set, std::shared_ptr<mtac::Quadruple>& quadruple);
        
        ltac::Register to_register(std::shared_ptr<Variable> var);
        ltac::Argument to_arg(mtac::Argument argument);
        
        ltac::Address to_address(std::shared_ptr<Variable> var, int offset);
        ltac::Address to_address(std::shared_ptr<Variable> var, mtac::Argument offset);
        
        ltac::Address to_pointer(std::shared_ptr<Variable> var, int offset);
};

} //end of ltac

} //end of eddic

#endif
