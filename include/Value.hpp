//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VALUE_H
#define VALUE_H

#include <string>

#include "ParseNode.hpp"
#include "Types.hpp"

namespace eddic {

class Variable;
class IntermediateProgram;
class Operand;

class Value : public ParseNode {
    protected:
        Type m_type;

    public:
        Value(std::shared_ptr<Context> context, const Tok& token);

        Type type() const ;

        virtual bool isConstant();
        virtual bool isImmediate();
        
        virtual std::string getStringValue();
        virtual std::string getStringLabel();
        virtual int getStringSize();
        
        virtual int getIntValue();

        virtual void assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program) = 0;
        virtual void assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program) = 0;
        virtual void push(IntermediateProgram& program) = 0;
};

} //end of eddic

#endif
