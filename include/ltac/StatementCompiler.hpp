//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_STATEMENT_COMPILER_H
#define LTAC_STATEMENT_COMPILER_H

#include <memory>
#include <vector>

#include "variant.hpp"
#include "FloatPool.hpp"
#include "Options.hpp"

#include "mtac/Program.hpp"

#include "ltac/RegisterManager.hpp"

namespace eddic {

namespace ltac {

class StatementCompiler : public boost::static_visitor<> {
    public:
        const PlatformDescriptor* descriptor;
        Platform platform;
        std::shared_ptr<Configuration> configuration;

        StatementCompiler(const std::vector<ltac::Register>& registers, const std::vector<ltac::FloatRegister>& float_registers, 
                mtac::function_p function, std::shared_ptr<FloatPool> float_pool);
    
        /*!
         * Deleted copy constructor
         */
        StatementCompiler(const StatementCompiler& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        StatementCompiler& operator=(const StatementCompiler& rhs) = delete;

        void set_current(mtac::Statement statement);
        void collect_parameters(std::shared_ptr<eddic::Function> definition);
        
        void end_bb();

        void operator()(std::shared_ptr<mtac::IfFalse> if_false);
        void operator()(std::shared_ptr<mtac::If> if_);
        void operator()(std::shared_ptr<mtac::Goto> goto_);
        void operator()(std::shared_ptr<mtac::Param> param);
        void operator()(std::shared_ptr<mtac::Call> call);
        void operator()(std::shared_ptr<mtac::Quadruple> quadruple);
        void operator()(std::shared_ptr<mtac::NoOp>);
        void operator()(std::string& str);

        void push(ltac::Argument arg);
        void pop(ltac::Argument arg);

        ltac::RegisterManager manager;

        mtac::basic_block_p bb;
        std::shared_ptr<mtac::Program> program;
   
    private:
        //The function being compiled
        mtac::function_p function;

        std::shared_ptr<FloatPool> float_pool;

        bool first_param = true;

        //Uses for the next call
        std::vector<ltac::PseudoRegister> uses;
        std::vector<ltac::PseudoFloatRegister> float_uses;
        
        void pass_in_int_register(mtac::Argument& argument, int position);
        void pass_in_float_register(mtac::Argument& argument, int position);

        void compare_binary(mtac::Argument& arg1, mtac::Argument& arg2);
        void compare_float_binary(mtac::Argument& arg1, mtac::Argument& arg2);
        void compare_unary(mtac::Argument arg1);

        void set_if_cc(ltac::Operator set, std::shared_ptr<mtac::Quadruple> quadruple);
        
        ltac::PseudoRegister to_register(std::shared_ptr<Variable> var);
        
        ltac::PseudoRegister get_address_in_pseudo_reg(std::shared_ptr<Variable> var, int offset);
        ltac::PseudoRegister get_address_in_pseudo_reg2(std::shared_ptr<Variable> var, ltac::PseudoRegister offset);

        ltac::Argument to_arg(mtac::Argument argument);
        
        ltac::Address address(std::shared_ptr<Variable> var, mtac::Argument offset);
    
        void compile_ASSIGN(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_PASSIGN(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FASSIGN(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_ADD(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_SUB(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_MUL(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_DIV(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_MOD(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FADD(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FSUB(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FMUL(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FDIV(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_EQUALS(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_NOT_EQUALS(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_GREATER(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_GREATER_EQUALS(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_LESS(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_LESS_EQUALS(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FE(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FNE(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FG(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FGE(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FLE(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FL(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_MINUS(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FMINUS(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_I2F(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_F2I(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_DOT(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_FDOT(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_PDOT(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_DOT_ASSIGN(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_DOT_FASSIGN(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_DOT_PASSIGN(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_RETURN(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_NOT(std::shared_ptr<mtac::Quadruple> quadruple);
        void compile_AND(std::shared_ptr<mtac::Quadruple> quadruple);
};

} //end of ltac

} //end of eddic

#endif
