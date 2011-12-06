//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "IntermediateCompiler.hpp"

#include "MainDeclaration.hpp"
#include "Methods.hpp"
#include "StringPool.hpp"
#include "VisitorUtils.hpp"

#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "SemanticalException.hpp"
#include "IsImmediateVisitor.hpp"
#include "IsConstantVisitor.hpp"
#include "GetConstantValue.hpp"
#include "GetTypeVisitor.hpp"

#include "mangling.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Operands.hpp"
#include "il/Labels.hpp"
#include "il/Math.hpp"

#include "ast/SourceFile.hpp"

using namespace eddic;

inline bool isImmediate(ast::Value& value){
   return boost::apply_visitor(IsImmediateVisitor(), value); 
}

inline Operation toOperation(char op){
    switch(op){
        case '+':
            return Operation::ADD;
        case '-':
            return Operation::SUB;
        case '/':
            return Operation::DIV;
        case '*':
            return Operation::MUL;
        case '%':
            return Operation::MOD;
        default:
            assert(false);
    }
}

void executeCall(ast::FunctionCall& functionCall, IntermediateProgram& program);
inline void putInRegister(ast::Value& value, std::shared_ptr<Operand> operand, IntermediateProgram& program);

void computeAddressOfElement(std::shared_ptr<Variable> array, int index, IntermediateProgram& program, std::shared_ptr<Operand> operand){
    int offset = -1 * ((index * size(array->type().base())) + size(BaseType::INT));
    
    //We have to go upper to point to the string
    if(array->type().base() == BaseType::STRING){
        offset -= 4;
    }
    
    auto position = array->position();
    if(position.isGlobal()){
        offset += size(array->type().base()) * array->type().size();

        program.addInstruction(program.factory().createMove(createImmediateOperand("VA" + position.name()), operand));
    } else if(position.isStack()){
        offset -= position.offset();

        program.addInstruction(program.factory().createMove(program.registers(EBP), operand));
    } else if(position.isParameter()){
        program.addInstruction(program.factory().createMove(createBaseStackOperand(position.offset()), operand));
    }
        
    program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(offset), operand));
}

void computeAddressOfElement(std::shared_ptr<Variable> array, std::shared_ptr<Operand> indexOperand, IntermediateProgram& program, std::shared_ptr<Operand> operand){
    int offset = -size(BaseType::INT);

    //We have to go upper to point to the string
    if(array->type().base() == BaseType::STRING){
        offset -= 4;
    }
    
    auto position = array->position();
    if(position.isGlobal()){
        offset += size(array->type().base()) * array->type().size();

        program.addInstruction(program.factory().createMove(createImmediateOperand("VA" + position.name()), operand));
    } else if(position.isStack()){
        offset -= position.offset();

        program.addInstruction(program.factory().createMove(program.registers(EBP), operand));
    } else if(position.isParameter()){
        program.addInstruction(program.factory().createMove(createBaseStackOperand(position.offset()), operand));
    }
    
    program.addInstruction(program.factory().createMath(Operation::MUL, createImmediateOperand(-1 * size(array->type().base())), indexOperand));
    program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(offset), indexOperand));
   
    program.addInstruction(program.factory().createMath(Operation::ADD, indexOperand, operand));
}

void computeAddressOfElement(std::shared_ptr<Variable> array, ast::Value indexValue, IntermediateProgram& program, std::shared_ptr<Operand> operand){
    assert(operand->isRegister());

    if(boost::apply_visitor(IsConstantVisitor(), indexValue)){
        int index = boost::get<int>(boost::apply_visitor(GetConstantValue(), indexValue));

        computeAddressOfElement(array, index, program, operand);
    } else {
        auto registerA = program.registers(EAX);

        putInRegister(indexValue, registerA, program);
        computeAddressOfElement(array, registerA, program, operand);
    }
}

void computeAddressOfElement(std::shared_ptr<Variable> array, std::shared_ptr<Variable> indexVar, IntermediateProgram& program, std::shared_ptr<Operand> operand){
    assert(operand->isRegister());

    auto registerA = program.registers(EAX);

    program.addInstruction(program.factory().createMove(indexVar->toIntegerOperand(), registerA));
    computeAddressOfElement(array, registerA, program, operand);
}

void computeLenghtOfArray(std::shared_ptr<Variable> array, IntermediateProgram& program, std::shared_ptr<Operand> operand){
    assert(operand->isRegister());
    
    auto position = array->position();
    if(position.isGlobal()){
        program.addInstruction(program.factory().createMove(createImmediateOperand(array->type().size()), operand));
    } else if(position.isStack()){
        program.addInstruction(program.factory().createMove(createImmediateOperand(array->type().size()), operand));
    } else if(position.isParameter()){
        auto reg = program.registers(EDI);
        program.addInstruction(program.factory().createMove(createBaseStackOperand(position.offset()), reg));
        program.addInstruction(program.factory().createMove(reg->valueOf(), operand));
    }
}

inline std::shared_ptr<Operand> performIntOperation(ast::ComposedValue& value, IntermediateProgram& program){
    assert(value.Content->operations.size() > 0); //This has been enforced by previous phases

    auto registerA = program.registers(ECX);
    auto registerB = program.registers(EBX);

    putInRegister(value.Content->first, registerA, program);

    //Apply all the operations in chain
    for(auto& operation : value.Content->operations){
        if(isImmediate(operation.get<1>())){
            putInRegister(operation.get<1>(), registerB, program);

            //Perform the operation 
            program.addInstruction(program.factory().createMath(toOperation(operation.get<0>()), registerA, registerB));
        } else { //The right value is composed
            program.addInstruction(program.factory().createPush(registerA)); //To be sure that the right operation does not override our register 
            
            putInRegister(operation.get<1>(), registerB, program);

            program.addInstruction(program.factory().createPop(registerA));

            program.addInstruction(program.factory().createMath(toOperation(operation.get<0>()), registerA, registerB));
        }
    }

    return registerB;
}

inline std::pair<std::shared_ptr<Operand>, std::shared_ptr<Operand>> performStringOperation(ast::ComposedValue& value, IntermediateProgram& program);

class PushValue : public boost::static_visitor<> {
    private:
        IntermediateProgram& program;
    
    public:
        PushValue(IntermediateProgram& p) : program(p) {}

        void operator()(ast::Litteral& litteral){
            program.addInstruction(
                program.factory().createPush(
                    createImmediateOperand(litteral.label)
                )
            );
            
            program.addInstruction(
                program.factory().createPush(
                    createImmediateOperand(litteral.value.size() - 2)
                )
            );
        }

        void operator()(ast::Integer& integer){
            program.addInstruction(
                program.factory().createPush(
                    createImmediateOperand(integer.value)
                )
            );
        }

        void operator()(ast::FunctionCall& call){
           executeCall(call, program);
           
           Type type = call.Content->function->returnType;
           
           switch(type.base()){
                case BaseType::INT:
                    program.addInstruction(program.factory().createPush(program.registers(EAX)));

                    break;
                case BaseType::STRING:
                    program.addInstruction(program.factory().createPush(program.registers(EAX)));
                    program.addInstruction(program.factory().createPush(program.registers(EBX)));

                    break;
                default:
                    throw SemanticalException("This function doesn't return anything");   
           }
        }

        void operator()(ast::VariableValue& variable){
            auto var = variable.Content->var;

            //If it's an array, we have to put the adress of the array, not a value
            if(var->type().isArray()){
                auto position = var->position();

                if(position.isGlobal()){
                    auto registerD = program.registers(EDX);
                    auto offset = size(var->type().base()) * var->type().size();

                    program.addInstruction(program.factory().createMove(createImmediateOperand("VA" + position.name()), registerD));
                    program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(offset), registerD));
                    program.addInstruction(program.factory().createPush(registerD));
                } else if(position.isStack()){
                    auto registerD = program.registers(EDX);
                    auto registerE = program.registers(EBP);

                    program.addInstruction(program.factory().createMove(registerE, registerD));
                    program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(-position.offset()), registerD));
                    program.addInstruction(program.factory().createPush(registerD));
                } else if(position.isParameter()){
                    program.addInstruction(program.factory().createPush(createBaseStackOperand(position.offset())));
                }
            } else {
                if(var->type().base() == BaseType::INT){
                    program.addInstruction(
                            program.factory().createPush(
                                var->toIntegerOperand()
                                )
                            );
                } else {
                    auto operands = var->toStringOperand();

                    program.addInstruction(
                            program.factory().createPush(
                                operands.first
                                )
                            );

                    program.addInstruction(
                            program.factory().createPush(
                                operands.second
                                )
                            );
                }
            }
        }

        void operator()(ast::ArrayValue& array){
            auto esi = program.registers(ESI);

            computeAddressOfElement(array.Content->var, array.Content->indexValue, program, esi);
            
            if(array.Content->var->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createPush(esi->valueOf()));
            } else {
                program.addInstruction(program.factory().createPush(esi->valueOf()));
                program.addInstruction(program.factory().createPush(esi->valueOf(4)));
            }
        }

        void operator()(ast::ComposedValue& value){
            Type type = GetTypeVisitor()(value);

            if(type.base() == BaseType::INT){
                program.addInstruction(program.factory().createPush(performIntOperation(value, program)));
            } else if(type.base() == BaseType::STRING){
                auto pair = performStringOperation(value, program);

                program.addInstruction(program.factory().createPush(pair.first));
                program.addInstruction(program.factory().createPush(pair.second));
            }
        } 
};

class AssignValueToOperand : public boost::static_visitor<> {
    private:
        std::shared_ptr<Operand> operand;
        IntermediateProgram& program;
    
    public:
        AssignValueToOperand(std::shared_ptr<Operand> op, IntermediateProgram& p) : operand(op), program(p) {}

        void operator()(ast::Litteral&){
            assert(false); //Cannot assign a string to a single operand
        }

        void operator()(ast::Integer& integer){
            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(integer.value),
                    operand
                )
            ); 
        }

        void operator()(ast::FunctionCall& call){
            assert(call.Content->function->returnType.base() == BaseType::INT);
      
            executeCall(call, program);
            
            program.addInstruction(
                program.factory().createMove(
                    program.registers(EAX),
                    operand
                )
            ); 
        }

        void operator()(ast::VariableValue& variable){
            assert(variable.Content->var->type().base() == BaseType::INT);
        
            program.addInstruction(program.factory().createMove(variable.Content->var->toIntegerOperand(), operand));
        }

        void operator()(ast::ArrayValue& array){
            assert(array.Content->var->type().base() == BaseType::INT);
            
            auto esi = program.registers(ESI);

            computeAddressOfElement(array.Content->var, array.Content->indexValue, program, esi);

            program.addInstruction(program.factory().createMove(esi->valueOf(), operand));
        }

        void operator()(ast::ComposedValue& value){
            assert(GetTypeVisitor()(value).base() == BaseType::INT); //Cannot be used for string operations

            program.addInstruction(program.factory().createMove(performIntOperation(value, program), operand));
        } 
};

class AssignValueToVariable : public boost::static_visitor<> {
    private:
        std::shared_ptr<Variable> variable;
        IntermediateProgram& program;
    
    public:
        AssignValueToVariable(std::shared_ptr<Variable> v, IntermediateProgram& p) : variable(v), program(p) {}

        void operator()(ast::Litteral& litteral){
            auto operands = variable->toStringOperand();

            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(litteral.label),
                    operands.first
                )
            );
            
            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(litteral.value.size() - 2),
                    operands.second
                )
            );
        }

        void operator()(ast::Integer& integer){
            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(integer.value),
                    variable->toIntegerOperand()
                )
            ); 
        }

        void operator()(ast::FunctionCall& call){
            executeCall(call, program);
            
           switch(variable->type().base()){
                case BaseType::INT:
                    program.addInstruction(program.factory().createMove(program.registers(EAX), variable->toIntegerOperand()));

                    break;
                case BaseType::STRING:{
                    auto destination = variable->toStringOperand();

                    program.addInstruction(program.factory().createMove(program.registers(EAX), destination.first));
                    program.addInstruction(program.factory().createMove(program.registers(EBX), destination.second));

                    break;
                }
                default:
                    throw SemanticalException("This function doesn't return anything");   
           }
        }

        void operator()(ast::VariableValue& variableSource){
            auto var = variableSource.Content->var;

            if(var->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(var->toIntegerOperand(), variable->toIntegerOperand()));
            } else {
                auto source = var->toStringOperand();
                auto destination = variable->toStringOperand();

                program.addInstruction(program.factory().createMove(source.first, destination.first));
                program.addInstruction(program.factory().createMove(source.second, destination.second));
            }
        }

        void operator()(ast::ArrayValue& array){
            auto esi = program.registers(ESI);

            computeAddressOfElement(array.Content->var, array.Content->indexValue, program, esi);
            
            if(array.Content->var->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(esi->valueOf(), variable->toIntegerOperand()));
            } else {
                auto destination = variable->toStringOperand();

                program.addInstruction(program.factory().createMove(esi->valueOf(), destination.first));
                program.addInstruction(program.factory().createMove(esi->valueOf(4), destination.second));
            }
        }

        void operator()(ast::ComposedValue& value){
            Type type = GetTypeVisitor()(value);

            if(type.base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(performIntOperation(value, program), variable->toIntegerOperand()));
            } else if(type.base() == BaseType::STRING){
                auto source = performStringOperation(value, program);
                auto destination = variable->toStringOperand();

                program.addInstruction(program.factory().createMove(source.first, destination.first));
                program.addInstruction(program.factory().createMove(source.second, destination.second));
            }
        } 
};

struct AssignValueToArray : public boost::static_visitor<> {
    private:
        std::shared_ptr<Variable> variable;
        ast::Value indexValue;
        IntermediateProgram& program;

    public:
        AssignValueToArray(std::shared_ptr<Variable> v, ast::Value i, IntermediateProgram& p) : variable(v), indexValue(i), program(p) {}

        void operator()(ast::Litteral& litteral){
            assert(variable->type().base() == BaseType::STRING);

            auto edi = program.registers(EDI);

            computeAddressOfElement(variable, indexValue, program, edi);
                
            program.addInstruction(program.factory().createMove(createImmediateOperand(litteral.label), edi->valueOf()));
            program.addInstruction(program.factory().createMove(createImmediateOperand(litteral.value.size() -2), edi->valueOf(4)));
        }
        
        void operator()(ast::Integer& integer){
            assert(variable->type().base() == BaseType::INT);

            auto edi = program.registers(EDI);

            computeAddressOfElement(variable, indexValue, program, edi);
                
            program.addInstruction(program.factory().createMove(createImmediateOperand(integer.value), edi->valueOf()));
        }
        
        void operator()(ast::VariableValue& source){
            auto edi = program.registers(EDI);

            computeAddressOfElement(variable, indexValue, program, edi);
            
            if(variable->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(source.Content->var->toIntegerOperand(), edi->valueOf()));
            } else if(variable->type().base() == BaseType::STRING){
                auto operands = source.Content->var->toStringOperand();

                program.addInstruction(program.factory().createMove(operands.first, edi->valueOf()));
                program.addInstruction(program.factory().createMove(operands.second, edi->valueOf(4)));
            }
        }
        
        void operator()(ast::FunctionCall& call){
            executeCall(call, program);
            
            auto edi = program.registers(EDI);

            computeAddressOfElement(variable, indexValue, program, edi);
            
            switch(variable->type().base()){
                case BaseType::INT:
                    program.addInstruction(program.factory().createMove(program.registers(EAX), edi->valueOf()));

                    break;
                case BaseType::STRING:
                    program.addInstruction(program.factory().createMove(program.registers(EAX), edi->valueOf()));
                    program.addInstruction(program.factory().createMove(program.registers(EBX), edi->valueOf(4)));

                    break;
                default:
                    throw SemanticalException("This function doesn't return anything");   
            }
        }
        
        void operator()(ast::ArrayValue& array){
            auto esi = program.registers(ESI);
            auto edi = program.registers(EDI);

            computeAddressOfElement(array.Content->var, array.Content->indexValue, program, esi);
            computeAddressOfElement(variable, indexValue, program, edi);
          
            if(variable->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(esi->valueOf(), edi->valueOf()));
            } else if(variable->type().base() == BaseType::STRING){
                program.addInstruction(program.factory().createMove(esi->valueOf(), edi->valueOf()));
                program.addInstruction(program.factory().createMove(esi->valueOf(4), edi->valueOf(4)));
            }
        }
        
        void operator()(ast::ComposedValue& value){
            auto edi = program.registers(EDI);

            computeAddressOfElement(variable, indexValue, program, edi);
            
            if(variable->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(performIntOperation(value, program), edi->valueOf()));
            } else if(variable->type().base() == BaseType::STRING){
                auto operands = performStringOperation(value, program);

                program.addInstruction(program.factory().createMove(operands.first, edi->valueOf()));
                program.addInstruction(program.factory().createMove(operands.second, edi->valueOf(4)));
            }
        }
};

inline JumpCondition toJumpNotCondition(std::string op){
    if(op == "!="){
        return JumpCondition::EQUALS;
    } else if(op == "=="){
        return JumpCondition::NOT_EQUALS;
    } else if(op == ">="){
        return JumpCondition::LESS;
    } else if(op == ">"){
        return JumpCondition::LESS_EQUALS;
    } else if(op == "<="){
        return JumpCondition::GREATER;
    } else if(op == "<"){
        return JumpCondition::GREATER_EQUALS;
    }

    assert(false); //Not handled
}

inline void writeILJumpIfNot(IntermediateProgram& program, ast::Condition& condition, const std::string& label) {
    //No need to jump for a true boolean value 
    if(boost::get<ast::False>(&condition)){
        program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label));
    } else if(auto* ptr = boost::get<ast::BinaryCondition>(&condition)){
        ast::BinaryCondition& binaryCondition = *ptr;
        
        AssignValueToOperand leftVisitor(program.registers(EAX), program);

        boost::apply_visitor(leftVisitor, binaryCondition.Content->lhs);
        
        AssignValueToOperand rightVisitor(program.registers(EBX), program);
        boost::apply_visitor(rightVisitor, binaryCondition.Content->rhs);

        program.addInstruction(program.factory().createCompare(program.registers(EBX), program.registers(EAX)));

        program.addInstruction(program.factory().createJump(toJumpNotCondition(binaryCondition.Content->op), label));
    }
}

inline std::pair<std::shared_ptr<Operand>, std::shared_ptr<Operand>> performStringOperation(ast::ComposedValue& value, IntermediateProgram& program){
    assert(value.Content->operations.size() > 0); //Other values must be transformed before that phase

    auto registerA = program.registers(EAX);
    auto registerB = program.registers(EDX);

    PushValue pusher(program); 
    boost::apply_visitor(pusher, value.Content->first);

    unsigned int iter = 0;

    //Perfom all the additions
    for(auto& operation : value.Content->operations){
        boost::apply_visitor(pusher, operation.get<1>());

        program.addInstruction(program.factory().createCall("concat"));
        
        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(16), program.registers(ESP)));

        ++iter;

        //If there is more operation, push the answer
        if(iter < value.Content->operations.size()){
           program.addInstruction(program.factory().createPush(registerA)); 
           program.addInstruction(program.factory().createPush(registerB)); 
        }
    }
    
    return make_pair(registerA, registerB); 
}

inline void putInRegister(ast::Value& value, std::shared_ptr<Operand> operand, IntermediateProgram& program){
    assert(operand->isRegister());

    if(isImmediate(value)){
        AssignValueToOperand visitor(operand, program);
        boost::apply_visitor(visitor, value);
    } else {
        PushValue visitor(program);
        boost::apply_visitor(visitor, value);
        
        program.addInstruction(program.factory().createPop(operand));
    }
}

void executeCall(ast::FunctionCall& functionCall, IntermediateProgram& program){
    PushValue visitor(program);
    for(auto& value : functionCall.Content->values){
        boost::apply_visitor(visitor, value);
    }

    if(functionCall.Content->functionName == "print" || functionCall.Content->functionName == "println"){
        Type type = boost::apply_visitor(GetTypeVisitor(), functionCall.Content->values[0]);

        switch (type.base()) {
            case BaseType::INT:
                program.addInstruction(program.factory().createCall("print_integer"));
                program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(4), program.registers(ESP)));

                break;
            case BaseType::STRING:
                program.addInstruction(program.factory().createCall("print_string"));
                program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(8), program.registers(ESP)));

                break;
            default:
                throw SemanticalException("Variable of invalid type");
        }

        if(functionCall.Content->functionName == "println"){
            program.addInstruction(program.factory().createCall("print_line"));
        }
    } else {
        std::string mangled = mangle(functionCall.Content->functionName, functionCall.Content->values);

        program.addInstruction(program.factory().createCall(mangled));

        int total = 0;

        for(auto& value : functionCall.Content->values){
            Type type = boost::apply_visitor(GetTypeVisitor(), value);   

            if(type.isArray()){
                //Passing an array is just passing an adress
                total += size(BaseType::INT);
            } else {
                total += size(type);
            }
        }

        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(total), program.registers(ESP)));
    }
}

class CompilerVisitor : public boost::static_visitor<> {
    private:
        StringPool& pool;
        IntermediateProgram& program;
    
    public:
        CompilerVisitor(StringPool& p, IntermediateProgram& intermediateProgram) : pool(p), program(intermediateProgram) {}
        
        void operator()(ast::SourceFile& p){
            MainDeclaration().writeIL(program);

            visit_each(*this, p.Content->blocks);

            Methods().writeIL(program);

            pool.writeIL(program);

            p.Content->context->writeIL(program);
        }

        void operator()(ast::FunctionDeclaration& function){
            program.addInstruction(program.factory().createFunctionDeclaration(function.Content->mangledName, function.Content->context->size()));

            auto iter = function.Content->context->begin();
            auto end = function.Content->context->end();

            for(; iter != end; iter++){
                auto var = iter->second;
                if(var->type().isArray() && var->position().isStack()){
                    int position = -var->position().offset();

                    program.addInstruction(program.factory().createMove(createImmediateOperand(var->type().size()), createBaseStackOperand(position))); 

                    if(var->type().base() == BaseType::INT){
                        for(unsigned int i = 0; i < var->type().size(); ++i){
                            program.addInstruction(program.factory().createMove(createImmediateOperand(0), createBaseStackOperand(position -= 4)));
                        }
                    } else if(var->type().base() == BaseType::STRING){
                        for(unsigned int i = 0; i < var->type().size(); ++i){
                            program.addInstruction(program.factory().createMove(createImmediateOperand(0), createBaseStackOperand(position -= 4)));
                            program.addInstruction(program.factory().createMove(createImmediateOperand(0), createBaseStackOperand(position -= 4)));
                        }
                    }
                }
            }

            visit_each(*this, function.Content->instructions);

            program.addInstruction(program.factory().createFunctionExit(function.Content->context->size()));
        }

        void operator()(ast::Import&){
            //Nothing to compile, the content of the dependency have been included into this source. 
        }

        void operator()(ast::StandardImport&){
            //Nothing to compile, the content of the dependency have been included into this source. 
        }

        void operator()(ast::GlobalVariableDeclaration&){
            //Nothing to compile, the global variable values are written using global contexts
        }
        
        void operator()(ast::GlobalArrayDeclaration&){
            //Nothing to compile, the global arrays are written using global contexts
        }

        void operator()(ast::ArrayDeclaration&){
            //Nothing to compile there, everything is done by the function context
        }

        void operator()(ast::If& if_){
            if (if_.Content->elseIfs.empty()) {
                std::string a = newLabel();

                writeILJumpIfNot(program, if_.Content->condition, a);

                visit_each(*this, if_.Content->instructions);

                if (if_.Content->else_) {
                    std::string b = newLabel();

                    program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, b));

                    program.addInstruction(program.factory().createLabel(a));

                    visit_each(*this, (*if_.Content->else_).instructions);

                    program.addInstruction(program.factory().createLabel(b));
                } else {
                    program.addInstruction(program.factory().createLabel(a));
                }
            } else {
                std::string end = newLabel();
                std::string next = newLabel();

                writeILJumpIfNot(program, if_.Content->condition, next);

                visit_each(*this, if_.Content->instructions);

                program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, end));

                for (std::vector<ast::ElseIf>::size_type i = 0; i < if_.Content->elseIfs.size(); ++i) {
                    ast::ElseIf& elseIf = if_.Content->elseIfs[i];

                    program.addInstruction(program.factory().createLabel(next));

                    //Last elseif
                    if (i == if_.Content->elseIfs.size() - 1) {
                        if (if_.Content->else_) {
                            next = newLabel();
                        } else {
                            next = end;
                        }
                    } else {
                        next = newLabel();
                    }

                    writeILJumpIfNot(program, elseIf.condition, next);

                    visit_each(*this, elseIf.instructions);

                    program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, end));
                }

                if (if_.Content->else_) {
                    program.addInstruction(program.factory().createLabel(next));

                    visit_each(*this, (*if_.Content->else_).instructions);
                }

                program.addInstruction(program.factory().createLabel(end));
            }
        }

        void operator()(ast::Assignment& assignment){
            AssignValueToVariable visitor(assignment.Content->context->getVariable(assignment.Content->variableName), program);
            boost::apply_visitor(visitor, assignment.Content->value);
        }
        
        void operator()(ast::ArrayAssignment& assignment){
            AssignValueToArray visitor(assignment.Content->context->getVariable(assignment.Content->variableName), assignment.Content->indexValue, program);
            boost::apply_visitor(visitor, assignment.Content->value);
        }

        void operator()(ast::VariableDeclaration& declaration){
            if(!declaration.Content->const_){
                AssignValueToVariable visitor(declaration.Content->context->getVariable(declaration.Content->variableName), program);
                boost::apply_visitor(visitor, *declaration.Content->value);
            }
        }

        void operator()(ast::Swap& swap){
            auto lhs_var = swap.Content->lhs_var;
            auto rhs_var = swap.Content->rhs_var;

            //We have the guarantee here that both variables are of the same type
            switch (lhs_var->type().base()) {
                case BaseType::INT:{
                    auto registerA = program.registers(EAX);
             
                    auto left = lhs_var->toIntegerOperand();
                    auto right = rhs_var->toIntegerOperand();

                    program.addInstruction(program.factory().createMove(left, registerA));
                    program.addInstruction(program.factory().createMove(right, left));
                    program.addInstruction(program.factory().createMove(registerA, right));

                    break;
                }
                case BaseType::STRING:{
                    auto registerA = program.registers(EAX);
                   
                    auto left = lhs_var->toStringOperand();
                    auto right = rhs_var->toStringOperand();
                    
                    program.addInstruction(program.factory().createMove(left.first, registerA));
                    program.addInstruction(program.factory().createMove(right.first, left.first));
                    program.addInstruction(program.factory().createMove(registerA, right.first));
                    
                    program.addInstruction(program.factory().createMove(left.second, registerA));
                    program.addInstruction(program.factory().createMove(right.second, left.second));
                    program.addInstruction(program.factory().createMove(registerA, right.second));
                    
                    break;
                }
                default:
                   throw SemanticalException("Variable of invalid type");
            }
        }

        void operator()(ast::While& while_){
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            program.addInstruction(program.factory().createLabel(startLabel));

            writeILJumpIfNot(program, while_.Content->condition, endLabel);

            visit_each(*this, while_.Content->instructions);

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, startLabel));

            program.addInstruction(program.factory().createLabel(endLabel));
        }

        void operator()(ast::For for_){
            visit_optional(*this, for_.Content->start);

            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            program.addInstruction(program.factory().createLabel(startLabel));

            if(for_.Content->condition){
                writeILJumpIfNot(program, *for_.Content->condition, endLabel);
            }

            visit_each(*this, for_.Content->instructions);

            visit_optional(*this, for_.Content->repeat);

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, startLabel));

            program.addInstruction(program.factory().createLabel(endLabel));
        }

        void operator()(ast::Foreach&){
            assert(false); //This node has been transformed into a for node
        }
       
        void operator()(ast::ForeachIn& foreach){
            auto iterVar = foreach.Content->iterVar;
            auto arrayVar = foreach.Content->arrayVar;
            auto var = foreach.Content->var;

            auto startLabel = newLabel();
            auto endLabel = newLabel();

            auto registerA = program.registers(EAX);
            auto registerB = program.registers(EBX);
            auto registerE = program.registers(ESI);

            //Init the index to 0
            program.addInstruction(program.factory().createMove(createImmediateOperand(0), iterVar->toIntegerOperand()));

            program.addInstruction(program.factory().createLabel(startLabel));

            computeLenghtOfArray(arrayVar, program, registerA);
            program.addInstruction(program.factory().createMove(iterVar->toIntegerOperand(), registerB));

            program.addInstruction(program.factory().createCompare(program.registers(EAX), program.registers(EBX)));
            program.addInstruction(program.factory().createJump(JumpCondition::GREATER_EQUALS, endLabel));

            computeAddressOfElement(arrayVar, iterVar, program, registerE);

            if(var->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(registerE->valueOf(), var->toIntegerOperand())); 
            } else {
                auto operands = var->toStringOperand();

                program.addInstruction(program.factory().createMove(registerE->valueOf(), operands.first)); 
                program.addInstruction(program.factory().createMove(registerE->valueOf(4), operands.second)); 
            }

            visit_each(*this, foreach.Content->instructions);    

            program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(1), iterVar->toIntegerOperand()));

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, startLabel));
            
            program.addInstruction(program.factory().createLabel(endLabel));
        }

        void operator()(ast::FunctionCall& functionCall){
            executeCall(functionCall, program);
        }

        void operator()(ast::Return& return_){
            if(return_.Content->function->returnType.base() == BaseType::INT) {
                AssignValueToOperand visitor(program.registers(EAX), program);
                boost::apply_visitor(visitor, return_.Content->value);
            } else if(return_.Content->function->returnType.base() == BaseType::STRING) {
                PushValue visitor(program);
                boost::apply_visitor(visitor, return_.Content->value);
          
                program.addInstruction(program.factory().createPop(program.registers(EBX)));
                program.addInstruction(program.factory().createPop(program.registers(EAX)));
            }

            program.addInstruction(program.factory().createFunctionExit(return_.Content->context->size()));
        }
};

void IntermediateCompiler::compile(ast::SourceFile& program, StringPool& pool, IntermediateProgram& intermediateProgram) const {
    CompilerVisitor visitor(pool, intermediateProgram);
    visitor(program);
}
