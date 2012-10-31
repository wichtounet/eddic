//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ASM_PSEUDO_REGISTERS_H
#define ASM_PSEUDO_REGISTERS_H

#include <memory>
#include <unordered_map>

namespace eddic {

class Variable;

namespace as {

template<typename Reg>
struct PseudoRegisters {
    public:
        PseudoRegisters();

        /*!
         * Deleted copy constructor
         */
        PseudoRegisters(const PseudoRegisters<Reg>& rhs) = delete;
        
        /*!
         * Deleted copy assignment operator. 
         */
        PseudoRegisters& operator=(const PseudoRegisters<Reg>& rhs) = delete;

        /*!
         * Indicates if the given variable is in a register. 
         * \param variable The variable to test. 
         * \return true if the variable is hold in a register, otherwise false. 
         */
        bool inRegister(std::shared_ptr<Variable> variable);
        
        /*!
         * Indicates if the given variable is in the given register. 
         * \param variable The variable to test. 
         * \param reg The register to test. 
         * \return true if the variable is hold in the given register, otherwise false. 
         */
        bool inRegister(std::shared_ptr<Variable> variable, Reg reg);

        /*!
         * Return the register in which the variable is hold. 
         * \param The variable to test. 
         * \return The register in which the variable is hold.
         */
        Reg operator[](const std::shared_ptr<Variable> variable);

        /*!
         * Set the location of the given variable to the given register. 
         * \param variable The variable to set the location for. 
         * \param reg The register that holds the variable. 
         */
        void setLocation(std::shared_ptr<Variable> variable, Reg reg);

        Reg get_new_reg();
        int last_reg();
        
        Reg get_bound_reg(unsigned short);

    private:
        int current_reg = 0;
        std::unordered_map<std::shared_ptr<Variable>, Reg> variables;
};

template<typename Reg>
PseudoRegisters<Reg>::PseudoRegisters() {
    //Nothing to do
}
        
template<typename Reg>
int PseudoRegisters<Reg>::last_reg() {
    return current_reg;
}

template<typename Reg>
bool PseudoRegisters<Reg>::inRegister(std::shared_ptr<Variable> variable) {
    return variables.find(variable) != variables.end();
}

template<typename Reg>
bool PseudoRegisters<Reg>::inRegister(std::shared_ptr<Variable> variable, Reg reg) {
    return inRegister(variable) && variables.at(variable) == reg;
}

template<typename Reg>
Reg PseudoRegisters<Reg>::operator[](const std::shared_ptr<Variable> variable){
    //This method should never be called without the variable into the map
    assert(inRegister(variable));

    return variables[variable];
}

template<typename Reg>
Reg PseudoRegisters<Reg>::get_bound_reg(unsigned short hard){
    return Reg(current_reg++, hard);
}

template<typename Reg>
Reg PseudoRegisters<Reg>::get_new_reg(){
    return Reg(current_reg++);
}

template<typename Reg>
void PseudoRegisters<Reg>::setLocation(std::shared_ptr<Variable> variable, Reg reg){
    variables[variable] = reg;
    
    assert(inRegister(variable, reg));
}

} //end of as

} //end of eddic

#endif
