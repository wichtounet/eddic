//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLE_H
#define VARIABLE_H

#include <utility>
#include <string>
#include <map>
#include <vector>
#include <memory>

#include <boost/variant/variant.hpp>

#include "Types.hpp"

#include "Position.hpp"

namespace eddic {

class Value;
class IntermediateProgram;
class Operand;

typedef std::shared_ptr<Operand> OperandPtr;

typedef boost::variant<int, std::pair<std::string, int>> Val;

class Variable {
    private:
        const std::string m_name;
        const Type m_type;
        Position m_position;
        Val v_value;

    public:
        Variable(const std::string& name, Type type, Position position);
        Variable(const std::string& name, Type type, Position position, Val value);

        OperandPtr toIntegerOperand();
        std::pair<OperandPtr, OperandPtr> toStringOperand();

        std::string name() const ;
        Type type() const ;
        Position position() const ;

        Val val() const ;
};

} //end of eddic

#endif
