//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <unordered_map>
#include <unordered_set>

#include "Types.hpp"

#include "AssemblyFileWriter.hpp"
#include "Position.hpp"

namespace eddic {

class Value;

class Variable {
    private:
        const std::string m_name;
        const Type m_type;
        Position m_position;
        std::shared_ptr<Value> m_value;

    public:
        Variable(const std::string& name, Type type, Position position) : m_name(name), m_type(type), m_position(position) {}
        Variable(const std::string& name, Type type, Position position, std::shared_ptr<Value> value) : m_name(name), m_type(type), m_position(position), m_value(value) {}

        void moveToRegister(AssemblyFileWriter& writer, std::string reg);
        void moveToRegister(AssemblyFileWriter& writer, std::string reg1, std::string reg2);

        void moveFromRegister(AssemblyFileWriter& writer, std::string reg);
        void moveFromRegister(AssemblyFileWriter& writer, std::string reg1, std::string reg2);

        void pushToStack(AssemblyFileWriter& writer);
        void popFromStack(AssemblyFileWriter& writer);

        std::string name() const  {
            return m_name;
        }
        
        Type type() const {
            return m_type;
        }
        
        Position position() const {
            return m_position;
        }
        
        std::shared_ptr<Value> value() const {
            return m_value;
        }
};

} //end of eddic

#endif
