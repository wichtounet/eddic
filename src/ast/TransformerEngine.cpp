//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"
#include "cpp_utils/tmp.hpp"

#include "variant.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"
#include "iterators.hpp"
#include "GlobalContext.hpp"

#include "ast/TransformerEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/GetTypeVisitor.hpp"

using namespace eddic;

namespace {

struct ValueCleaner : public boost::static_visitor<ast::Value> {
    AUTO_RETURN_BOOLEAN(ast::Value)
    AUTO_RETURN_NULL(ast::Value)
    AUTO_RETURN_LITERAL(ast::Value)
    AUTO_RETURN_CHAR_LITERAL(ast::Value)
    AUTO_RETURN_FLOAT(ast::Value)
    AUTO_RETURN_INTEGER(ast::Value)
    AUTO_RETURN_INTEGER_SUFFIX(ast::Value)
    AUTO_RETURN_VARIABLE_VALUE(ast::Value)

    ast::Value operator()(ast::Expression& value){
        if(value.operations.empty()){
            return visit(*this, value.first);
        }

        value.first = visit(*this, value.first);

        for(auto& op : value.operations){
            if(ast::has_operation_value(op)){
                op.get<1>() = visit(*this, op.get<1>());
            }
        }

        assert(value.operations.size() > 0); //Once here, there is no more empty composed value

        return ast::Value(value);
    }

    ast::Value operator()(ast::Cast& cast){
        cast.value = visit(*this, cast.value);

        return ast::Value(cast);
    }

    ast::Value operator()(ast::FunctionCall& functionCall){
        for(auto& value : functionCall.values){
            value = visit(*this, value);
        }

        return ast::Value(functionCall);
    }

    ast::Value operator()(ast::New& new_){
        for(auto& value : new_.values){
            value = visit(*this, value);
        }

        return ast::Value(new_);
    }

    ast::Value operator()(ast::NewArray& new_){
        new_.size = visit(*this, new_.size);

        return ast::Value(new_);
    }

    ast::Value operator()(ast::Assignment& assignment){
        assignment.left_value = visit(*this, assignment.left_value);
        assignment.value = visit(*this, assignment.value);

        return ast::Value(assignment);
    }

    ast::Value operator()(ast::Ternary& ternary){
        ternary.condition = visit(*this, ternary.condition);
        ternary.true_value = visit(*this, ternary.true_value);
        ternary.false_value = visit(*this, ternary.false_value);

        return ast::Value(ternary);
    }

    ast::Value operator()(ast::PrefixOperation& operation){
        operation.left_value = visit(*this, operation.left_value);

        return ast::Value(operation);
    }

    ast::Value operator()(ast::BuiltinOperator& builtin){
        for(auto& value : builtin.values){
            value = visit(*this, value);
        }

        return ast::Value(builtin);
    }
};

struct ValueTransformer : public boost::static_visitor<ast::Value> {
    AUTO_RETURN_BOOLEAN(ast::Value)
    AUTO_RETURN_NULL(ast::Value)
    AUTO_RETURN_LITERAL(ast::Value)
    AUTO_RETURN_CHAR_LITERAL(ast::Value)
    AUTO_RETURN_FLOAT(ast::Value)
    AUTO_RETURN_INTEGER(ast::Value)
    AUTO_RETURN_INTEGER_SUFFIX(ast::Value)
    AUTO_RETURN_VARIABLE_VALUE(ast::Value)

    ast::Value operator()(ast::Cast& cast){
        cast.value = visit(*this, cast.value);

        return ast::Value(cast);
    }

    ast::Value operator()(ast::Expression& value){
        value.first = visit(*this, value.first);

        for(auto& op : value.operations){
            if(ast::has_operation_value(op)){
                op.get<1>() = visit(*this, op.get<1>());
            }
        }

        return ast::Value(value);
    }

    ast::Value operator()(ast::FunctionCall& functionCall){
        for(auto& value : functionCall.values){
            value = visit(*this, value);
        }

        return ast::Value(functionCall);
    }

    ast::Value operator()(ast::New& new_){
        for(auto& value : new_.values){
            value = visit(*this, value);
        }

        return ast::Value(new_);
    }

    ast::Value operator()(ast::NewArray& new_){
        new_.size = visit(*this, new_.size);

        return ast::Value(new_);
    }

    ast::Value operator()(ast::Assignment& assignment){
        assignment.left_value = visit(*this, assignment.left_value);
        assignment.value = visit(*this, assignment.value);

        return ast::Value(assignment);
    }

    ast::Value operator()(ast::Ternary& ternary){
        ternary.condition = visit(*this, ternary.condition);
        ternary.true_value = visit(*this, ternary.true_value);
        ternary.false_value = visit(*this, ternary.false_value);

        return ast::Value(ternary);
    }

    ast::Value operator()(ast::PrefixOperation& operation){
        operation.left_value = visit(*this, operation.left_value);

        return ast::Value(operation);
    }

    ast::Value operator()(ast::BuiltinOperator& builtin){
        for(auto& value : builtin.values){
            value = visit(*this, value);
        }

        return ast::Value(builtin);
    }
};

struct InstructionTransformer : public boost::static_visitor<std::vector<ast::Instruction>> {
    result_type operator()(ast::Assignment& compound) const {
        if(compound.op == ast::Operator::ASSIGN || compound.op == ast::Operator::SWAP){
            return {};
        }

        ast::Expression composed;
        composed.context = compound.context;
        composed.first = compound.left_value;
        composed.operations.push_back(boost::make_tuple(compound.op, ast::Value(compound.value)));

        ast::Assignment assignment;
        assignment.context = compound.context;
        assignment.left_value = compound.left_value;
        assignment.value = composed;

        return {ast::Instruction(assignment)};
    }

    //Transform while in do while loop as an optimization (less jumps)
    result_type operator()(ast::While& while_) const {
        ast::If if_;
        if_.context = while_.context;
        if_.condition = while_.condition;

        ast::DoWhile do_while;
        do_while.context = while_.context;
        do_while.condition = while_.condition;
        do_while.instructions = while_.instructions;

        if_.instructions.emplace_back(do_while);

        return {ast::Instruction(if_)};
    }

    //Transform foreach loop in do while loop
    result_type operator()(ast::Foreach& foreach) const {
        ast::Integer from_value;
        from_value.value = foreach.from;

        ast::Integer to_value;
        to_value.value = foreach.to;

        ast::Expression condition;
        condition.context = foreach.context;
        condition.first = from_value;
        condition.operations.push_back(boost::make_tuple(ast::Operator::LESS_EQUALS, ast::Value(to_value)));

        ast::If if_;
        if_.context = foreach.context;
        if_.condition = condition;

        ast::VariableValue left_value;
        left_value.context = foreach.context;
        left_value.variableName = foreach.variableName;
        left_value.var = foreach.context->getVariable(foreach.variableName);

        ast::Assignment start_assign;
        start_assign.context = foreach.context;
        start_assign.left_value = left_value;
        start_assign.value = from_value;

        if_.instructions.emplace_back(start_assign);

        ast::VariableValue v;
        v.variableName = foreach.variableName;
        v.context = foreach.context;
        v.var = v.context->getVariable(foreach.variableName);

        ast::Expression while_condition;
        while_condition.context = foreach.context;
        while_condition.first = v;
        while_condition.operations.push_back(boost::make_tuple(ast::Operator::LESS_EQUALS, ast::Value(to_value)));

        ast::DoWhile do_while;
        do_while.context = foreach.context;
        do_while.condition = while_condition;
        do_while.instructions = foreach.instructions;

        ast::Integer inc;
        inc.value = 1;

        ast::Expression addition;
        addition.context = foreach.context;
        addition.first = v;
        addition.operations.push_back(boost::make_tuple(ast::Operator::ADD, ast::Value(inc)));

        ast::Assignment repeat_assign;
        repeat_assign.context = foreach.context;
        repeat_assign.left_value = left_value;
        repeat_assign.value = addition;

        do_while.instructions.emplace_back(repeat_assign);

        if_.instructions.emplace_back(do_while);

        return {ast::Instruction(if_)};
    }

    //Transform foreach loop in do while loop
    result_type operator()(ast::ForeachIn& foreach) const {
        result_type instructions;

        auto iterVar = foreach.iterVar;
        auto arrayVar = foreach.arrayVar;
        auto var = foreach.var;

        ast::Integer init_value;
        init_value.value = 0;

        ast::VariableValue left_value;
        left_value.context = foreach.context;
        left_value.variableName = iterVar->name();
        left_value.var = foreach.context->getVariable(iterVar->name());

        ast::Assignment init_assign;
        init_assign.context = foreach.context;
        init_assign.left_value = left_value;
        init_assign.value = init_value;

        instructions.emplace_back(init_assign);

        ast::VariableValue iter_var_value;
        iter_var_value.var = iterVar;
        iter_var_value.variableName = iterVar->name();
        iter_var_value.context = foreach.context;

        ast::VariableValue array_var_value;
        array_var_value.var = arrayVar;
        array_var_value.variableName = arrayVar->name();
        array_var_value.context = foreach.context;

        ast::BuiltinOperator size_builtin;
        size_builtin.values.emplace_back(array_var_value);

        if(arrayVar->type()->is_array()){
            size_builtin.type = ast::BuiltinType::SIZE;
        } else {
            size_builtin.type = ast::BuiltinType::LENGTH;
        }

        ast::Expression while_condition;
        while_condition.context = foreach.context;
        while_condition.first = iter_var_value;
        while_condition.operations.push_back(boost::make_tuple(ast::Operator::LESS, ast::Value(size_builtin)));

        ast::If if_;
        if_.context = foreach.context;
        if_.condition = while_condition;

        ast::DoWhile do_while;
        do_while.context = foreach.context;
        do_while.condition = while_condition;

        ast::VariableValue array_var_value_2;
        array_var_value_2.var = arrayVar;
        array_var_value_2.variableName = arrayVar->name();
        array_var_value_2.context = foreach.context;

        ast::Expression array_value;
        array_value.context = foreach.context;
        array_value.first = array_var_value_2;
        array_value.operations.push_back(boost::make_tuple(ast::Operator::BRACKET, ast::Value(iter_var_value)));

        ast::VariableDeclaration variable_declaration;
        variable_declaration.context = foreach.context;
        variable_declaration.value = ast::Value(array_value);
        variable_declaration.variableName = var->name();

        do_while.instructions.emplace_back(variable_declaration);

        //Insert all the instructions of the foreach
        std::copy(foreach.instructions.begin(), foreach.instructions.end(), std::back_inserter(do_while.instructions));

        ast::Integer inc;
        inc.value = 1;

        ast::Expression addition;
        addition.context = foreach.context;
        addition.first = iter_var_value;
        addition.operations.push_back(boost::make_tuple(ast::Operator::ADD, ast::Value(inc)));

        ast::Assignment repeat_assign;
        repeat_assign.context = foreach.context;
        repeat_assign.left_value = left_value;
        repeat_assign.value = ast::Value(addition);

        do_while.instructions.emplace_back(repeat_assign);

        if_.instructions.emplace_back(do_while);

        instructions.emplace_back(if_);

        return instructions;
    }

    //Transform for loop in do while loop
    result_type operator()(ast::For& for_) const {
        result_type instructions;

        if(for_.start){
            instructions.push_back(*for_.start);
        }

        if(for_.condition){
            ast::DoWhile do_while;
            do_while.context = for_.context;
            do_while.condition = *for_.condition;
            do_while.instructions = for_.instructions;

            if(for_.repeat){
                do_while.instructions.push_back(*for_.repeat);
            }

            ast::If if_;
            if_.context = for_.context;
            if_.condition = *for_.condition;
            if_.instructions.emplace_back(do_while);

            instructions.emplace_back(if_);
        } else {
            ast::Boolean condition{true};

            ast::DoWhile do_while;
            do_while.context = for_.context;
            do_while.condition = condition;
            do_while.instructions = for_.instructions;

            if(for_.repeat){
                do_while.instructions.push_back(*for_.repeat);
            }

            instructions.emplace_back(do_while);
        }

        return instructions;
    }

    result_type operator()(ast::Switch& switch_) const {
        auto cases = switch_.cases;
        auto value_type = visit(ast::GetTypeVisitor(), switch_.value);

        if(value_type == INT){
            ast::Expression first_condition;
            first_condition.context = switch_.context;
            first_condition.first = ast::Value(switch_.value);
            first_condition.operations.emplace_back(ast::Operator::EQUALS, cases[0].value);

            ast::If if_;
            if_.context = switch_.context;
            if_.condition = first_condition;
            if_.instructions = cases[0].instructions;

            for(const auto& case_ : cases){
                ast::Expression condition;
                condition.context = switch_.context;
                condition.first = ast::Value(switch_.value);
                condition.operations.emplace_back(ast::Operator::EQUALS, case_.value);

                ast::ElseIf else_if;
                else_if.context = case_.context;
                else_if.condition = condition;
                else_if.instructions = case_.instructions;

                if_.elseIfs.push_back(else_if);
            }

            if(switch_.default_case){
                ast::Else else_;
                else_.context = (*switch_.default_case).context;
                else_.instructions = (*switch_.default_case).instructions;

                if_.else_ = else_;
            }

            return {ast::Instruction(if_)};
        } else if(value_type == STRING){
            ast::FunctionCall first_condition;
            first_condition.context = switch_.context;
            first_condition.position = switch_.position;
            first_condition.function_name = "str_equals";
            first_condition.mangled_name = "_F10str_equalsSS";
            first_condition.values.push_back(switch_.value);
            first_condition.values.push_back(cases[0].value);

            ast::If if_;
            if_.context = switch_.context;
            if_.condition = first_condition;
            if_.instructions = cases[0].instructions;

            for(const auto& case_ : cases){
                ast::FunctionCall condition;
                condition.context = switch_.context;
                condition.position = case_.position;
                condition.function_name = "str_equals";
                condition.mangled_name = "_F10str_equalsSS";
                condition.values.push_back(switch_.value);
                condition.values.push_back(case_.value);

                ast::ElseIf else_if;
                else_if.context = case_.context;
                else_if.condition = condition;
                else_if.instructions = case_.instructions;

                if_.elseIfs.push_back(else_if);
            }

            if(switch_.default_case){
                ast::Else else_;
                else_.context = (*switch_.default_case).context;
                else_.instructions = (*switch_.default_case).instructions;

                if_.else_ = else_;
            }

            return {ast::Instruction(if_)};
        } else {
            cpp_unreachable("Unhandled switch value type");
        }
    }

    //No transformation for the other nodes
    template<typename T>
    result_type operator()(x3::forward_ast<T>& a) const {
        return (*this)(a.get());
    }

    template<typename T>
    result_type operator()(T&) const {
        return {};//Empty vector means no transformation
    }
};

struct CleanerVisitor : public boost::static_visitor<> {
    ValueCleaner transformer;

    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_ELSE()
    AUTO_RECURSE_ALL_FUNCTION_DECLARATION()
    AUTO_RECURSE_CONSTRUCTOR()
    AUTO_RECURSE_DESTRUCTOR()
    AUTO_RECURSE_SCOPE()
    AUTO_RECURSE_FOREACH()

    AUTO_IGNORE_MEMBER_DECLARATION()
    AUTO_IGNORE_BOOLEAN()
    AUTO_IGNORE_NULL()
    AUTO_IGNORE_LITERAL()
    AUTO_IGNORE_CHAR_LITERAL()
    AUTO_IGNORE_FLOAT()
    AUTO_IGNORE_INTEGER()
    AUTO_IGNORE_INTEGER_SUFFIX()
    AUTO_IGNORE_IMPORT()
    AUTO_IGNORE_STANDARD_IMPORT()

    void operator()(ast::struct_definition& struct_){
        visit_each(*this, struct_.blocks);
    }

    void operator()(ast::If& if_){
        if_.condition = visit(transformer, if_.condition);

        visit_each(*this, if_.instructions);
        visit_each_non_variant(*this, if_.elseIfs);
        visit_optional_non_variant(*this, if_.else_);
    }

    void operator()(ast::ElseIf& elseIf){
        elseIf.condition = visit(transformer, elseIf.condition);

        visit_each(*this, elseIf.instructions);
    }

    void operator()(ast::For& for_){
        visit_optional(*this, for_.start);
        if(for_.condition){
            for_.condition = visit(transformer, *for_.condition);
        }
        visit_optional(*this, for_.repeat);
        visit_each(*this, for_.instructions);
    }

    void operator()(ast::While& while_){
        while_.condition = visit(transformer, while_.condition);
        visit_each(*this, while_.instructions);
    }

    void operator()(ast::DoWhile& while_){
        while_.condition = visit(transformer, while_.condition);
        visit_each(*this, while_.instructions);
    }

    void operator()(ast::Switch& switch_){
        visit_each_non_variant(*this, switch_.cases);
        switch_.value = visit(transformer, switch_.value);
        visit_optional_non_variant(*this, switch_.default_case);
    }

    void operator()(ast::SwitchCase& switch_case){
        switch_case.value = visit(transformer, switch_case.value);
        visit_each(*this, switch_case.instructions);
    }

    void operator()(ast::DefaultCase& default_case){
        visit_each(*this, default_case.instructions);
    }

    void operator()(ast::FunctionCall& functionCall){
        for(auto& value : functionCall.values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::BuiltinOperator& builtin){
        for(auto& value : builtin.values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::GlobalArrayDeclaration& declaration){
        declaration.size = visit(transformer, declaration.size);
    }

    void operator()(ast::ArrayDeclaration& declaration){
        declaration.size = visit(transformer, declaration.size);
    }

    void operator()(ast::GlobalVariableDeclaration& declaration){
        if(declaration.value){
            declaration.value = visit(transformer, *declaration.value);
        }
    }

    void operator()(ast::Assignment& assignment){
        assignment.left_value = visit(transformer, assignment.left_value);
        assignment.value = visit(transformer, assignment.value);
    }

    void operator()(ast::Ternary& ternary){
        ternary.condition = visit(transformer, ternary.condition);
        ternary.true_value = visit(transformer, ternary.true_value);
        ternary.false_value = visit(transformer, ternary.false_value);
    }

    void operator()(ast::Return& return_){
        return_.value = visit(transformer, return_.value);
    }

    void operator()(ast::Delete& delete_){
        delete_.value = visit(transformer, delete_.value);
    }

    void operator()(ast::PrefixOperation& operation){
        operation.left_value = visit(transformer, operation.left_value);
    }

    void operator()(ast::StructDeclaration& declaration){
        for(auto& value : declaration.values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::New& new_){
        for(auto& value : new_.values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::NewArray& new_){
        new_.size = visit(transformer, new_.size);
    }

    void operator()(ast::VariableDeclaration& declaration){
        if(declaration.value){
            declaration.value = visit(transformer, *declaration.value);
        }
    }

    void operator()(ast::Expression& expression){
        visit_non_variant(transformer, expression);
    }
};

struct TransformerVisitor : public boost::static_visitor<> {
    InstructionTransformer instructionTransformer;
    ValueTransformer transformer;

    AUTO_RECURSE_PROGRAM()

    AUTO_IGNORE_MEMBER_DECLARATION()
    AUTO_IGNORE_DELETE()
    AUTO_IGNORE_IMPORT()
    AUTO_IGNORE_STANDARD_IMPORT()
    AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION()
    AUTO_IGNORE_GLOBAL_VARIABLE_DECLARATION()

    void operator()(ast::struct_definition& struct_){
        if(!struct_.is_template_declaration()){
            visit_each(*this, struct_.blocks);
        }
    }

    template<typename T>
    void transform(T& instructions){
        auto start = instructions.begin();
        auto end = instructions.end();

        while(start != end){
            auto transformed = visit(instructionTransformer, *start);

            if(transformed.size() == 1){
                *start = transformed[0];
            } else if(transformed.size() > 1){
                //Replace the current instruction with a scope of instructions
                *start = ast::Scope{{std::move(transformed)}};
            }

            ++start;
        }

        visit_each(*this, instructions);
    }

    void operator()(ast::TemplateFunctionDeclaration& declaration){
        if(!declaration.is_template()){
            transform(declaration.instructions);
        }
    }

    void operator()(ast::Constructor& declaration){
        transform(declaration.instructions);
    }

    void operator()(ast::Scope& declaration){
        transform(declaration.instructions);
    }

    void operator()(ast::Destructor& declaration){
        transform(declaration.instructions);
    }

    void operator()(ast::If& if_){
        if_.condition = visit(transformer, if_.condition);
        transform(if_.instructions);
        visit_each_non_variant(*this, if_.elseIfs);
        visit_optional_non_variant(*this, if_.else_);
    }

    void operator()(ast::ElseIf& elseIf){
        elseIf.condition = visit(transformer, elseIf.condition);
        transform(elseIf.instructions);
    }

    void operator()(ast::Else& else_){
        transform(else_.instructions);
    }

    void operator()(ast::For& for_){
        visit_optional(*this, for_.start);

        if(for_.condition){
            for_.condition = visit(transformer, *for_.condition);
        }

        visit_optional(*this, for_.repeat);
        transform(for_.instructions);
    }

    void operator()(ast::Foreach& foreach){
        transform(foreach.instructions);
    }

    void operator()(ast::ForeachIn& foreach){
        transform(foreach.instructions);
    }

    void operator()(ast::While& while_){
        while_.condition = visit(transformer, while_.condition);
        transform(while_.instructions);
    }

    void operator()(ast::DoWhile& while_){
        while_.condition = visit(transformer, while_.condition);
        transform(while_.instructions);
    }

    void operator()(ast::FunctionCall& functionCall){
        for(auto& value : functionCall.values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::Switch& switch_){
        visit_each_non_variant(*this, switch_.cases);
        switch_.value = visit(transformer, switch_.value);
        visit_optional_non_variant(*this, switch_.default_case);
    }

    void operator()(ast::SwitchCase& switch_case){
        switch_case.value = visit(transformer, switch_case.value);
        visit_each(*this, switch_case.instructions);
    }

    void operator()(ast::DefaultCase& default_case){
        visit_each(*this, default_case.instructions);
    }

    void operator()(ast::VariableDeclaration& declaration){
        if(declaration.value){
            declaration.value = visit(transformer, *declaration.value);
        }
    }

    void operator()(ast::Assignment& assignment){
        assignment.left_value = visit(transformer, assignment.left_value);
        assignment.value = visit(transformer, assignment.value);
    }

    void operator()(ast::StructDeclaration& declaration){
        for(auto& value : declaration.values){
            value = visit(transformer, value);
        }
    }

    void operator()(ast::PrefixOperation& operation){
        operation.left_value = visit(transformer, operation.left_value);
    }

    void operator()(ast::Return& return_){
        return_.value = visit(transformer, return_.value);
    }

    void operator()(ast::ArrayDeclaration& declaration){
        declaration.size = visit(transformer, declaration.size);
    }

    void operator()(ast::Expression& expression){
        visit_non_variant(transformer, expression);
    }
};

} //end of anonymous namespace

void ast::CleanPass::apply_program(ast::SourceFile& program, bool){
    CleanerVisitor visitor;
    visitor(program);
}

bool ast::CleanPass::is_simple(){
    return true;
}

bool ast::TransformPass::is_simple(){
    return true;
}

void ast::TransformPass::apply_program(ast::SourceFile& program, bool){
    TransformerVisitor visitor;
    visitor(program);
}
