//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"
#include "variant.hpp"
#include "assert.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"
#include "iterators.hpp"
#include "Type.hpp"
#include "SemanticalException.hpp"
#include "FunctionContext.hpp"

#include "ast/TemplateEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

using namespace eddic;

namespace {

struct ValueCopier : public boost::static_visitor<ast::Value> {
    ast::Value operator()(const ast::Integer& source) const {
        ast::Integer copy;
        
        copy.value = source.value;
        
        return copy;
    }
    
    ast::Value operator()(const ast::IntegerSuffix& source) const {
        ast::IntegerSuffix copy;
        
        copy.value = source.value;
        copy.suffix = source.suffix;
        
        return copy;
    }
    
    ast::Value operator()(const ast::Float& source) const {
        ast::Float copy;
        
        copy.value = source.value;
        
        return copy;
    }
    
    ast::Value operator()(const ast::Literal& source) const {
        ast::Literal copy;
        
        copy.value = source.value;
        copy.label = source.label;
        
        return copy;
    }
    
    ast::Value operator()(const ast::CharLiteral& source) const {
        ast::CharLiteral copy;
        
        copy.value = source.value;
        
        return copy;
    }
    
    ast::Value operator()(const ast::VariableValue& source) const {
        ast::VariableValue copy;
        
        copy.Content->context = source.Content->context;
        copy.Content->var = source.Content->var;
        copy.Content->position = source.Content->position;
        copy.Content->variableName = source.Content->variableName;
        
        return copy;
    }
    
    ast::Value operator()(const ast::DereferenceValue& source) const {
        ast::DereferenceValue copy;
        
        copy.Content->position = source.Content->position;
        
        auto ref = visit(*this, source.Content->ref);
        
        if(auto* ptr = boost::get<ast::VariableValue>(&ref)){
            copy.Content->ref = *ptr;
        } else if(auto* ptr = boost::get<ast::MemberValue>(&ref)){
            copy.Content->ref = *ptr;
        } else if(auto* ptr = boost::get<ast::ArrayValue>(&ref)){
            copy.Content->ref = *ptr;
        } else {
            ASSERT_PATH_NOT_TAKEN("Invalid ref type");
        }
        
        return copy;
    }
    
    ast::Value operator()(const ast::Expression& source) const {
        ast::Expression copy;

        copy.Content->position = source.Content->position;
        copy.Content->first = visit(*this, source.Content->first);

        for(auto& operation : source.Content->operations){
            copy.Content->operations.push_back(boost::make_tuple(operation.get<0>(), visit(*this, operation.get<1>())));
        }

        return copy;
    }
    
    ast::Value operator()(const ast::Unary& source) const {
        ast::Unary copy;

        copy.Content->op = source.Content->op;
        copy.Content->value = visit(*this, source.Content->value);

        return copy;
    }
    
    ast::Value operator()(const ast::Null& source) const {
        return source;
    }
    
    ast::Value operator()(const ast::True& source) const {
        return source;
    }
    
    ast::Value operator()(const ast::False& source) const {
        return source;
    }
    
    ast::Value operator()(const ast::ArrayValue& source) const {
        ast::ArrayValue copy;

        copy.Content->context = source.Content->context;
        copy.Content->var = source.Content->var;
        copy.Content->position = source.Content->position;
        copy.Content->arrayName = source.Content->arrayName;
        copy.Content->indexValue = visit(*this, source.Content->indexValue);

        return copy;
    }
    
    ast::Value operator()(const ast::FunctionCall& source) const {
        ast::FunctionCall copy;

        copy.Content->function = source.Content->function;
        copy.Content->mangled_name = source.Content->mangled_name;
        copy.Content->position = source.Content->position;
        copy.Content->function_name = source.Content->function_name;
        copy.Content->template_types = source.Content->template_types;

        for(auto& value : source.Content->values){
            copy.Content->values.push_back(visit(*this, value));
        }

        return copy;
    }
    
    ast::Value operator()(const ast::MemberFunctionCall& source) const {
        ast::MemberFunctionCall copy;

        copy.Content->function = source.Content->function;
        copy.Content->context = source.Content->context;
        copy.Content->mangled_name = source.Content->mangled_name;
        copy.Content->position = source.Content->position;
        copy.Content->object_name = source.Content->object_name;
        copy.Content->function_name = source.Content->function_name;
        copy.Content->template_types = source.Content->template_types;

        for(auto& value : source.Content->values){
            copy.Content->values.push_back(visit(*this, value));
        }

        return copy;
    }
    
    ast::Value operator()(const ast::Cast& source) const {
        ast::Cast copy;

        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->type = source.Content->type;
        copy.Content->value = visit(*this, source.Content->value);

        return copy;
    }
    
    ast::Value operator()(const ast::BuiltinOperator& source) const {
        ast::BuiltinOperator copy;

        copy.Content->position = source.Content->position;
        copy.Content->type = source.Content->type;
        
        for(auto& value : source.Content->values){
            copy.Content->values.push_back(visit(*this, value));
        }

        return copy;
    }
    
    ast::Value operator()(const ast::Assignment& source) const {
        ast::Assignment copy;

        copy.Content->position = source.Content->position;
        copy.Content->op = source.Content->op;
        copy.Content->value = visit(*this, source.Content->value);
        copy.Content->left_value = ast::to_left_value(visit(*this, source.Content->left_value));

        return copy;
    }
    
    ast::Value operator()(const ast::SuffixOperation& source) const {
        ast::SuffixOperation copy;

        copy.Content->position = source.Content->position;
        copy.Content->op = source.Content->op;
        copy.Content->left_value = ast::to_left_value(visit(*this, source.Content->left_value));

        return copy;
    }
    
    ast::Value operator()(const ast::PrefixOperation& source) const {
        ast::PrefixOperation copy;

        copy.Content->position = source.Content->position;
        copy.Content->op = source.Content->op;
        copy.Content->left_value = ast::to_left_value(visit(*this, source.Content->left_value));

        return copy;
    }
    
    ast::Value operator()(const ast::Ternary& source) const {
        ast::Ternary copy;

        copy.Content->position = source.Content->position;
        copy.Content->condition = visit(*this, source.Content->condition);
        copy.Content->false_value = visit(*this, source.Content->false_value);
        copy.Content->true_value = visit(*this, source.Content->true_value);

        return copy;
    }
    
    ast::Value operator()(const ast::MemberValue& source) const {
        ast::MemberValue copy;

        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->memberNames = source.Content->memberNames;
        
        auto location = visit(*this, source.Content->location);
        
        if(auto* ptr = boost::get<ast::VariableValue>(&location)){
            copy.Content->location = *ptr;
        } else if(auto* ptr = boost::get<ast::ArrayValue>(&location)){
            copy.Content->location = *ptr;
        } else {
            ASSERT_PATH_NOT_TAKEN("Invalid ref type");
        }

        return copy;
    }
    
    ast::Value operator()(const ast::New& source) const {
        ast::New copy;

        copy.Content->context = source.Content->context;
        copy.Content->mangled_name = source.Content->mangled_name;
        copy.Content->position = source.Content->position;
        copy.Content->type = source.Content->type;
        
        for(auto& value : source.Content->values){
            copy.Content->values.push_back(visit(*this, value));
        }
        
        return copy;
    }
};

struct InstructionCopier : public boost::static_visitor<ast::Instruction> {
    ast::Instruction operator()(const ast::MemberFunctionCall& source) const {
        ast::MemberFunctionCall copy;

        copy.Content->function = source.Content->function;
        copy.Content->context = source.Content->context;
        copy.Content->mangled_name = source.Content->mangled_name;
        copy.Content->position = source.Content->position;
        copy.Content->object_name = source.Content->object_name;
        copy.Content->function_name = source.Content->function_name;
        copy.Content->template_types = source.Content->template_types;
        
        for(auto& value : source.Content->values){
            copy.Content->values.push_back(visit(ValueCopier(), value));
        }
        
        return copy;
    }
    
    ast::Instruction operator()(const ast::FunctionCall& source) const {
        ast::FunctionCall copy;

        copy.Content->function = source.Content->function;
        copy.Content->mangled_name = source.Content->mangled_name;
        copy.Content->position = source.Content->position;
        copy.Content->template_types = source.Content->template_types;
        copy.Content->function_name = source.Content->function_name;
        
        for(auto& value : source.Content->values){
            copy.Content->values.push_back(visit(ValueCopier(), value));
        }
        
        return copy;
    }
    
    ast::Instruction operator()(const ast::Swap& source) const {
        ast::Swap copy;
        
        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->lhs = source.Content->lhs;
        copy.Content->rhs = source.Content->rhs;
        copy.Content->lhs_var = source.Content->lhs_var;
        copy.Content->rhs_var = source.Content->rhs_var;

        return copy;
    }
    
    ast::Instruction operator()(const ast::VariableDeclaration& source) const {
        ast::VariableDeclaration copy;

        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->variableType = source.Content->variableType;
        copy.Content->variableName = source.Content->variableName;
        
        if(source.Content->value){
            copy.Content->value = visit(ValueCopier(), *source.Content->value);
        }
        
        return copy;
    }
    
    ast::Instruction operator()(const ast::StructDeclaration& source) const {
        ast::StructDeclaration copy;

        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->variableType = source.Content->variableType;
        copy.Content->variableName = source.Content->variableName;
        
        for(auto& value : source.Content->values){
            copy.Content->values.push_back(visit(ValueCopier(), value));
        }
        
        return copy;
    }
    
    ast::Instruction operator()(const ast::ArrayDeclaration& source) const {
        ast::ArrayDeclaration copy;

        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->arrayType = source.Content->arrayType;
        copy.Content->arrayName = source.Content->arrayName;
        copy.Content->size = source.Content->size;
        
        return copy;
    }
    
    ast::Instruction operator()(const ast::Assignment& source) const {
        ast::Assignment copy;

        copy.Content->position = source.Content->position;
        copy.Content->left_value = ast::to_left_value(visit(ValueCopier(), source.Content->left_value));
        copy.Content->value = visit(ValueCopier(), source.Content->value);
        copy.Content->op = source.Content->op;
        
        return copy;
    }
    
    ast::Instruction operator()(const ast::Return& source) const {
        ast::Return copy;

        copy.Content->function = source.Content->function;
        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->value = visit(ValueCopier(), source.Content->value);
        
        return copy;
    }

    ast::Instruction operator()(const ast::If& source) const {
        ast::If copy;

        copy.Content->context = source.Content->context;
        copy.Content->condition = visit(ValueCopier(), source.Content->condition);
        
        for(auto& instruction : source.Content->instructions){
            copy.Content->instructions.push_back(visit(*this, instruction));
        }

        for(auto& else_if : source.Content->elseIfs){
            ast::ElseIf else_if_copy;

            else_if_copy.context = else_if.context;
            else_if_copy.condition = visit(ValueCopier(), else_if.condition);
        
            for(auto& instruction : else_if.instructions){
                else_if_copy.instructions.push_back(visit(*this, instruction));
            }

            copy.Content->elseIfs.push_back(else_if_copy);
        }

        if(source.Content->else_){
            ast::Else else_copy;
            
            else_copy.context = source.Content->else_->context;
            
            for(auto& instruction : (*source.Content->else_).instructions){
                else_copy.instructions.push_back(visit(*this, instruction));
            }

            copy.Content->else_ = else_copy;
        }

        return copy;
    }
    
    ast::Instruction operator()(const ast::While& source) const {
        ast::While copy;

        copy.Content->context = source.Content->context;
        copy.Content->condition = visit(ValueCopier(), source.Content->condition);
        
        for(auto& instruction : source.Content->instructions){
            copy.Content->instructions.push_back(visit(*this, instruction));
        }

        return copy;
    }
    
    ast::Instruction operator()(const ast::DoWhile& source) const {
        ast::DoWhile copy;

        copy.Content->context = source.Content->context;
        copy.Content->condition = visit(ValueCopier(), source.Content->condition);
        
        for(auto& instruction : source.Content->instructions){
            copy.Content->instructions.push_back(visit(*this, instruction));
        }

        return copy;
    }
    
    ast::Instruction operator()(const ast::Foreach& source) const {
        ast::Foreach copy;

        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->variableType = source.Content->variableType;
        copy.Content->variableName = source.Content->variableName;
        copy.Content->from = source.Content->from;
        copy.Content->to = source.Content->to;
        
        for(auto& instruction : source.Content->instructions){
            copy.Content->instructions.push_back(visit(*this, instruction));
        }

        return copy;
    }
    
    ast::Instruction operator()(const ast::ForeachIn& source) const {
        ast::ForeachIn copy;

        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->variableType = source.Content->variableType;
        copy.Content->variableName = source.Content->variableName;
        copy.Content->arrayName = source.Content->arrayName;
        copy.Content->var = source.Content->var;
        copy.Content->arrayVar = source.Content->arrayVar;
        copy.Content->iterVar = source.Content->iterVar;
        
        for(auto& instruction : source.Content->instructions){
            copy.Content->instructions.push_back(visit(*this, instruction));
        }

        return copy;
    }
    
    ast::Instruction operator()(const ast::SuffixOperation& source) const {
        ast::SuffixOperation copy;

        copy.Content->position = source.Content->position;
        copy.Content->left_value = ast::to_left_value(visit(ValueCopier(), source.Content->left_value));
        copy.Content->op = source.Content->op;

        return copy;
    }
    
    ast::Instruction operator()(const ast::PrefixOperation& source) const {
        ast::PrefixOperation copy;

        copy.Content->position = source.Content->position;
        copy.Content->left_value = ast::to_left_value(visit(ValueCopier(), source.Content->left_value));
        copy.Content->op = source.Content->op;

        return copy;
    }
    
    ast::Instruction operator()(const ast::For& source) const {
        ast::For copy;
        
        copy.Content->context = source.Content->context;

        if(source.Content->start){
            copy.Content->start = visit(*this, *source.Content->start);
        }

        if(source.Content->condition){
            copy.Content->condition = visit(ValueCopier(), *source.Content->condition);
        }

        if(source.Content->repeat){
            copy.Content->repeat = visit(*this, *source.Content->repeat);
        }
        
        for(auto& instruction : source.Content->instructions){
            copy.Content->instructions.push_back(visit(*this, instruction));
        }

        return copy;
    }
    
    ast::Instruction operator()(const ast::Switch& source) const {
        ast::Switch copy;

        copy.Content->context = source.Content->context;
        copy.Content->position = source.Content->position;
        copy.Content->value = visit(ValueCopier(), source.Content->value);

        for(auto& switch_case : source.Content->cases){
            ast::SwitchCase case_;

            case_.value = visit(ValueCopier(), switch_case.value);
            case_.context = switch_case.context;

            for(auto& instruction : switch_case.instructions){
                case_.instructions.push_back(visit(*this, instruction));
            }
    
            copy.Content->cases.push_back(case_);
        }

        if(source.Content->default_case){
            ast::DefaultCase default_;
            
            default_.context = source.Content->default_case->context;

            for(auto& instruction : source.Content->default_case->instructions){
                default_.instructions.push_back(visit(*this, instruction));
            }

            copy.Content->default_case = default_;
        }

        return copy;
    }
    
    ast::Instruction operator()(const ast::Delete& source) const {
        ast::Delete copy;

        copy.Content->context = source.Content->context;
        copy.Content->variable = source.Content->variable;
        copy.Content->position = source.Content->position;
        copy.Content->variable_name = source.Content->variable_name;

        return copy;
    }
};

struct Adaptor : public boost::static_visitor<> {
    const std::unordered_map<std::string, ast::Type>& replacements;

    Adaptor(const std::unordered_map<std::string, ast::Type>& replacements) : replacements(replacements) {}

    AUTO_RECURSE_DESTRUCTOR()
    AUTO_RECURSE_RETURN_VALUES()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_PREFIX()
    AUTO_RECURSE_SUFFIX()
    AUTO_RECURSE_SWITCH()
    
    AUTO_IGNORE_SWAP()

    void operator()(ast::Struct& struct_){
        visit_each_non_variant(*this, struct_.Content->members);
        visit_each_non_variant(*this, struct_.Content->constructors);
        visit_each_non_variant(*this, struct_.Content->destructors);
        visit_each_non_variant(*this, struct_.Content->functions);
        visit_each_non_variant(*this, struct_.Content->template_functions);
    }

    bool has_to_be_replaced(const std::string& type){
        return replacements.find(type) != replacements.end();
    }

    ast::Type replace(ast::Type& type){
        if(auto* ptr = boost::get<ast::SimpleType>(&type)){
            if(has_to_be_replaced(ptr->type)){
                return replacements.at(ptr->type);
            } else {
                return *ptr;
            }
        } else if(auto* ptr = boost::get<ast::ArrayType>(&type)){
            ptr->type = replace(ptr->type.get());

            return *ptr;
        } else if(auto* ptr = boost::get<ast::PointerType>(&type)){
            ptr->type = replace(ptr->type.get());

            return *ptr;
        } else if(auto* ptr = boost::get<ast::TemplateType>(&type)){
            if(has_to_be_replaced(ptr->type)){
                auto replacement = replacements.at(ptr->type);

                auto simple = boost::get<ast::SimpleType>(replacement);
                ptr->type = simple.type;
            }

            for(std::size_t i = 0; i < ptr->template_types.size(); ++i){
                ptr->template_types[i] = replace(ptr->template_types[i]);
            }

            return *ptr;
        } else {
            ASSERT_PATH_NOT_TAKEN("Unhandled type");
        }
    }
    
    void operator()(ast::MemberDeclaration& declaration){
        declaration.Content->type = replace(declaration.Content->type); 
    }
    
    void operator()(ast::Constructor& declaration){
        for(auto& param : declaration.Content->parameters){
            param.parameterType = replace(param.parameterType);
        }

        visit_each(*this, declaration.Content->instructions);
    }

    void operator()(ast::FunctionDeclaration& declaration){
        declaration.Content->returnType = replace(declaration.Content->returnType);

        for(auto& param : declaration.Content->parameters){
            param.parameterType = replace(param.parameterType);
        }

        visit_each(*this, declaration.Content->instructions);
    }

    void operator()(ast::Assignment& assignment){
        visit(*this, assignment.Content->left_value);
        visit(*this, assignment.Content->value);
    }

    template<typename FunctionCall>
    void adapt_function_call(FunctionCall& source){
        for(std::size_t i = 0; i < source.Content->template_types.size(); ++i){
            source.Content->template_types[i] = replace(source.Content->template_types[i]);
        }

        visit_each(*this, source.Content->values);
    }

    void operator()(ast::MemberFunctionCall& source){
        adapt_function_call(source);
    }
    
    void operator()(ast::FunctionCall& source){
        adapt_function_call(source);
    }
    
    void operator()(ast::VariableDeclaration& source){
        source.Content->variableType = replace(source.Content->variableType);
        
        visit_optional(*this, source.Content->value);
    }
    
    void operator()(ast::StructDeclaration& source){
        source.Content->variableType = replace(source.Content->variableType);
        
        visit_each(*this, source.Content->values);
    }
    
    void operator()(ast::ArrayDeclaration& source){
        source.Content->arrayType = replace(source.Content->arrayType);
    }
    
    void operator()(ast::Foreach& source){
        source.Content->variableType = replace(source.Content->variableType);

        visit_each(*this, source.Content->instructions);
    }
    
    void operator()(ast::ForeachIn& source){
        source.Content->variableType = replace(source.Content->variableType);

        visit_each(*this, source.Content->instructions);
    }
    
    void operator()(ast::New& source){
        source.Content->type = replace(source.Content->type);
        
        visit_each(*this, source.Content->values);
    }

    AUTO_IGNORE_OTHERS()
};

struct Instantiator : public boost::static_visitor<> {
    ast::TemplateEngine::FunctionTemplateMap& function_templates;
    ast::TemplateEngine::FunctionInstantiationMap& function_template_instantiations;
    
    ast::TemplateEngine::ClassTemplateMap& class_templates;
    ast::TemplateEngine::ClassInstantiationMap& class_template_instantiations;
    
    std::unordered_map<std::string, std::vector<ast::FunctionDeclaration>> function_template_instantiated;
    std::vector<ast::Struct> class_template_instantiated;

    Instantiator(ast::TemplateEngine& engine) : 
        function_templates(engine.function_templates), function_template_instantiations(engine.function_template_instantiations),
        class_templates(engine.class_templates), class_template_instantiations(engine.class_template_instantiations) {}

    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_GLOBAL_DECLARATION() 
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
    AUTO_RECURSE_BUILTIN_OPERATORS()
    AUTO_RECURSE_COMPOSED_VALUES()
    AUTO_RECURSE_MEMBER_VALUE()
    AUTO_RECURSE_UNARY_VALUES()
    AUTO_RECURSE_STRUCT()
    AUTO_RECURSE_CONSTRUCTOR()
    AUTO_RECURSE_DESTRUCTOR()
    AUTO_RECURSE_SWITCH()
    AUTO_RECURSE_SWITCH_CASE()
    AUTO_RECURSE_DEFAULT_CASE()
    AUTO_RECURSE_RETURN_VALUES()

    template<typename Container>
    bool is_instantiated(const Container& container, const std::string& name, const std::vector<ast::Type>& template_types){
        auto it_pair = container.equal_range(name);

        if(it_pair.first == it_pair.second && it_pair.second == container.end()){
            return false;
        }

        do {
            auto types = it_pair.first->second;
           
            if(are_equals(types, template_types)){
                return true;
            }

            ++it_pair.first;
        } while(it_pair.first != it_pair.second);

        return false;
    }

    bool is_instantiated(const std::string& name, const std::string& context, const std::vector<ast::Type>& template_types){
        return is_instantiated(function_template_instantiations[context], name, template_types);
    }

    bool is_class_instantiated(const std::string& name, const std::vector<ast::Type>& template_types){
        return is_instantiated(class_template_instantiations, name, template_types);
    }
    
    std::vector<ast::Constructor> copy(const std::vector<ast::Constructor>& source){
        std::vector<ast::Constructor> destination;
        destination.reserve(source.size());
    
        for(auto& constructor : source){
            ast::Constructor c;
            c.Content->context = constructor.Content->context;
            c.Content->struct_name = constructor.Content->struct_name;
            c.Content->position = constructor.Content->position;
            c.Content->parameters = constructor.Content->parameters;
            c.Content->instructions = copy(constructor.Content->instructions);

            destination.push_back(c);
        }

        return destination;
    }
    
    std::vector<ast::Destructor> copy(const std::vector<ast::Destructor>& source){
        std::vector<ast::Destructor> destination;
        destination.reserve(source.size());
    
        for(auto& destructor : source){
            ast::Destructor d;
            d.Content->context = destructor.Content->context;
            d.Content->struct_name = destructor.Content->struct_name;
            d.Content->position = destructor.Content->position;
            d.Content->parameters = destructor.Content->parameters;
            d.Content->instructions = copy(destructor.Content->instructions);

            destination.push_back(d);
        }

        return destination;
    }
    
    std::vector<ast::FunctionDeclaration> copy(const std::vector<ast::FunctionDeclaration>& source){
        std::vector<ast::FunctionDeclaration> destination;
        destination.reserve(source.size());
    
        for(auto& function : source){
            ast::FunctionDeclaration f;
            f.Content->context = function.Content->context;
            f.Content->position = function.Content->position;
            f.Content->struct_name = function.Content->struct_name;
            f.Content->returnType = function.Content->returnType;
            f.Content->functionName = function.Content->functionName;
            f.Content->instructions = copy(function.Content->instructions);
            f.Content->parameters = function.Content->parameters;

            destination.push_back(f);
        }

        return destination;
    }
    
    std::vector<ast::TemplateFunctionDeclaration> copy(const std::vector<ast::TemplateFunctionDeclaration>& source){
        std::vector<ast::TemplateFunctionDeclaration> destination;
        destination.reserve(source.size());
    
        for(auto& function : source){
            ast::TemplateFunctionDeclaration f;
            f.Content->context = function.Content->context;
            f.Content->position = function.Content->position;
            f.Content->struct_name = function.Content->struct_name;
            f.Content->template_types = function.Content->template_types;
            f.Content->returnType = function.Content->returnType;
            f.Content->functionName = function.Content->functionName;
            f.Content->instructions = copy(function.Content->instructions);
            f.Content->parameters = function.Content->parameters;

            destination.push_back(f);
        }

        return destination;
    }
    
    std::vector<ast::MemberDeclaration> copy(const std::vector<ast::MemberDeclaration>& source){
        std::vector<ast::MemberDeclaration> destination;   
        destination.reserve(source.size());
    
        for(auto& member_declaration : source){
            ast::MemberDeclaration member;
            member.Content->position = member_declaration.Content->position;
            member.Content->type = member_declaration.Content->type;
            member.Content->name = member_declaration.Content->name;

            destination.push_back(member);
        }

        return destination;
    }

    void check_type(ast::Type& type, ast::Position& position){
        if(auto* ptr = boost::get<ast::TemplateType>(&type)){
            auto template_types = ptr->template_types;
            auto name = ptr->type;

            auto it = class_templates.find(name);

            if(it == class_templates.end()){
                throw SemanticalException("There are no class template named " + name, position);
            }
            
            while(it != class_templates.end()){
                auto struct_declaration = it->second;
                auto source_types = struct_declaration.Content->template_types;

                if(source_types.size() == template_types.size()){
                    if(!is_class_instantiated(name, template_types)){
                        log::emit<Info>("Template") << "Instantiate class template " << name << log::endl;

                        //Instantiate the struct
                        ast::Struct declaration;
                        declaration.Content->name = struct_declaration.Content->name;
                        declaration.Content->position = struct_declaration.Content->position;
                        declaration.Content->template_types = template_types; 
                        
                        declaration.Content->members = copy(struct_declaration.Content->members);
                        declaration.Content->constructors = copy(struct_declaration.Content->constructors);
                        declaration.Content->destructors = copy(struct_declaration.Content->destructors);
                        declaration.Content->functions = copy(struct_declaration.Content->functions);
                        declaration.Content->template_functions = copy(struct_declaration.Content->template_functions);

                        //For handling later
                        declaration.Content->instantiated = true;
                        declaration.Content->marked = false;

                        std::unordered_map<std::string, ast::Type> replacements;

                        for(std::size_t i = 0; i < template_types.size(); ++i){
                            replacements[source_types[i]] = template_types[i];    
                        }

                        Adaptor adaptor(replacements);
                        visit_non_variant(adaptor, declaration);

                        //Mark it as instantiated
                        class_template_instantiations.insert(ast::TemplateEngine::ClassInstantiationMap::value_type(name, template_types));

                        class_template_instantiated.push_back(declaration);
                    }
                    
                    ptr->resolved = true;

                    return;
                }
            }
        }
    }

    void operator()(const ast::FunctionDeclaration& function){
        for(auto& param_type : function.Content->parameters){
            check_type(param_type.parameterType, function.Content->position);
        }

        visit_each(*this, function.Content->instructions);
    }

    void operator()(const ast::Assignment& assignment){
        visit(*this, assignment.Content->left_value);
        visit(*this, assignment.Content->value);
    }

    void operator()(const ast::VariableDeclaration& declaration){
        check_type(declaration.Content->variableType, declaration.Content->position);

        visit_optional(*this, declaration.Content->value);
    }

    void operator()(const ast::StructDeclaration& declaration){
        check_type(declaration.Content->variableType, declaration.Content->position);

        visit_each(*this, declaration.Content->values);
    }

    void operator()(ast::Foreach& foreach_){
        check_type(foreach_.Content->variableType, foreach_.Content->position);

        visit_each(*this, foreach_.Content->instructions);
    }

    void operator()(ast::ForeachIn& foreach_){
        check_type(foreach_.Content->variableType, foreach_.Content->position);

        visit_each(*this, foreach_.Content->instructions);
    }
    
    std::vector<ast::Instruction> copy(const std::vector<ast::Instruction>& source){
        std::vector<ast::Instruction> destination;   
        destination.reserve(source.size());
    
        InstructionCopier copier;
    
        for(auto& instruction : source){
            destination.push_back(visit(copier, instruction));
        }

        return destination;
    }

    template<typename FunctionCall>
    void handle_template(FunctionCall& functionCall, const std::string context){
        auto template_types = functionCall.Content->template_types;
        auto name = functionCall.Content->function_name;

        auto it_pair = function_templates[context].equal_range(name);

        if(it_pair.first == it_pair.second && it_pair.second == function_templates[context].end()){
            throw SemanticalException("There are no template function named " + name, functionCall.Content->position);
        }

        do{
            auto function_declaration = it_pair.first->second;
            auto source_types = function_declaration.Content->template_types;

            if(source_types.size() == template_types.size()){
                if(!is_instantiated(name, context, template_types)){
                    log::emit<Info>("Template") << "Instantiate function template " << name << log::endl;

                    //Instantiate the function 
                    ast::FunctionDeclaration declaration;
                    declaration.Content->position = function_declaration.Content->position;
                    declaration.Content->returnType = function_declaration.Content->returnType;
                    declaration.Content->functionName = function_declaration.Content->functionName;
                    declaration.Content->parameters = function_declaration.Content->parameters;
                    declaration.Content->instructions = copy(function_declaration.Content->instructions);

                    //For handling later
                    declaration.Content->instantiated = true;
                    declaration.Content->marked = false;

                    std::unordered_map<std::string, ast::Type> replacements;

                    for(std::size_t i = 0; i < template_types.size(); ++i){
                        replacements[source_types[i]] = template_types[i];    
                    }

                    Adaptor adaptor(replacements);
                    visit_non_variant(adaptor, declaration);

                    //Mark it as instantiated
                    function_template_instantiations[context].insert(ast::TemplateEngine::LocalFunctionInstantiationMap::value_type(name, template_types));

                    function_template_instantiated[context].push_back(declaration);
                } 

                functionCall.Content->resolved = true;

                return;
            }

            ++it_pair.first;
        } while(it_pair.first != it_pair.second);

        throw SemanticalException("No matching function " + name, functionCall.Content->position);
    }

    void operator()(ast::FunctionCall& functionCall){
        visit_each(*this, functionCall.Content->values);
        
        auto template_types = functionCall.Content->template_types;

        if(!template_types.empty()){
            handle_template(functionCall, "");
        }
    }

    void operator()(ast::MemberFunctionCall& functionCall){
        visit_each(*this, functionCall.Content->values);

        if(!functionCall.Content->template_types.empty()){
            auto object_name = functionCall.Content->object_name;

            if(functionCall.Content->context->exists(object_name)){
                auto object_var = functionCall.Content->context->getVariable(object_name);
                auto object_type = object_var->type()->is_pointer() ? object_var->type()->data_type() : object_var->type();
                auto context = object_type->type();

                handle_template(functionCall, context);            
            }
        }
    }

    template<typename T> 
    void operator()(const T&){
        //Nothing to check
    }
};

struct Collector : public boost::static_visitor<> {
    ast::TemplateEngine::FunctionTemplateMap& function_templates;
    ast::TemplateEngine::ClassTemplateMap& class_templates;

    std::string parent_struct;

    Collector(ast::TemplateEngine::FunctionTemplateMap& function_templates, ast::TemplateEngine::ClassTemplateMap& class_templates) : 
            function_templates(function_templates), class_templates(class_templates) {}

    AUTO_RECURSE_PROGRAM()

    void operator()(ast::TemplateFunctionDeclaration& declaration){
        function_templates[parent_struct].insert(ast::TemplateEngine::LocalFunctionTemplateMap::value_type(declaration.Content->functionName, declaration));
    }
    
    void operator()(ast::TemplateStruct& template_struct){
        class_templates.insert(ast::TemplateEngine::ClassTemplateMap::value_type(template_struct.Content->name, template_struct)); 
    }
        
    void operator()(ast::Struct& struct_){
        parent_struct = struct_.Content->name;

        visit_each_non_variant(*this, struct_.Content->template_functions);

        parent_struct = "";
    }

    AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace

void ast::TemplateEngine::template_instantiation(ast::SourceFile& program){
    Collector collector(function_templates, class_templates);
    collector(program);

    Instantiator instantiator(*this);
    instantiator(program);

    for(auto& struct_ : instantiator.class_template_instantiated){
        program.Content->blocks.push_back(struct_);
    }

    for(auto& context_pair : instantiator.function_template_instantiated){
        auto context = context_pair.first;
        auto instantiated_functions = context_pair.second;

        if(context.empty()){
            for(auto& function : instantiated_functions){
                program.Content->blocks.push_back(function);
            }
        } else {
            for(auto& block : program.Content->blocks){
                if(auto* struct_type = boost::get<ast::Struct>(&block)){
                    if(struct_type->Content->name == context){
                        for(auto& function : instantiated_functions){
                            struct_type->Content->functions.push_back(function);
                        }

                        break;
                    }
                }
            }
        }
    }
}
