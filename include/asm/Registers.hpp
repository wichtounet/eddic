//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ASM_REGISTERS_H
#define ASM_REGISTERS_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>

namespace eddic {

class Variable;

namespace as {

/*!
 * \class Registers
 * \brief Manager for registers. 
 * This class handles the management of the registers for a low-level compiler. 
 * This class holds the free registers, the locations of the variables and reservations of registers. 
 * \param Reg The type of register to handle. 
 */
template<typename Reg>
class Registers {
    typedef typename std::vector<Reg>::const_iterator Iterator;
    typedef typename std::vector<Reg>::const_reverse_iterator ReverseIterator;

    public:
        Registers(std::vector<Reg> registers, std::shared_ptr<Variable> var);

        /*!
         * Deleted copy constructor
         */
        Registers(const Registers<Reg>& rhs) = delete;
        
        /*!
         * Deleted copy assignment operator. 
         */
        Registers& operator=(const Registers<Reg>& rhs) = delete;
    
        /*!
         * Reset the content of the registers manager. Only the list of available registers is kept. 
         */
        void reset();

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
         * Indicates if the given register is used. 
         * \param reg The register to test. 
         * \return true if the register is used, otherwise false. 
         */
        bool used(Reg reg);

        /*!
         * Reserve the given register. A reserved register cannot be used to hold a variable. 
         * \param reg The register to reserve. 
         */
        void reserve(Reg reg);

        /*!
         * Release the given reserved register. 
         * \param reg The register to release. 
         */
        void release(Reg reg);
        
        /*!
         * Indicates if the given register is reserved. 
         * \param reg The register to test. 
         * \return true if the register is reserved, otherwise false. 
         */
        bool reserved(Reg reg);

        /*!
         * Return the register in which the variable is hold. 
         * \param The variable to test. 
         * \return The register in which the variable is hold.
         */
        Reg operator[](const std::shared_ptr<Variable> variable);

        /*!
         * Return the variable hold in the given register. 
         * \param The register to test. 
         * \return The variable hold in the given register.
         */
        std::shared_ptr<Variable> operator[](Reg reg);

        /*!
         * Return the first register. 
         * \return The first register.
         */
        Reg first() const ;

        /*!
         * Set the location of the given variable to the given register. 
         * \param variable The variable to set the location for. 
         * \param reg The register that holds the variable. 
         */
        void setLocation(std::shared_ptr<Variable> variable, Reg reg);
        
        /*!
         * Remove the variable from the variable that holds it. 
         * \param variable The variable to remove from the register. 
         */
        void remove(std::shared_ptr<Variable> variable);

        /*!
         * Returns an iterator to the first register. 
         * \return An iterator pointing to the first register. 
         */
        Iterator begin() const;
        
        /*!
         * Returns an iterator to the last register. 
         * \return An iterator pointing past the last register. 
         */
        Iterator end() const;
        
        /*!
         * Returns an iterator to the last register. 
         * \return An iterator pointing to the last register. 
         */
        ReverseIterator rbegin() const;
        
        /*!
         * Returns an iterator to the first register. 
         * \return An iterator pointing past the first register. 
         */
        ReverseIterator rend() const;

    private:
        std::vector<Reg> registers;
        std::vector<std::shared_ptr<Variable>> descriptors;
        std::unordered_map<std::shared_ptr<Variable>, Reg> variables;

        std::shared_ptr<Variable> retainVariable;
};

template<typename Reg>
Registers<Reg>::Registers(std::vector<Reg> reg, std::shared_ptr<Variable> var) : registers(reg), retainVariable(var) {
    //Resize the descriptors the same way as the registers
    descriptors.resize(reg.size());
}

template<typename Reg>
bool Registers<Reg>::inRegister(std::shared_ptr<Variable> variable) {
    return variables.find(variable) != variables.end();
}

template<typename Reg>
bool Registers<Reg>::inRegister(std::shared_ptr<Variable> variable, Reg reg) {
    return inRegister(variable) && variables.at(variable) == reg;
}

template<typename Reg>
Reg Registers<Reg>::first() const {
    return registers.at(0);
}

template<typename Reg>
bool Registers<Reg>::used(Reg reg) {
    return descriptors.at(static_cast<int>(reg)).get() != 0;
}

template<typename Reg>
Reg Registers<Reg>::operator[](const std::shared_ptr<Variable> variable){
    //This method should never be called without the variable into the map
    assert(inRegister(variable));

    return variables[variable];
}

template<typename Reg>
std::shared_ptr<Variable> Registers<Reg>::operator[](Reg reg){
    //This method should never be called when the register is not used
    assert(used(reg));
    assert(!reserved(reg));

    return descriptors.at(static_cast<int>(reg));
}

template<typename Reg>
void Registers<Reg>::reset(){
    variables.clear();

    for(unsigned int i = 0; i < descriptors.size(); ++i){
        descriptors[i] = nullptr;
    }
}

template<typename Reg>
void Registers<Reg>::setLocation(std::shared_ptr<Variable> variable, Reg reg){
    //Make sure that there is no other refernce to this variable
    if(inRegister(variable)){
        descriptors.at(static_cast<int>((*this)[variable])) = nullptr;
    }

    variables[variable] = reg;
    descriptors.at(static_cast<int>(reg)) = variable;
    
    assert((*this)[(*this)[variable]] == variable);
}

template<typename Reg>
void Registers<Reg>::remove(std::shared_ptr<Variable> variable){
    assert(inRegister(variable));
    assert((*this)[(*this)[variable]] == variable);

    descriptors.at(static_cast<int>((*this)[variable])) = nullptr;

    variables.erase(variable);
}
        
template<typename Reg>
typename Registers<Reg>::Iterator Registers<Reg>::begin() const {
    return registers.cbegin();
}

template<typename Reg>
typename Registers<Reg>::Iterator Registers<Reg>::end() const {
    return registers.cend();
}

template<typename Reg>
typename Registers<Reg>::ReverseIterator Registers<Reg>::rbegin() const {
    return registers.crbegin();
}

template<typename Reg>
typename Registers<Reg>::ReverseIterator Registers<Reg>::rend() const {
    return registers.crend();
}

template<typename Reg>
void Registers<Reg>::reserve(Reg reg){
    assert(!used(reg));

    descriptors.at(static_cast<int>(reg)) = retainVariable;
}

template<typename Reg>
void Registers<Reg>::release(Reg reg){
   assert(descriptors[static_cast<int>(reg)] == retainVariable);
   
   descriptors.at(static_cast<int>(reg)) = nullptr;
}

template<typename Reg>
bool Registers<Reg>::reserved(Reg reg){
   return descriptors.at(static_cast<int>(reg)) == retainVariable; 
}

} //end of as

} //end of eddic

#endif
