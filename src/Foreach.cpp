//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Foreach.hpp"
#include "CompilerException.hpp"
#include "Context.hpp"
#include "Variable.hpp"

#include "If.hpp"
#include "Condition.hpp"
#include "Addition.hpp"

#include "Value.hpp"
#include "VariableValue.hpp"
#include "Integer.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Labels.hpp"

using namespace eddic;

Foreach::Foreach(std::shared_ptr<Context> context, const Tok token, Type type, std::string variable, std::shared_ptr<Value> from, std::shared_ptr<Value> to) : ParseNode(context, token), m_type(type), m_variable(variable), m_from(from), m_to(to) {}

//TODO Rewrite that function, perhaps with a transformation into several element in a previous stage
//TODO Do not use smart pointers here, can use directly stack based values
void Foreach::writeIL(IntermediateProgram& program){
    //Assign the base value to the variable
    m_from->assignTo(m_var, program);
    
    static int labels = -1;

    ++labels;

    program.addInstruction(program.factory().createLabel(label("start_foreach", labels)));

    //Create a condition
    std::shared_ptr<Value> variableValue(new VariableValue(context(), token(), m_var));
    std::shared_ptr<Condition> condition(new Condition(LESS_EQUALS_OPERATOR, variableValue, m_to));
    writeILJumpIfNot(program, condition, "end_foreach", labels);

    //Write all the instructions
    ParseNode::writeIL(program);

    //Increment the variable
    std::shared_ptr<Value> one(new Integer(context(), token(), 1));
    std::shared_ptr<Value> addition(new Addition(context(), token(), one, variableValue));

    addition->assignTo(m_var, program);

    program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label("start_foreach", labels)));

    program.addInstruction(program.factory().createLabel(label("end_foreach", labels)));
}

void Foreach::checkVariables(){
    if(context()->exists(m_variable)){
        throw CompilerException("The variable already exists", token());
    }

    m_var = context()->addVariable(m_variable, m_type);

    m_from->checkVariables();
    m_to->checkVariables();

    ParseNode::checkVariables();
}

void Foreach::checkStrings(StringPool& pool){
    m_from->checkStrings(pool);
    m_to->checkStrings(pool);

    ParseNode::checkStrings(pool);
}

void Foreach::optimize(){
    m_from->optimize();
    m_to->optimize();

    ParseNode::optimize();
}
