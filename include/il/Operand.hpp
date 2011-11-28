//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPERAND_H
#define OPERAND_H

#include <string>
#include <memory>

namespace eddic {

enum class OperandType : unsigned int {
    IMMEDIATE,
    REGISTER,
    STACK,
    GLOBAL
};

/*!
 * \class Operand
 * \brief Represent an assembly operand
 */
class Operand {
    private:
        OperandType m_type;

    public:
        Operand(OperandType type);

        virtual std::string getValue() const = 0;

        /*!
         * \brief Return a new operand the value of what is pointed by the operand. Example: %eax becomes (%eax). 
         * \param offset Address offset of the operand
         * \return A new Operand representing what is pointed by the operand plus the offset
         */
        std::shared_ptr<Operand> valueOf(int offset = 0) const;

        bool isImmediate() const ;
        bool isRegister() const ;
        bool isStack() const ;
        bool isGlobal() const;
};

} //end of eddic

#endif
