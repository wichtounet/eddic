//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"
#include "Utils.hpp"
#include "Value.hpp"

#include "il/Operands.hpp"
#include "il/Operand.hpp"

using std::map;
using std::string;
using std::endl;
using std::vector;

using namespace eddic;

Variable::Variable(const std::string& name, Type type, Position position) : m_name(name), m_type(type), m_position(position) {}
Variable::Variable(const std::string& name, Type type, Position position, std::shared_ptr<Value> value) : m_name(name), m_type(type), m_position(position), m_value(value) {}

std::string Variable::name() const  {
    return m_name;
}

Type Variable::type() const {
    return m_type;
}

Position Variable::position() const {
    return m_position;
}

std::shared_ptr<Value> Variable::value() const {
    return m_value;
}

void Variable::moveToRegister(AssemblyFileWriter& writer, const std::string& reg){
    if(m_type == Type::INT){ 
        if(m_position.isStack()){
            writer.stream() << "movl -" << m_position.offset() << "(%ebp), " << reg << endl;
        } else if(m_position.isParameter()){
            writer.stream() << "movl " << m_position.offset() << "(%ebp), " << reg << endl;
        } else if(m_position.isGlobal()){
            writer.stream() << "movl VI" << m_position.name() << ", " << reg << endl;
        }
   } else if (m_type == Type::STRING){
       //TODO Should never be called 
   }
}

void Variable::moveToRegister(AssemblyFileWriter& writer, const std::string& reg1, const std::string& reg2){
    if(m_type == Type::INT){ 
       //TODO Should never be called 
   } else if (m_type == Type::STRING){
       if(m_position.isStack()){
           writer.stream() << "movl -" << m_position.offset() << "(%ebp), " << reg1 << endl;
           writer.stream() << "movl -" << (m_position.offset() + 4) << "(%ebp), " << reg2 << endl;
       } else if(m_position.isParameter()){
           writer.stream() << "movl " << m_position.offset() << "(%ebp), " << reg1 << endl;
           writer.stream() << "movl " << (m_position.offset() + 4) << "(%ebp), " << reg2 << endl;
       } else {
           writer.stream() << "movl VS" << m_position.name() << ", " << reg1 << endl;
           writer.stream() << "movl VS" << m_position.name() << "+4, " << reg2 << endl;
       }
   }
}

void Variable::moveFromRegister(AssemblyFileWriter& writer, const std::string& reg){
    if(m_type == Type::INT){ 
        if(m_position.isStack()){
            writer.stream() << "movl " << reg << ", -" << m_position.offset() << "(%ebp)" << endl;
        } else if(m_position.isParameter()){
            writer.stream() << "movl " << reg << ", " << m_position.offset() << "(%ebp)" << endl;
        } else if(m_position.isGlobal()){
            writer.stream() << "movl " << reg << ", VI" << m_position.name()  << endl;
        }
   } else if (m_type == Type::STRING){
       //TODO Should never be called 
   }
}

void Variable::moveFromRegister(AssemblyFileWriter& writer, const std::string& reg1, const std::string& reg2){
    if(m_type == Type::INT){ 
       //TODO Should never be called 
   } else if (m_type == Type::STRING){
       if(m_position.isStack()){
           writer.stream() << "movl " << reg1 << ", -" << m_position.offset() << "(%ebp)" << endl;
           writer.stream() << "movl " << reg2 << ", -" << (m_position.offset() + 4) << "(%ebp)" << endl;
       } else if(m_position.isParameter()){
           writer.stream() << "movl " << reg1 << ", " << m_position.offset() << "(%ebp)" << endl;
           writer.stream() << "movl " << reg2 << ", " << (m_position.offset() + 4) << "(%ebp)" << endl;
       } else {
           writer.stream() << "movl " << reg1 << ", VS" << m_position.name() << "+4" << endl;
           writer.stream() << "movl " << reg2 << ", VS" << m_position.name() << endl;
       }
   }
}

void Variable::pushToStack(AssemblyFileWriter& writer){
    switch (m_type) {
        case Type::INT:
            if(m_position.isStack()){
                writer.stream() << "pushl -" << m_position.offset() << "(%ebp)" << std::endl;
            } else if(m_position.isParameter()){
                writer.stream() << "pushl " << m_position.offset() << "(%ebp)" << std::endl;
            } else if(m_position.isGlobal()){
                writer.stream() << "pushl VI" << m_position.name() << std::endl;
            }

            break;
        case Type::STRING:
            if(m_position.isStack()){
                writer.stream() << "pushl -" << (m_position.offset() + 4) << "(%ebp)" << std::endl;
                writer.stream() << "pushl -" << m_position.offset() << "(%ebp)" << std::endl;
            } else if(m_position.isParameter()){
                writer.stream() << "pushl " << (m_position.offset() + 4) << "(%ebp)" << std::endl;
                writer.stream() << "pushl " << m_position.offset() << "(%ebp)" << std::endl;
            } else if(m_position.isGlobal()){
                writer.stream() << "pushl VS" << m_position.name() << endl;
                writer.stream() << "pushl VS" << m_position.name() << "+4" << std::endl;
            }

            break;
        default:
            throw CompilerException("Variable of invalid type");
    }
}

void Variable::popFromStack(AssemblyFileWriter& writer){
    switch (m_type) {
        case Type::INT:
            writer.stream() << "movl (%esp), %eax" << endl;
            writer.stream() << "addl $4, %esp" << endl;

            moveFromRegister(writer, "%eax");

            break;
        case Type::STRING:
            writer.stream() << "movl (%esp), %eax" << endl;
            writer.stream() << "movl 4(%esp), %ebx" << endl;
            writer.stream() << "addl $8, %esp" << endl;

            moveFromRegister(writer, "%eax", "%ebx");
            
            break;
        default:
            throw CompilerException("Variable of invalid type");
    }
}

std::shared_ptr<Operand> Variable::toIntegerOperand(){
    //TODO Assert that type is int
    
    if(m_position.isStack()){//TODO Rename in a way that we can understand that it is a variable
        return createBaseStackOperand(-1 * m_position.offset());
    } else if(m_position.isParameter()){
        return createBaseStackOperand(m_position.offset());
    } else if(m_position.isGlobal()){
        return createGlobalOperand(m_position.name());
    }

    throw "ERROR";
}

std::pair<OperandPtr, OperandPtr> Variable::toStringOperand(){
    //TODO Assert that type is string
   
    //TODO Verify
    if(m_position.isStack()){//TODO Rename in a way that we can understand that it is a variable
        return make_pair(
                    createBaseStackOperand(-1 * m_position.offset()), 
                    createBaseStackOperand(-1 * m_position.offset() - 4)
                );
    } else if(m_position.isParameter()){
        return make_pair(
                    createBaseStackOperand(m_position.offset()), 
                    createBaseStackOperand(m_position.offset() + 4)
                );
    } else if(m_position.isGlobal()){
        return make_pair(
                    createGlobalOperand(m_position.name()), 
                    createGlobalOperand(m_position.name(), 4)
                );
    }

    throw "ERROR";
}
