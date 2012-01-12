//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "TransformerEngine.hpp"

#include <boost/variant/static_visitor.hpp>

#include "GetTypeVisitor.hpp"
#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"
#include "Variable.hpp"

#include "ast/SourceFile.hpp"

using namespace eddic;

struct ValueTransformer : public boost::static_visitor<ast::Value> {
    ast::Value operator()(ast::ComposedValue& value) const {
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

    ast::Value operator()(ast::ArrayValue& value) const {
        value.Content->indexValue = visit(*this, value.Content->indexValue); 

        return value;
    }

    ast::Value operator()(ast::FunctionCall& functionCall) const {
        auto start = functionCall.Content->values.begin();
        auto end = functionCall.Content->values.end();

        while(start != end){
            *start = visit(*this, *start);

            ++start;
        }

        return functionCall;
    }

    //No transformations
    template<typename T>
    ast::Value operator()(T& value) const {
        return value;
    }
};

struct InstructionTransformer : public boost::static_visitor<ast::Instruction> {
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

        ast::BinaryCondition binaryCondition; 
        binaryCondition.Content->lhs = v;
        binaryCondition.Content->rhs = toValue;
        binaryCondition.Content->op = ast::Operator::LESS_EQUALS;

        for_.Content->condition = binaryCondition;

        //Define the repeat instruction

        ast::Integer inc;
        inc.value = 1;

        ast::ComposedValue addition;
        addition.Content->first = v;
        addition.Content->operations.push_back(ast::Operation(ast::Operator::ADD, inc));
        
        ast::Assignment repeatAssign;
        repeatAssign.Content->context = foreach.Content->context;
        repeatAssign.Content->variableName = foreach.Content->variableName;
        repeatAssign.Content->value = addition;

        for_.Content->repeat = repeatAssign;

        //Put the operations into the new for
        for_.Content->instructions = foreach.Content->instructions;

        return for_;
    }
     
    //No transformations
    template<typename T>
    ast::Instruction operator()(T& value) const {
        return value;
    }
};

struct CleanerVisitor : public boost::static_visitor<> {
    ValueTransformer transformer;

    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_DECLARATION()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_FOREACH()

    void operator()(ast::FunctionCall& functionCall) const {
        auto start = functionCall.Content->values.begin();
        auto end = functionCall.Content->values.end();

        while(start != end){
            *start = visit(transformer, *start);

            ++start;
        }
    }

    void operator()(ast::GlobalVariableDeclaration& declaration) const {
        if(declaration.Content->value){
            declaration.Content->value = visit(transformer, *declaration.Content->value); 
        }
    }

    void operator()(ast::Assignment& assignment) const {
        assignment.Content->value = visit(transformer, assignment.Content->value); 
    }

    void operator()(ast::Return& return_) const {
        return_.Content->value = visit(transformer, return_.Content->value); 
    }

    void operator()(ast::ArrayAssignment& assignment) const {
        assignment.Content->value = visit(transformer, assignment.Content->value); 
        assignment.Content->indexValue = visit(transformer, assignment.Content->indexValue); 
    }

    void operator()(ast::VariableDeclaration& declaration) const {
        if(declaration.Content->value){
            declaration.Content->value = visit(transformer, *declaration.Content->value); 
        }
    }

    void operator()(ast::BinaryCondition& binaryCondition) const {
        binaryCondition.Content->lhs = visit(transformer, binaryCondition.Content->lhs); 
        binaryCondition.Content->rhs = visit(transformer, binaryCondition.Content->rhs); 
    }

    //No transformations
    template<typename T>
    void operator()(T&) const {
        //Do nothing
    }
};

struct TransformerVisitor : public boost::static_visitor<> {
    InstructionTransformer instructionTransformer;

    AUTO_RECURSE_PROGRAM()

    template<typename T>
    void transform(T& instructions) const {
        auto start = instructions.begin();
        auto end = instructions.end();

        while(start != end){
            *start = visit(instructionTransformer, *start);

            ++start;
        }

        visit_each(*this, instructions);
    }
    
    void operator()(ast::FunctionDeclaration& declaration) const {
        transform(declaration.Content->instructions);
    }

    void operator()(ast::If& if_) const {
        visit(*this, if_.Content->condition);
        transform(if_.Content->instructions);
        visit_each_non_variant(*this, if_.Content->elseIfs);
        visit_optional_non_variant(*this, if_.Content->else_);
    }

    void operator()(ast::ElseIf& elseIf) const {
        visit(*this, elseIf.condition);
        transform(elseIf.instructions);
    }

    void operator()(ast::Else& else_) const {
        transform(else_.instructions);
    }

    void operator()(ast::For& for_) const {
        visit_optional(*this, for_.Content->start);
        visit_optional(*this, for_.Content->condition);
        visit_optional(*this, for_.Content->repeat);
        transform(for_.Content->instructions);
    }

    void operator()(ast::ForeachIn& foreach) const {
        transform(foreach.Content->instructions);
    }

    void operator()(ast::While& while_) const {
        visit(*this, while_.Content->condition);
        transform(while_.Content->instructions);
    }

    //No transformations
    template<typename T>
    void operator()(T&) const {
        //Do nothing
    }
};

void TransformerEngine::clean(ast::SourceFile& program) const {
    CleanerVisitor visitor;
    visitor(program);
}

void TransformerEngine::transform(ast::SourceFile& program) const {
    TransformerVisitor visitor;
    visitor(program);
}
