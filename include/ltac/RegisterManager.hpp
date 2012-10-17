//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_REGISTER_MANAGER_H
#define LTAC_REGISTER_MANAGER_H

#include <memory>
#include <vector>
#include <unordered_set>

#include "variant.hpp"
#include "Platform.hpp"
#include "FloatPool.hpp"
#include "Options.hpp"

#include "mtac/Program.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/EscapeAnalysis.hpp"

#include "ltac/Program.hpp"
#include "ltac/AbstractRegisterManager.hpp"

namespace eddic {

namespace ltac {

class StatementCompiler;

class RegisterManager : public AbstractRegisterManager {
    public:
        //Keep track of the written variables to spills them
        std::unordered_set<std::shared_ptr<Variable>> written;

        std::shared_ptr<Configuration> configuration;

        //Liveness information
        std::shared_ptr<mtac::DataFlowResults<mtac::LiveVariableAnalysisProblem::ProblemDomain>> liveness;
        mtac::EscapedVariables pointer_escaped;

        //The function being compiled
        std::shared_ptr<ltac::Function> function;

        std::shared_ptr<FloatPool> float_pool;
        std::weak_ptr<StatementCompiler> compiler;

        RegisterManager(const std::vector<ltac::Register>& registers, const std::vector<ltac::FloatRegister>& float_registers, 
                std::shared_ptr<ltac::Function> function, std::shared_ptr<FloatPool> float_pool);

        /*!
         * Deleted copy constructor
         */
        RegisterManager(const RegisterManager& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        RegisterManager& operator=(const RegisterManager& rhs) = delete;

        void reset();

        ltac::Register get_reg(std::shared_ptr<Variable> var);
        ltac::Register get_reg_no_move(std::shared_ptr<Variable> var);
        ltac::FloatRegister get_float_reg(std::shared_ptr<Variable> var);
        ltac::FloatRegister get_float_reg_no_move(std::shared_ptr<Variable> var);

        bool in_reg(std::shared_ptr<Variable> var);

        void copy(mtac::Argument argument, ltac::FloatRegister reg);
        void copy(mtac::Argument argument, ltac::Register reg);

        void move(mtac::Argument argument, ltac::Register reg);
        void move(mtac::Argument argument, ltac::FloatRegister reg);

        void safe_move(std::shared_ptr<Variable> variable, ltac::Register reg);
        void safe_move(std::shared_ptr<Variable> variable, ltac::FloatRegister reg);

        ltac::Register get_free_reg();
        ltac::FloatRegister get_free_float_reg();

        void spills_all();

        void spills(ltac::Register reg);
        void spills(ltac::FloatRegister reg);

        void spills_if_necessary(ltac::Register reg, mtac::Argument arg);

        bool is_written(std::shared_ptr<Variable> variable);
        void set_written(std::shared_ptr<Variable> variable);

        void set_current(mtac::Statement statement);

        bool is_live(std::shared_ptr<Variable> variable, mtac::Statement statement);
        bool is_live(std::shared_ptr<Variable> variable);
        bool is_escaped(std::shared_ptr<Variable> variable);

        void collect_parameters(std::shared_ptr<eddic::Function> definition, const PlatformDescriptor* descriptor);

        void save_registers(std::shared_ptr<mtac::Param> param, const PlatformDescriptor* descriptor);
        void restore_pushed_registers();

        std::shared_ptr<StatementCompiler> access_compiler();
    
    private: 
        mtac::Statement current;
        
        //Store the Register that are saved before call
        std::vector<ltac::Register> int_pushed;
        std::vector<ltac::FloatRegister> float_pushed;

        //Allow to push needed register before the first push param
        bool first_param = true;
};

} //end of ltac

} //end of eddic

#endif
