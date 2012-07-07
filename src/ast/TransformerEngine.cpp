//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant/static_visitor.hpp>

#include "assert.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"

#include "ast/TransformerEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

using namespace eddic;

struct ValueTransformer : public boost::static_visitor<ast::Value> {
    AUTO_RETURN_CAST(ast::Value)
    AUTO_RETURN_FALSE(ast::Value)
    AUTO_RETURN_TRUE(ast::Value)
    AUTO_RETURN_NULL(ast::Value)
    AUTO_RETURN_LITERAL(ast::Value)
    AUTO_RETURN_FLOAT(ast::Value)
    AUTO_RETURN_INTEGER(ast::Value)
    AUTO_RETURN_INTEGER_SUFFIX(ast::Value)
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

    ast::Value operator()(ast::DereferenceValue& value){
        auto left = visit(*this, value.Content->ref); 

        if(auto* ptr = boost::get<ast::VariableValue>(&left)){
            value.Content->ref = *ptr;
        } else if(auto* ptr = boost::get<ast::ArrayValue>(&left)){
            value.Content->ref = *ptr;
        } else {
            ASSERT_PATH_NOT_TAKEN("Unhandled left value type");
        }

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

struct InstructionTransformer : public boost::static_visitor<std::vector<ast::Instruction>> {
    result_type operator()(ast::Assignment& compound) const {
        if(compound.Content->op == ast::Operator::ASSIGN){
            return {};
        }

        ast::Expression composed;
        composed.Content->first = compound.Content->left_value;
        composed.Content->operations.push_back({compound.Content->op, compound.Content->value});

        ast::Assignment assignment;
        assignment.Content->left_value = compound.Content->left_value;
        assignment.Content->value = composed;

        return {assignment};
    }
    
    //Transform while in do while loop as an optimization (less jumps)
    result_type operator()(ast::While& while_) const {
        ast::If if_;
        if_.Content->condition = while_.Content->condition;

        ast::DoWhile do_while;
        do_while.Content->condition = while_.Content->condition;
        do_while.Content->instructions = while_.Content->instructions;

        if_.Content->instructions.push_back(do_while);

        return {if_};
    }

    //Transform foreach loop in do while loop
    result_type operator()(ast::Foreach& foreach) const {
        ast::If if_;

        ast::Integer from_value;
        from_value.value = foreach.Content->from;

        ast::Integer to_value;
        to_value.value = foreach.Content->to;
        
        ast::Expression condition;
        condition.Content->first = from_value;
        condition.Content->operations.push_back({ast::Operator::LESS_EQUALS, to_value});

        if_.Content->condition = condition;

        ast::VariableValue left_value;
        left_value.Content->context = foreach.Content->context;
        left_value.Content->variableName = foreach.Content->variableName;
        left_value.Content->var = foreach.Content->context->getVariable(foreach.Content->variableName);
        
        ast::Assignment start_assign;
        start_assign.Content->left_value = left_value;
        start_assign.Content->value = from_value;

        if_.Content->instructions.push_back(start_assign);

        ast::DoWhile do_while;
        
        ast::VariableValue v;
        v.Content->variableName = foreach.Content->variableName;
        v.Content->context = foreach.Content->context;
        v.Content->var = v.Content->context->getVariable(foreach.Content->variableName);

        ast::Expression while_condition;
        while_condition.Content->first = v;
        while_condition.Content->operations.push_back({ast::Operator::LESS_EQUALS, to_value});

        do_while.Content->condition = while_condition;
        do_while.Content->instructions = foreach.Content->instructions;

        ast::Integer inc;
        inc.value = 1;

        ast::Expression addition;
        addition.Content->first = v;
        addition.Content->operations.push_back({ast::Operator::ADD, inc});
        
        ast::Assignment repeat_assign;
        repeat_assign.Content->left_value = left_value;
        repeat_assign.Content->value = addition;
        
        do_while.Content->instructions.push_back(repeat_assign);

        if_.Content->instructions.push_back(do_while);

        return {if_};
    }

    //Transform foreach loop in do while loop
    result_type operator()(ast::ForeachIn& foreach) const {
        result_type instructions;
            
        auto iterVar = foreach.Content->iterVar;
        auto arrayVar = foreach.Content->arrayVar;
        auto var = foreach.Content->var;
        
        ast::Integer init_value;
        init_value.value = 0;
        
        ast::VariableValue left_value;
        left_value.Content->context = foreach.Content->context;
        left_value.Content->variableName = iterVar->name();
        left_value.Content->var = foreach.Content->context->getVariable(iterVar->name());
        
        ast::Assignment init_assign;
        init_assign.Content->left_value = left_value;
        init_assign.Content->value = init_value;

        instructions.push_back(init_assign);
        
        ast::VariableValue iter_var_value;
        iter_var_value.Content->var = iterVar;
        iter_var_value.Content->variableName = iterVar->name();
        iter_var_value.Content->context = foreach.Content->context;
        
        ast::VariableValue array_var_value;
        array_var_value.Content->var = arrayVar;
        array_var_value.Content->variableName = arrayVar->name();
        array_var_value.Content->context = foreach.Content->context;

        ast::BuiltinOperator size_builtin; 
        size_builtin.Content->type = ast::BuiltinType::SIZE;
        size_builtin.Content->values.push_back(array_var_value);

        ast::Expression while_condition;
        while_condition.Content->first = iter_var_value;
        while_condition.Content->operations.push_back({ast::Operator::LESS, size_builtin});

        ast::If if_;
        if_.Content->condition = while_condition;

        ast::DoWhile do_while;
        do_while.Content->condition = while_condition;

        ast::ArrayValue array_value;
        array_value.Content->context = foreach.Content->context;
        array_value.Content->arrayName = foreach.Content->arrayName;
        array_value.Content->var = arrayVar;
        array_value.Content->indexValue = iter_var_value;

        ast::VariableDeclaration variable_declaration;
        variable_declaration.Content->context = foreach.Content->context;
        variable_declaration.Content->value = array_value;
        variable_declaration.Content->variableName = var->name();
        
        do_while.Content->instructions.push_back(variable_declaration);

        //Insert all the instructions of the foreach
        std::copy(foreach.Content->instructions.begin(), foreach.Content->instructions.end(), std::back_inserter(do_while.Content->instructions));

        ast::Integer inc;
        inc.value = 1;

        ast::Expression addition;
        addition.Content->first = iter_var_value;
        addition.Content->operations.push_back({ast::Operator::ADD, inc});
        
        ast::Assignment repeat_assign;
        repeat_assign.Content->left_value = left_value;
        repeat_assign.Content->value = addition;

        do_while.Content->instructions.push_back(repeat_assign);

        if_.Content->instructions.push_back(do_while);

        instructions.push_back(if_);

        return instructions;
    }
    
    //Transform for loop in do while loop
    result_type operator()(ast::For& for_) const {
        result_type instructions;

        if(for_.Content->start){
            instructions.push_back(*for_.Content->start);
        }

        if(for_.Content->condition){
            ast::If if_;
            if_.Content->condition = *for_.Content->condition; 

            ast::DoWhile do_while;
            do_while.Content->condition = *for_.Content->condition; 
            do_while.Content->instructions = for_.Content->instructions;
            
            if(for_.Content->repeat){
                do_while.Content->instructions.push_back(*for_.Content->repeat);
            }

            if_.Content->instructions.push_back(do_while);

            instructions.push_back(if_);
        } else {
            ast::DoWhile do_while;

            ast::True condition;
            do_while.Content->condition = condition;
            do_while.Content->instructions = for_.Content->instructions;
            
            if(for_.Content->repeat){
                do_while.Content->instructions.push_back(*for_.Content->repeat);
            }

            instructions.push_back(do_while);
        }

        return instructions;
    }

    //No transformation for the other nodes
    template<typename T>
    result_type operator()(T&) const {
        return {};//Empty vector means no transformation
    }
};

struct CleanerVisitor : public boost::static_visitor<> {
    ValueTransformer transformer;

    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_ELSE()
    AUTO_RECURSE_FUNCTION_DECLARATION()
    AUTO_RECURSE_FOREACH()
        
    AUTO_IGNORE_FALSE()
    AUTO_IGNORE_TRUE()
    AUTO_IGNORE_NULL()
    AUTO_IGNORE_LITERAL()
    AUTO_IGNORE_FLOAT()
    AUTO_IGNORE_INTEGER()
    AUTO_IGNORE_INTEGER_SUFFIX()
    AUTO_IGNORE_IMPORT()
    AUTO_IGNORE_STANDARD_IMPORT()
    AUTO_IGNORE_STRUCT()
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
    
    void operator()(ast::GlobalArrayDeclaration& declaration){
        declaration.Content->size = visit(transformer, declaration.Content->size); 
    }
    
    void operator()(ast::ArrayDeclaration& declaration){
        declaration.Content->size = visit(transformer, declaration.Content->size); 
    }

    void operator()(ast::GlobalVariableDeclaration& declaration){
        if(declaration.Content->value){
            declaration.Content->value = visit(transformer, *declaration.Content->value); 
        }
    }

    void operator()(ast::Assignment& assignment){
        assignment.Content->left_value = ast::to_left_value(visit(transformer, assignment.Content->left_value)); 
        assignment.Content->value = visit(transformer, assignment.Content->value); 
    }

    void operator()(ast::Return& return_){
        return_.Content->value = visit(transformer, return_.Content->value); 
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
    AUTO_IGNORE_DEREFERENCE_VALUE()
    AUTO_IGNORE_FUNCTION_CALLS()
    AUTO_IGNORE_SWAP()
    AUTO_IGNORE_EXPRESSION()
    AUTO_IGNORE_FALSE()
    AUTO_IGNORE_TRUE()
    AUTO_IGNORE_NULL()
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
    AUTO_IGNORE_STRUCT()
    AUTO_IGNORE_PLUS()
    AUTO_IGNORE_MINUS()
    AUTO_IGNORE_PREFIX_OPERATION()
    AUTO_IGNORE_SUFFIX_OPERATION()

    template<typename T>
    void transform(T& instructions){
        auto start = instructions.begin();
        auto end = instructions.end();

        while(start != end){
            auto transformed = visit(instructionTransformer, *start);

            if(transformed.size() == 1){
                *start = transformed[0];
            } else if(transformed.size() == 2){
                //Replace the current instruction with the first one
                *start = transformed[0];

                //Insert the other instructions after the previously inserted
                start = instructions.insert(start+1, transformed[1]);

                //Update the end iterator
                end = instructions.end();
            }

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
