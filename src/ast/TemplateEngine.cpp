//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "variant.hpp"
#include "assert.hpp"
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

struct InstructionAdaptor : public boost::static_visitor<> {
    const std::unordered_map<std::string, std::string>& replacements;

    InstructionAdaptor(const std::unordered_map<std::string, std::string>& replacements) : replacements(replacements) {}

    AUTO_RECURSE_RETURN_VALUES()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_PREFIX()
    AUTO_RECURSE_SUFFIX()
    AUTO_RECURSE_SWITCH()
    
    AUTO_IGNORE_SWAP()

    std::string replace(const std::string& current) const {
        if(replacements.find(current) != replacements.end()){
            return replacements.at(current);
        }

        return current;
    }

    ast::Type replace(const ast::Type& type) const {
        if(auto* ptr = boost::get<ast::SimpleType>(&type)){
            ast::SimpleType t = *ptr;

            t.type = replace(t.type);

            return t;
        } else if(auto* ptr = boost::get<ast::ArrayType>(&type)){
            ast::ArrayType t = *ptr;

            t.type = replace(t.type);

            return t;
        } else if(auto* ptr = boost::get<ast::PointerType>(&type)){
            ast::PointerType t = *ptr;

            t.type = replace(t.type);

            return t;
        } else {
            ASSERT_PATH_NOT_TAKEN("Unhandled type");
        }
    }

    void operator()(const ast::Assignment& assignment) const {
        visit(*this, assignment.Content->left_value);
        visit(*this, assignment.Content->value);
    }

    template<typename FunctionCall>
    void adapt_function_call(const FunctionCall& source) const {
        for(std::size_t i = 0; i < source.Content->template_types.size(); ++i){
            auto a = source.Content->template_types[i];
            source.Content->template_types[i] = replace(source.Content->template_types[i]);
        }

        visit_each(*this, source.Content->values);
    }

    void operator()(const ast::MemberFunctionCall& source) const {
        adapt_function_call(source);
    }
    
    void operator()(const ast::FunctionCall& source) const {
        adapt_function_call(source);
    }
    
    void operator()(const ast::VariableDeclaration& source) const {
        source.Content->variableType = replace(source.Content->variableType);
        
        visit_optional(*this, source.Content->value);
    }
    
    void operator()(const ast::StructDeclaration& source) const {
        source.Content->variableType = replace(source.Content->variableType);
        
        visit_each(*this, source.Content->values);
    }
    
    void operator()(const ast::ArrayDeclaration& source) const {
        source.Content->arrayType = replace(source.Content->arrayType);
    }
    
    void operator()(const ast::Foreach& source) const {
        source.Content->variableType = replace(source.Content->variableType);

        visit_each(*this, source.Content->instructions);
    }
    
    void operator()(const ast::ForeachIn& source) const {
        source.Content->variableType = replace(source.Content->variableType);

        visit_each(*this, source.Content->instructions);
    }
    
    void operator()(const ast::New& source) const {
        source.Content->type = replace(source.Content->type);
        
        visit_each(*this, source.Content->values);
    }

    AUTO_IGNORE_OTHERS_CONST_CONST()
};

struct Instantiator : public boost::static_visitor<> {
    ast::TemplateEngine::FunctionTemplateMap& function_templates;
    ast::TemplateEngine::FunctionInstantiationMap& function_template_instantiations;
    
    std::unordered_map<std::string, std::vector<ast::FunctionDeclaration>> function_template_instantiated;

    std::shared_ptr<FunctionContext> current_context;

    Instantiator(ast::TemplateEngine::FunctionTemplateMap& function_templates, ast::TemplateEngine::FunctionInstantiationMap& function_template_instantiations) : 
        function_templates(function_templates), function_template_instantiations(function_template_instantiations) {}

    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_GLOBAL_DECLARATION() 
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_FOREACH()
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

    void check_type(const ast::Type& type){
        if(auto* ptr = boost::get<ast::TemplateType>(&type)){
            std::cout << "Detected template type usage" << std::endl;


            ptr->resolved = true;
        }
    }

    void operator()(const ast::FunctionDeclaration& function){
        current_context = function.Content->context;

        for(auto& param_type : function.Content->parameters){
            check_type(param_type.parameterType);
        }

        visit_each(*this, function.Content->instructions);
    }

    void operator()(const ast::Assignment& assignment){
        visit(*this, assignment.Content->left_value);
        visit(*this, assignment.Content->value);
    }

    void operator()(const ast::VariableDeclaration& declaration){
        check_type(declaration.Content->variableType);

        visit_optional(*this, declaration.Content->value);
    }

    void operator()(const ast::StructDeclaration& declaration){
        check_type(declaration.Content->variableType);

        visit_each(*this, declaration.Content->values);
    }

    bool are_equals(const std::vector<std::string>& template_types, const std::vector<std::string>& types){
        if(types.size() != template_types.size()){
            return false;
        }

        for(std::size_t i = 0; i < template_types.size(); ++i){
            if(template_types[i] != types[i]){
                return false;
            }
        }

        return true;
    }

    bool is_instantiated(const std::string& name, const std::string& context, const std::vector<std::string>& template_types){
        auto it = function_template_instantiations[context].find(name);

        while(it != function_template_instantiations[context].end()){
            auto types = it->second;
           
            if(are_equals(types, template_types)){
                return true;
            }

            ++it;
        }

        return false;
    }

    std::vector<ast::FunctionParameter> copy(const std::vector<ast::FunctionParameter>& source){
        //All the function parameter can be copied by value
        return source;
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

        std::string name = functionCall.Content->function_name;

        auto it = function_templates[context].find(name);

        if(it == function_templates[context].end()){
            throw SemanticalException("There are no template function named " + name, functionCall.Content->position);
        }

        while(it != function_templates[context].end()){
            auto function_declaration = it->second;
            auto source_types = function_declaration.Content->template_types;

            if(source_types.size() == template_types.size()){
                if(!is_instantiated(name, context, template_types)){
                    //Instantiate the function 
                    ast::FunctionDeclaration declaration;
                    declaration.Content->position = function_declaration.Content->position;
                    declaration.Content->returnType = function_declaration.Content->returnType;
                    declaration.Content->functionName = function_declaration.Content->functionName;
                    declaration.Content->parameters = copy(function_declaration.Content->parameters);
                    declaration.Content->instructions = copy(function_declaration.Content->instructions);

                    //For handling later
                    declaration.Content->instantiated = true;
                    declaration.Content->marked = false;

                    std::unordered_map<std::string, std::string> replacements;

                    for(std::size_t i = 0; i < template_types.size(); ++i){
                        replacements[source_types[i]] = template_types[i];    
                    }

                    InstructionAdaptor adaptor(replacements);
                    visit_each(adaptor, declaration.Content->instructions);

                    for(auto& param : declaration.Content->parameters){
                        param.parameterType = adaptor.replace(param.parameterType);
                    }

                    //Mark it as instantiated
                    function_template_instantiations[context].insert(ast::TemplateEngine::LocalFunctionInstantiationMap::value_type(name, template_types));

                    function_template_instantiated[context].push_back(declaration);
                }

                functionCall.Content->resolved = true;

                return;
            }

            ++it;
        }

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

        auto template_types = functionCall.Content->template_types;

        if(!template_types.empty()){
            auto object_name = functionCall.Content->object_name;
            auto object_var = current_context->getVariable(object_name);
            auto object_type = object_var->type()->is_pointer() ? object_var->type()->data_type() : object_var->type();
            auto context = object_type->type();

            handle_template(functionCall, context);            
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
        class_templates[parent_struct].insert(ast::TemplateEngine::LocalClassTemplateMap::value_type(template_struct.Content->name, template_struct)); 
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

    Instantiator instantiator(function_templates, function_template_instantiations);
    instantiator(program);

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
