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

template<typename Reg>
class Registers {
    typedef typename std::vector<Reg>::const_iterator Iterator;
    typedef typename std::vector<Reg>::const_reverse_iterator ReverseIterator;

    public:
        Registers(std::vector<Reg> registers, std::shared_ptr<Variable> var);

        void reset();

        bool inRegister(std::shared_ptr<Variable> variable);
        bool inRegister(std::shared_ptr<Variable> variable, Reg reg);
        bool used(Reg reg);

        void reserve(Reg reg);
        void release(Reg reg);
        bool reserved(Reg reg);

        Reg operator[](const std::shared_ptr<Variable> variable);
        std::shared_ptr<Variable> operator[](Reg reg);

        Reg first() const ;

        void setLocation(std::shared_ptr<Variable> variable, Reg a);
        void remove(std::shared_ptr<Variable> variable);

        Iterator begin() const;
        Iterator end() const;
        
        ReverseIterator rbegin() const;
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
    return inRegister(variable) && variables[variable] == reg;
}

template<typename Reg>
Reg Registers<Reg>::first() const {
    return registers[0];
}

template<typename Reg>
bool Registers<Reg>::used(Reg reg) {
    return descriptors[(int) reg].get() != 0;
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

    return descriptors[(int) reg];
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
        descriptors[(int) (*this)[variable]] = nullptr;
    }

    variables[variable] = reg;
    descriptors[(int) reg] = variable;
    
    assert((*this)[(*this)[variable]] == variable);
}

template<typename Reg>
void Registers<Reg>::remove(std::shared_ptr<Variable> variable){
    assert(inRegister(variable));
    assert((*this)[(*this)[variable]] == variable);

    descriptors[(int) (*this)[variable]] = nullptr;
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

    descriptors[(int) reg] = retainVariable;
}

template<typename Reg>
void Registers<Reg>::release(Reg reg){
   assert(descriptors[(int) reg] == retainVariable);
   
   descriptors[(int) reg] = nullptr; 
}

template<typename Reg>
bool Registers<Reg>::reserved(Reg reg){
   return descriptors[(int) reg] == retainVariable; 
}

} //end of as

} //end of eddic

#endif
