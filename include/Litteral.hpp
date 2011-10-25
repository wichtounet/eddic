//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LITTERAL_H
#define LITTERAL_H

#include <string>

#include "Value.hpp"

namespace eddic {

class Litteral : public Value {
    private:
        std::string m_litteral;
        std::string m_label;
	
    public:
        Litteral(std::shared_ptr<Context> context, const Tok token, const std::string& litteral) : Value(context, token), m_litteral(litteral) {
            m_type = Type::STRING;
        };
		
        void checkStrings(StringPool& pool);
        
        bool isConstant();
        bool isImmediate();
        
        std::string getStringValue();
        std::string getStringLabel();
        int getStringSize();

        void assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program);
        void assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program);
        void push(IntermediateProgram& program);
};

} //end of eddic

#endif
