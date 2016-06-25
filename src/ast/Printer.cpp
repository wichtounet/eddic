//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "cpp_utils/assert.hpp"

#include "variant.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"

#include "ast/Printer.hpp"
#include "ast/SourceFile.hpp"
#include "ast/Operator.hpp"

using namespace eddic;

namespace {

struct DebugVisitor : public boost::static_visitor<> {
    mutable int level = 0;

    std::string indent() const {
        std::string acc = "";
        for(int i = 0; i < level; ++i){
            acc += "    ";
        }
        return acc;
    }

    template<typename Container>
    void print_each_sub(Container& container) const {
        level++;
        visit_each(*this, container);
        level--;
    }

    template<typename Container>
    void print_each_sub(Container& container, const std::string& title) const {
        std::cout << indent() << title << std::endl;
        print_each_sub(container);
    }

    template<typename Container>
    void print_each_sub_non_variant(Container& container) const {
        level++;
        visit_each_non_variant(*this, container);
        level--;
    }

    template<typename Container>
    void print_each_sub_non_variant(Container& container, const std::string& title) const {
        std::cout << indent() << title << std::endl;
        print_each_sub_non_variant(container);
    }

    template<typename Container>
    void print_sub(Container& container) const {
        level++;
        visit(*this, container);
        level--;
    }

    void print_template_list(const std::vector<ast::Type>& template_types) const {
        if(!template_types.empty()){
            std::cout << "<";

            std::cout << to_string(template_types[0]);

            for(std::size_t i = 1; i < template_types.size(); ++i){
                std::cout << ", " << to_string(template_types[i]);
            }

            std::cout << ">";
        }
    }

    void print_template_list(const std::vector<std::string>& template_types) const {
        if(!template_types.empty()){
            std::cout << "<";

            std::cout << template_types[0];

            for(std::size_t i = 1; i < template_types.size(); ++i){
                std::cout << ", " << template_types[i];
            }

            std::cout << ">";
        }
    }

    template<typename Container>
    void print_sub(Container& container, const std::string& title) const {
        std::cout << indent() << title << std::endl;
        print_sub(container);
    }

    void operator()(ast::SourceFile& program) const {
        print_each_sub(program.blocks, "SourceFile");
    }

    void operator()(ast::Import& import) const {
        std::cout << indent() << "include \"" << import.file << "\"" << std::endl;
    }

    void operator()(ast::StandardImport& import) const {
        std::cout << indent() << "include <" << import.header << ">" << std::endl;
    }

    void operator()(ast::struct_definition& declaration) const {
        std::cout << indent() << "Struct Definition";
        if(declaration.is_template_declaration()){
            print_template_list(declaration.decl_template_types);
        }
        std::cout << " " << declaration.name << std::endl;
        level++;
        print_each_sub(declaration.blocks, "Blocks");
        level--;
        std::cout << std::endl;
    }

    void operator()(ast::TemplateFunctionDeclaration& declaration) const {
        if(declaration.is_template()){
            std::cout << indent() << "Template Function";
            print_template_list(declaration.template_types);
            std::cout << declaration.functionName << std::endl;
            std::cout << std::endl;
        } else {
            std::cout << indent() << "Function " << declaration.functionName << std::endl;

            std::cout << indent() << "Parameters:" << std::endl;
            level++;
            for(auto& param : declaration.parameters){
                std::cout << indent() << param.parameterName << " : " << to_string(param.parameterType) << std::endl;
            }
            level--;

            print_each_sub(declaration.instructions, "Instructions:");
            std::cout << std::endl;
        }
    }

    void operator()(ast::Constructor& declaration) const {
        std::cout << indent() << "Constructor" << std::endl;

        std::cout << indent() << "Parameters:" << std::endl;
        level++;
        for(auto& param : declaration.parameters){
            std::cout << indent() << param.parameterName << std::endl;
        }
        level--;

        print_each_sub(declaration.instructions, "Instructions:");
    }

    void operator()(ast::Destructor& declaration) const {
        std::cout << indent() << "Destructor" << std::endl;
        print_each_sub(declaration.instructions, "Instructions:");
    }

    void operator()(ast::Scope& declaration) const {
        std::cout << indent() << "Scope" << std::endl;
        print_each_sub(declaration.instructions, "Instructions:");
    }

    void operator()(ast::MemberDeclaration& declaration) const {
        std::cout << indent() << ast::to_string(declaration.type)  << " " << declaration.name << std::endl;
    }

    void operator()(ast::GlobalVariableDeclaration& declaration) const {
        std::cout << indent() << "Global Variable declaration [" << declaration.variableName << "]" << std::endl;

        if(declaration.value){
            print_sub(*declaration.value);
        }
    }

    void operator()(ast::GlobalArrayDeclaration&) const {
        std::cout << indent() << "Global Array" << std::endl;
    }

    void operator()(ast::New& new_) const {
        std::cout << indent() << "New " << ast::to_string(new_.type) << std::endl;
        print_each_sub(new_.values, "Value");
    }

    void operator()(ast::NewArray& new_array) const {
        std::cout << indent() << "New array of " << ast::to_string(new_array.type) << std::endl;
        print_sub(new_array.size, "Size");
    }

    void operator()(ast::Delete& delete_) const {
        std::cout << indent() << "Delete" << std::endl;
        print_sub(delete_.value, "Value");
    }

    void operator()(ast::For& for_) const {
        print_each_sub(for_.instructions, "For");
    }

    void operator()(ast::Foreach& for_) const {
        print_each_sub(for_.instructions, "Foreach");
    }

    void operator()(ast::ForeachIn& for_) const {
        print_each_sub(for_.instructions, "Foreach In");
    }

    void operator()(ast::Switch& switch_) const {
        std::cout << indent() << "Switch " << std::endl;

        ++level;

        print_sub(switch_.value, "Value");

        for(auto& case_ : switch_.cases){
            visit_non_variant(*this, case_);
        }

        if(switch_.default_case){
            visit_non_variant(*this, *switch_.default_case);
        }

        --level;
    }

    void operator()(ast::SwitchCase& switch_case) const {
        std::cout << indent() << "Case" << std::endl;

        ++level;

        print_sub(switch_case.value, "Value");
        print_each_sub(switch_case.instructions, "Instructions");

        --level;
    }

    void operator()(ast::DefaultCase& default_case) const {
        print_each_sub(default_case.instructions, "Default Case");
    }

    void operator()(ast::While& while_) const {
        std::cout << indent() << "While" << std::endl;
        print_sub(while_.condition, "Condition:");
        print_each_sub(while_.instructions, "Instructions:");
    }

    void operator()(ast::DoWhile& while_) const {
        std::cout << indent() << "Do while" << std::endl;
        print_sub(while_.condition, "Condition:");
        print_each_sub(while_.instructions, "Instructions:");
    }

    void operator()(ast::If& if_) const {
        std::cout << indent() << "If" << std::endl;

        print_sub(if_.condition, "Condition");
        print_each_sub(if_.instructions, "Instructions");

        for(auto& else_if : if_.elseIfs){
            std::cout << indent() << "ElseIf" << std::endl;

            print_sub(else_if.condition, "Condition");
            print_each_sub(else_if.instructions, "Instructions");
        }

        if(if_.else_){
            std::cout << indent() << "Else" << std::endl;

            print_each_sub((*if_.else_).instructions, "Instructions");
        }
    }

    void operator()(ast::FunctionCall& call) const {
        std::cout << indent() << "FunctionCall " << call.function_name;
        print_template_list(call.template_types);
        std::cout << std::endl;
        print_each_sub(call.values);
    }

    void operator()(ast::BuiltinOperator& builtin) const {
        std::cout << indent() << "Builtin Operator " << (int) builtin.type << std::endl;
        print_each_sub(builtin.values);
    }

    void operator()(ast::StructDeclaration& declaration) const {
        std::cout << indent() << "Struct declaration [" << declaration.variableName << "]" << std::endl;

        print_each_sub(declaration.values, "Values");
    }

    void operator()(ast::VariableDeclaration& declaration) const {
        std::cout << indent() << "Variable declaration [" << declaration.variableName << "]" << std::endl;

        if(declaration.value){
            print_sub(*declaration.value);
        }
    }

    void operator()(ast::ArrayDeclaration&) const {
        std::cout << indent() << "Array declaration" << std::endl;
    }

    void operator()(ast::PrefixOperation& op) const {
        std::cout << indent() << ast::toString(op.op) << "(prefix)" << std::endl;

        print_sub(op.left_value, "Left Value");
    }

    void operator()(ast::Assignment& assign) const {
        std::cout << indent() << "Assignment [" << ast::toString(assign.op) << "] " << std::endl;

        print_sub(assign.left_value, "Left Value");
        print_sub(assign.value, "Right Value");
    }

    void operator()(ast::Ternary& ternary) const {
        std::cout << indent() << "Ternary" << std::endl;

        print_sub(ternary.condition, "Condition");
        print_sub(ternary.true_value, "True Value");
        print_sub(ternary.false_value, "False Value");
    }

    void operator()(ast::Return& return_) const {
        print_sub(return_.value, "Function Return");
    }

    void operator()(ast::Literal& literal) const {
        std::cout << indent() << "Literal [" << literal.value << "]" << std::endl;
    }

    void operator()(ast::CharLiteral& literal) const {
        std::cout << indent() << "Char Literal [" << literal.value << "]" << std::endl;
    }

    void operator()(ast::Integer& integer) const {
        std::cout << indent() << "Integer [" << integer.value << "]" << std::endl;
    }

    void operator()(ast::IntegerSuffix& integer) const {
        std::cout << indent() << "Float (suffix) [" << integer.value << "f]" << std::endl;
    }

    void operator()(ast::Float& float_) const {
        std::cout << indent() << "Float [" << float_.value << "]" << std::endl;
    }

    void operator()(ast::Boolean& boolean) const {
        std::cout << indent() << boolean.value << std::endl;
    }

    void operator()(ast::Null&) const {
        std::cout << indent() << "null" << std::endl;
    }

    void operator()(ast::VariableValue& value) const {
        std::cout << indent() << "Variable " << value.variableName << std::endl;
    }

    void operator()(ast::Expression& value) const {
        std::cout << indent() << "Expression [" << value.operations.size() << "]" << std::endl;
        ++level;

        visit(*this, value.first);

        for(auto& operation : value.operations){
            std::cout << indent() << ast::toString(operation.get<0>()) << std::endl;

            if(ast::has_operation_value(operation)){
                visit(*this, operation.get<1>());
            }
        }

        --level;
    }

    void operator()(ast::Cast& cast) const {
        std::cout << indent() << "Cast " << std::endl;
        std::cout << indent() << "\tType: " << ast::to_string(cast.type) << std::endl;
        print_sub(cast.value);
    }

    template<typename T>
    void operator()(x3::forward_ast<T>& value) const {
        (*this)(value.get());
    }
};

} //end of anonymous namespace

void ast::Printer::print(SourceFile& program){
    DebugVisitor visitor;
    visit_non_variant(visitor, program);
}

void ast::Printer::print(Expression& program){
    DebugVisitor visitor;
    visit_non_variant(visitor, program);
}

void ast::Printer::print(Value& value){
    DebugVisitor visitor;
    visit(visitor, value);
}

void ast::Printer::print(Instruction& value){
    DebugVisitor visitor;
    visit(visitor, value);
}
