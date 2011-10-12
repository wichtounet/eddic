//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTEGER_H
#define INTEGER_H

#include <string>

#include "Value.hpp"

namespace eddic {

class Integer : public Value {
    private:
        int m_value;

    public:
        Integer(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, int value) : Value(context, token), m_value(value) {
            m_type = Type::INT;
        };
        
        void write(AssemblyFileWriter& writer);
        bool isConstant();
        int getIntValue();
        
        void assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program);
        void assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program);
};

} //end of eddic

#endif
