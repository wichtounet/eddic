//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Foreach.hpp"
#include "AssemblyFileWriter.hpp"
#include "Context.hpp"
#include "Operators.hpp"

#include "If.hpp"
#include "Condition.hpp"

using namespace eddic;

//TODO Rewrite that function, perhaps with a transformation into several element in a previous stage
void Foreach::write(AssemblyFileWriter& writer){
    //Assign the base value to the variable
    m_from->write(writer);
    m_var->popFromStack(writer);
    
    static int labels = -1;

    ++labels;

    writer.stream() << "start_foreach" << labels << ":" << std::endl;

    //Create a condition
    std::shared_ptr<Value> variableValue(new VariableValue(m_var));
    std::shared_ptr<Condition> condition(new Condition(LESS_EQUALS_OPERATOR, variableValue, m_to));
    writeJumpIfNot(writer, condition, "end_foreach", labels);

    //Write all the instructions
    ParseNode::write(writer);

    //Increment the variable
    std::shared_ptr<Value> one(new Integer(context(), token(), 1));
    std::shared_ptr<ParseNode> addition(new Addition(context(), token(), one, variableValue));

    addition->write(writer);
    m_var->popFromStack(writer);

    writer.stream() << "jmp start_foreach" << labels << std::endl;

    writer.stream() << "end_foreach" << labels << ":" << std::endl;
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
