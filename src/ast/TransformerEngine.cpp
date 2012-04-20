//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant/static_visitor.hpp>

#include "ast/TransformerEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

#include "VisitorUtils.hpp"
#include "Variable.hpp"

using namespace eddic;

struct ValueTransformer : public boost::static_visitor<ast::Value> {
    AUTO_RETURN_CAST(ast::Value)
    AUTO_RETURN_FALSE(ast::Value)
    AUTO_RETURN_TRUE(ast::Value)
    AUTO_RETURN_LITERAL(ast::Value)
    AUTO_RETURN_FLOAT(ast::Value)
    AUTO_RETURN_INTEGER(ast::Value)
    AUTO_RETURN_INTEGER_SUFFIX(ast::Value)
    AUTO_RETURN_STRUCT_VALUE(ast::Value)
    AUTO_RETURN_VARIABLE_VALUE(ast::Value)
    AUTO_RETURN_PLUS(ast::Value)
    AUTO_RETURN_MINUS(ast::Value)
    AUTO_RETURN_PREFIX_OPERATION(ast::Value)
    AUTO_RETURN_SUFFIX_OPERATION(ast::Value)
    
    ast::Value operator()(ast::Expression& value){
        if(value.Content->operations.empty()){
            return visit(*this, value.Content->first);   
        }

        value.Content->first = visit(*this, value.Content->first);

        auto start = value.Content->operations.begin();
        auto end = value.Content->operations.end();

        while(start != end){
            start->get<1>() = visit(*this, start->get<1>());

            ++start;
        }

        assert(value.Content->operations.size() > 0); //Once here, there is no more empty composed value 

        return value;
    }

    ast::Value operator()(ast::ArrayValue& value){
        value.Content->indexValue = visit(*this, value.Content->indexValue); 

        return value;
    }

    ast::Value operator()(ast::FunctionCall& functionCall){
        auto start = functionCall.Content->values.begin();
        auto end = functionCall.Content->values.end();

        while(start != end){
            *start = visit(*this, *start);

            ++start;
        }

        return functionCall;
    }

    ast::Value operator()(ast::Assignment& assignment){
        assignment.Content->value = visit(*this, assignment.Content->value);

        return assignment;
    }

    ast::Value operator()(ast::BuiltinOperator& builtin){
        auto start = builtin.Content->values.begin();
        auto end = builtin.Content->values.end();

        while(start != end){
            *start = visit(*this, *start);

            ++start;
        }

        return builtin;
    }
};

struct InstructionTransformer : public boost::static_visitor<ast::Instruction> {
    ast::Instruction operator()(ast::CompoundAssignment& compound) const {
        ast::Assignment assignment;

        assignment.Content->context = compound.Content->context;
        assignment.Content->variableName = compound.Content->variableName;

        ast::VariableValue variable;
        variable.Content->context = compound.Content->context;
        variable.Content->variableName = compound.Content->variableName;
        variable.Content->var = compound.Content->context->getVariable(compound.Content->variableName);

        ast::Expression composed;
        composed.Content->first = variable;
        composed.Content->operations.push_back({compound.Content->op, compound.Content->value});

        assignment.Content->value = composed;

        return assignment;
    }
    
    ast::Instruction operator()(ast::StructCompoundAssignment& compound) const {
        ast::StructAssignment assignment;

        assignment.Content->context = compound.Content->context;
        assignment.Content->variableName = compound.Content->variableName;
        assignment.Content->memberName = compound.Content->memberName;

        ast::StructValue variable;
        variable.Content->context = compound.Content->context;
        variable.Content->variableName = compound.Content->variableName;
        variable.Content->memberName = compound.Content->memberName;
        variable.Content->variable = compound.Content->context->getVariable(compound.Content->variableName);

        ast::Expression composed;
        composed.Content->first = variable;
        composed.Content->operations.push_back({compound.Content->op, compound.Content->value});

        assignment.Content->value = composed;

        return assignment;
    }
    
    //Transform while in do while loop as an optimization (less jumps)
    ast::Instruction operator()(ast::While& while_) const {
        ast::If if_;
        if_.Content->condition = while_.Content->condition;

        ast::DoWhile do_while;
        do_while.Content->condition = while_.Content->condition;
        do_while.Content->instructions = while_.Content->instructions;

        if_.Content->instructions.push_back(do_while);

        return if_;
    }

    ast::Instruction operator()(ast::Foreach& foreach) const {
        ast::For for_;

        //Define the start instruction
    
        ast::Integer fromValue;
        fromValue.value = foreach.Content->from;

        ast::Assignment startAssign;
        startAssign.Content->context = foreach.Content->context;
        startAssign.Content->variableName = foreach.Content->variableName;
        startAssign.Content->value = fromValue;

        for_.Content->start = startAssign;

        //Defne the condition

        ast::Integer toValue;
        toValue.value = foreach.Content->to;

        ast::VariableValue v;
        v.Content->variableName = foreach.Content->variableName;
        v.Content->context = foreach.Content->context;
        v.Content->var = v.Content->context->getVariable(foreach.Content->variableName);

        ast::Expression cond;
        cond.Content->first = v;
        cond.Content->operations.push_back({ast::Operator::LESS_EQUALS, toValue});

        for_.Content->condition = cond;

        //Define the repeat instruction

        ast::Integer inc;
        inc.value = 1;

        ast::Expression addition;
        addition.Content->first = v;
        addition.Content->operations.push_back({ast::Operator::ADD, inc});
        
        ast::Assignment repeatAssign;
        repeatAssign.Content->context = foreach.Content->context;
        repeatAssign.Content->variableName = foreach.Content->variableName;
        repeatAssign.Content->value = addition;

        for_.Content->repeat = repeatAssign;

        //Put the operations into the new for
        for_.Content->instructions = foreach.Content->instructions;

        return for_;
    }

    AUTO_RETURN_OTHERS_CONST(ast::Instruction)
};

struct CleanerVisitor : public boost::static_visitor<> {
    ValueTransformer transformer;

    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_ELSE()
    AUTO_RECURSE_FUNCTION_DECLARATION()
    AUTO_RECURSE_FOREACH()
        
    AUTO_IGNORE_FALSE()
    AUTO_IGNORE_TRUE()
    AUTO_IGNORE_LITERAL()
    AUTO_IGNORE_FLOAT()
    AUTO_IGNORE_INTEGER()
    AUTO_IGNORE_INTEGER_SUFFIX()
    AUTO_IGNORE_IMPORT()
    AUTO_IGNORE_STANDARD_IMPORT()
    AUTO_IGNORE_STRUCT()
    AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION()
    AUTO_IGNORE_ARRAY_DECLARATION()
    AUTO_IGNORE_PREFIX_OPERATION()
    AUTO_IGNORE_SUFFIX_OPERATION()
    AUTO_IGNORE_SWAP()

    void operator()(ast::If& if_){
        if_.Content->condition = visit(transformer, if_.Content->condition);

        visit_each(*this, if_.Content->instructions);
        visit_each_non_variant(*this, if_.Content->elseIfs);
        visit_optional_non_variant(*this, if_.Content->else_);
    }

    void operator()(ast::ElseIf& elseIf){
        elseIf.condition = visit(transformer, elseIf.condition);

        visit_each(*this, elseIf.instructions);
    }

    void operator()(ast::For& for_){
        visit_optional(*this, for_.Content->start);
        if(for_.Content->condition){
            for_.Content->condition = visit(transformer, *for_.Content->condition);
        }
        visit_optional(*this, for_.Content->repeat);
        visit_each(*this, for_.Content->instructions);
    }

    void operator()(ast::While& while_){
        while_.Content->condition = visit(transformer, while_.Content->condition);
        visit_each(*this, while_.Content->instructions);
    }

    void operator()(ast::DoWhile& while_){
        while_.Content->condition = visit(transformer, while_.Content->condition);
        visit_each(*this, while_.Content->instructions);
    }

    void operator()(ast::FunctionCall& functionCall){
        auto start = functionCall.Content->values.begin();
        auto end = functionCall.Content->values.end();

        while(start != end){
            *start = visit(transformer, *start);

            ++start;
        }
    }
    
    void operator()(ast::BuiltinOperator& builtin){
        auto start = builtin.Content->values.begin();
        auto end = builtin.Content->values.end();

        while(start != end){
            *start = visit(transformer, *start);

            ++start;
        }
    }

    void operator()(ast::GlobalVariableDeclaration& declaration){
        if(declaration.Content->value){
            declaration.Content->value = visit(transformer, *declaration.Content->value); 
        }
    }

    void operator()(ast::Assignment& assignment){
        assignment.Content->value = visit(transformer, assignment.Content->value); 
    }
    
    void operator()(ast::CompoundAssignment& assignment){
        assignment.Content->value = visit(transformer, assignment.Content->value); 
    }
    
    void operator()(ast::StructCompoundAssignment& assignment){
        assignment.Content->value = visit(transformer, assignment.Content->value); 
    }

    void operator()(ast::Return& return_){
        return_.Content->value = visit(transformer, return_.Content->value); 
    }

    void operator()(ast::ArrayAssignment& assignment){
        assignment.Content->value = visit(transformer, assignment.Content->value); 
        assignment.Content->indexValue = visit(transformer, assignment.Content->indexValue); 
    }
    
    void operator()(ast::StructAssignment& assignment){
        assignment.Content->value = visit(transformer, assignment.Content->value); 
    }

    void operator()(ast::VariableDeclaration& declaration){
        if(declaration.Content->value){
            declaration.Content->value = visit(transformer, *declaration.Content->value); 
        }
    }

    void operator()(ast::BinaryCondition& binaryCondition){
        binaryCondition.Content->lhs = visit(transformer, binaryCondition.Content->lhs); 
        binaryCondition.Content->rhs = visit(transformer, binaryCondition.Content->rhs); 
    }
};

struct TransformerVisitor : public boost::static_visitor<> {
    InstructionTransformer instructionTransformer;

    AUTO_RECURSE_PROGRAM()
    
    AUTO_IGNORE_ARRAY_DECLARATION()
    AUTO_IGNORE_ARRAY_VALUE()
    AUTO_IGNORE_ASSIGNMENT()
    AUTO_IGNORE_BUILTIN_OPERATOR()
    AUTO_IGNORE_CAST()
    AUTO_IGNORE_VARIABLE_DECLARATION()
    AUTO_IGNORE_VARIABLE_VALUE()
    AUTO_IGNORE_FUNCTION_CALLS()
    AUTO_IGNORE_SWAP()
    AUTO_IGNORE_EXPRESSION()
    AUTO_IGNORE_FALSE()
    AUTO_IGNORE_TRUE()
    AUTO_IGNORE_LITERAL()
    AUTO_IGNORE_FLOAT()
    AUTO_IGNORE_INTEGER()
    AUTO_IGNORE_INTEGER_SUFFIX()
    AUTO_IGNORE_IMPORT()
    AUTO_IGNORE_STANDARD_IMPORT()
    AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION()
    AUTO_IGNORE_GLOBAL_VARIABLE_DECLARATION()
    AUTO_IGNORE_FOREACH_LOOP()
    AUTO_IGNORE_RETURN()
    AUTO_IGNORE_COMPOUND_ASSIGNMENT()
    AUTO_IGNORE_STRUCT()
    AUTO_IGNORE_STRUCT_VALUE()
    AUTO_IGNORE_STRUCT_ASSIGNMENT()
    AUTO_IGNORE_STRUCT_COMPOUND_ASSIGNMENT()
    AUTO_IGNORE_ARRAY_ASSIGNMENT()
    AUTO_IGNORE_PLUS()
    AUTO_IGNORE_MINUS()
    AUTO_IGNORE_PREFIX_OPERATION()
    AUTO_IGNORE_SUFFIX_OPERATION()

    template<typename T>
    void transform(T& instructions){
        auto start = instructions.begin();
        auto end = instructions.end();

        while(start != end){
            *start = visit(instructionTransformer, *start);

            ++start;
        }

        visit_each(*this, instructions);
    }
    
    void operator()(ast::FunctionDeclaration& declaration){
        transform(declaration.Content->instructions);
    }

    void operator()(ast::If& if_){
        visit(*this, if_.Content->condition);
        transform(if_.Content->instructions);
        visit_each_non_variant(*this, if_.Content->elseIfs);
        visit_optional_non_variant(*this, if_.Content->else_);
    }

    void operator()(ast::ElseIf& elseIf){
        visit(*this, elseIf.condition);
        transform(elseIf.instructions);
    }

    void operator()(ast::Else& else_){
        transform(else_.instructions);
    }

    void operator()(ast::For& for_){
        visit_optional(*this, for_.Content->start);
        visit_optional(*this, for_.Content->condition);
        visit_optional(*this, for_.Content->repeat);
        transform(for_.Content->instructions);
    }

    void operator()(ast::ForeachIn& foreach){
        transform(foreach.Content->instructions);
    }

    void operator()(ast::While& while_){
        visit(*this, while_.Content->condition);
        transform(while_.Content->instructions);
    }

    void operator()(ast::DoWhile& while_){
        visit(*this, while_.Content->condition);
        transform(while_.Content->instructions);
    }
};

void ast::cleanAST(ast::SourceFile& program){
    CleanerVisitor visitor;
    visitor(program);
}

void ast::transformAST(ast::SourceFile& program){
    TransformerVisitor visitor;
    visitor(program);
}
