//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "assert.hpp"
#include "VisitorUtils.hpp"

#define AUTO_RECURSE_PROGRAM()\
void operator()(ast::SourceFile& program){\
    visit_each(*this, program.Content->blocks);\
}

/* Structures */

#define AUTO_RECURSE_TEMPLATE_STRUCT()\
void operator()(ast::TemplateStruct& struct_){\
    visit_each_non_variant(*this, struct_.Content->arrays);\
    visit_each_non_variant(*this, struct_.Content->constructors);\
    visit_each_non_variant(*this, struct_.Content->destructors);\
    visit_each_non_variant(*this, struct_.Content->functions);\
    visit_each_non_variant(*this, struct_.Content->template_functions);\
}

#define AUTO_RECURSE_STRUCT()\
void operator()(ast::Struct& struct_){\
    visit_each_non_variant(*this, struct_.Content->arrays);\
    visit_each_non_variant(*this, struct_.Content->constructors);\
    visit_each_non_variant(*this, struct_.Content->destructors);\
    visit_each_non_variant(*this, struct_.Content->functions);\
    visit_each_non_variant(*this, struct_.Content->template_functions);\
}

/* Functions */

#define AUTO_RECURSE_TEMPLATE_FUNCTION_DECLARATION()\
void operator()(ast::TemplateFunctionDeclaration& function){\
    visit_each(*this, function.Content->instructions);\
}

#define AUTO_RECURSE_FUNCTION_DECLARATION()\
void operator()(ast::FunctionDeclaration& function){\
    visit_each(*this, function.Content->instructions);\
}

/* Instructions */

#define AUTO_RECURSE_TERNARY()\
void operator()(ast::Ternary& ternary){\
    visit(*this, ternary.Content->condition);\
    visit(*this, ternary.Content->true_value);\
    visit(*this, ternary.Content->false_value);\
}

#define AUTO_RECURSE_ELSE()\
void operator()(ast::Else& else_){\
    visit_each(*this, else_.instructions);\
}

#define AUTO_RECURSE_BRANCHES()\
void operator()(ast::If& if_){\
    visit(*this, if_.Content->condition);\
    visit_each(*this, if_.Content->instructions);\
    visit_each_non_variant(*this, if_.Content->elseIfs);\
    visit_optional_non_variant(*this, if_.Content->else_);\
}\
void operator()(ast::ElseIf& elseIf){\
    visit(*this, elseIf.condition);\
    visit_each(*this, elseIf.instructions);\
}\
AUTO_RECURSE_ELSE()

#define AUTO_RECURSE_SIMPLE_LOOPS()\
void operator()(ast::For& for_){\
    visit_optional(*this, for_.Content->start);\
    visit_optional(*this, for_.Content->condition);\
    visit_optional(*this, for_.Content->repeat);\
    visit_each(*this, for_.Content->instructions);\
}\
void operator()(ast::While& while_){\
    visit(*this, while_.Content->condition);\
    visit_each(*this, while_.Content->instructions);\
}\
void operator()(ast::DoWhile& while_){\
    visit(*this, while_.Content->condition);\
    visit_each(*this, while_.Content->instructions);\
}

#define AUTO_RECURSE_FOREACH()\
void operator()(ast::Foreach& foreach_){\
    visit_each(*this, foreach_.Content->instructions);\
}\
void operator()(ast::ForeachIn& foreach_){\
    visit_each(*this, foreach_.Content->instructions);\
}

#define AUTO_RECURSE_SWITCH()\
void operator()(ast::Switch& switch_){\
    visit(*this, switch_.Content->value);\
    visit_each_non_variant(*this, switch_.Content->cases);\
    visit_optional_non_variant(*this, switch_.Content->default_case);\
}

#define AUTO_RECURSE_SWITCH_CASE()\
void operator()(ast::SwitchCase& switch_case){\
    visit(*this, switch_case.value);\
    visit_each(*this, switch_case.instructions);\
}

#define AUTO_RECURSE_DEFAULT_CASE()\
void operator()(ast::DefaultCase& default_case){\
    visit_each(*this, default_case.instructions);\
}

#define AUTO_RECURSE_VARIABLE_OPERATIONS()\
void operator()(ast::Assignment& assignment){\
    visit(*this, assignment.Content->left_value);\
    visit(*this, assignment.Content->value);\
}\
void operator()(ast::VariableDeclaration& declaration){\
    visit_optional(*this, declaration.Content->value);\
}

#define AUTO_RECURSE_RETURN_VALUES()\
void operator()(ast::Return& return_){\
    visit(*this, return_.Content->value);\
}

#define AUTO_RECURSE_STRUCT_DECLARATION()\
void operator()(ast::StructDeclaration& declaration){\
    visit_each(*this, declaration.Content->values);\
}

#define AUTO_RECURSE_FUNCTION_CALLS()\
void operator()(ast::FunctionCall& functionCall){\
    visit_each(*this, functionCall.Content->values);\
}

#define AUTO_RECURSE_MEMBER_FUNCTION_CALLS()\
void operator()(ast::MemberFunctionCall& functionCall){\
    visit(*this, functionCall.Content->object);\
    visit_each(*this, functionCall.Content->values);\
}

#define AUTO_RECURSE_BUILTIN_OPERATORS()\
void operator()(ast::BuiltinOperator& builtin){\
    visit_each(*this, builtin.Content->values);\
}

#define VISIT_COMPOSED_VALUE(value)\
visit(*this, value.Content->first);\
for(auto& op : value.Content->operations){\
    if(op.get<1>()){\
        if(auto* ptr = boost::get<ast::Value>(&*op.get<1>())){\
            visit(*this, *ptr);\
        } else if(auto* ptr = boost::get<ast::CallOperationValue>(&*op.get<1>())){\
            visit_each(*this, ptr->get<1>());\
        }\
    }\
}

#define AUTO_RECURSE_COMPOSED_VALUES()\
void operator()(ast::Expression& value){\
    VISIT_COMPOSED_VALUE(value)\
}

#define AUTO_RECURSE_CAST_VALUES()\
void operator()(ast::Cast& cast){\
    visit(*this, cast.Content->value);\
}

#define AUTO_RECURSE_PREFIX()\
void operator()(ast::PrefixOperation& operation){\
    visit(*this, operation.Content->left_value);\
}
        
#define AUTO_RECURSE_CONSTRUCTOR()\
void operator()(ast::Constructor& function){\
    visit_each(*this, function.Content->instructions);\
}

#define AUTO_RECURSE_DESTRUCTOR()\
void operator()(ast::Destructor& function){\
    visit_each(*this, function.Content->instructions);\
}

#define AUTO_RECURSE_GLOBAL_DECLARATION()\
void operator()(ast::GlobalVariableDeclaration& declaration){\
    visit_optional(*this, declaration.Content->value);\
}

#define AUTO_RECURSE_NEW()\
void operator()(ast::New& new_){\
    visit_each(*this, new_.Content->values);\
}

#define AUTO_RECURSE_NEW_ARRAY()\
void operator()(ast::NewArray& new_){\
    visit(*this, new_.Content->size);\
}

/* Ignore macros  */

#define AUTO_IGNORE_ARRAY_DECLARATION() void operator()(ast::ArrayDeclaration&){}
#define AUTO_IGNORE_ASSIGNMENT() void operator()(ast::Assignment&){}
#define AUTO_IGNORE_BUILTIN_OPERATOR() void operator()(ast::BuiltinOperator&){}
#define AUTO_IGNORE_CAST() void operator()(ast::Cast&){}
#define AUTO_IGNORE_DELETE() void operator()(ast::Delete&){}
#define AUTO_IGNORE_EXPRESSION() void operator()(ast::Expression&){}
#define AUTO_IGNORE_FALSE() void operator()(ast::False&){}
#define AUTO_IGNORE_FLOAT() void operator()(ast::Float&){}
#define AUTO_IGNORE_FOR_LOOP() void operator()(ast::For&){}
#define AUTO_IGNORE_FOREACH_LOOP() void operator()(ast::Foreach&){}
#define AUTO_IGNORE_FOREACH_IN_LOOP() void operator()(ast::ForeachIn&){}
#define AUTO_IGNORE_FUNCTION_CALLS() void operator()(ast::FunctionCall&){}
#define AUTO_IGNORE_TEMPLATE_FUNCTION_DECLARATION() void operator()(ast::TemplateFunctionDeclaration&){}
#define AUTO_IGNORE_TEMPLATE_STRUCT() void operator()(ast::TemplateStruct&){}
#define AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION() void operator()(ast::GlobalArrayDeclaration&){}
#define AUTO_IGNORE_GLOBAL_VARIABLE_DECLARATION() void operator()(ast::GlobalVariableDeclaration&){}
#define AUTO_IGNORE_IMPORT() void operator()(ast::Import&){}
#define AUTO_IGNORE_INTEGER() void operator()(ast::Integer&){}
#define AUTO_IGNORE_INTEGER_SUFFIX() void operator()(ast::IntegerSuffix&){}
#define AUTO_IGNORE_LITERAL() void operator()(ast::Literal&){}
#define AUTO_IGNORE_CHAR_LITERAL() void operator()(ast::CharLiteral&){}
#define AUTO_IGNORE_MEMBER_FUNCTION_CALLS() void operator()(ast::MemberFunctionCall&){}
#define AUTO_IGNORE_NEW() void operator()(ast::New&){}
#define AUTO_IGNORE_NEW_ARRAY() void operator()(ast::NewArray&){}
#define AUTO_IGNORE_NULL() void operator()(ast::Null&){}
#define AUTO_IGNORE_PREFIX_OPERATION() void operator()(ast::PrefixOperation&){}
#define AUTO_IGNORE_RETURN() void operator()(ast::Return&){}
#define AUTO_IGNORE_STANDARD_IMPORT() void operator()(ast::StandardImport&){}
#define AUTO_IGNORE_STRUCT() void operator()(ast::Struct&){}
#define AUTO_IGNORE_SWAP() void operator()(ast::Swap&){}
#define AUTO_IGNORE_SWITCH() void operator()(ast::Switch&){}
#define AUTO_IGNORE_SWITCH_CASE() void operator()(ast::SwitchCase&){}
#define AUTO_IGNORE_DEFAULT_CASE() void operator()(ast::DefaultCase&){}
#define AUTO_IGNORE_TRUE() void operator()(ast::True&){}
#define AUTO_IGNORE_TERNARY() void operator()(ast::Ternary&){}
#define AUTO_IGNORE_STRUCT_DECLARATION() void operator()(ast::StructDeclaration&){}
#define AUTO_IGNORE_VARIABLE_DECLARATION() void operator()(ast::VariableDeclaration&){}
#define AUTO_IGNORE_VARIABLE_VALUE() void operator()(ast::VariableValue&){}

/* auto return macros */ 

#define AUTO_RETURN_ARRAY_DECLARATION(return_type) return_type operator()(ast::ArrayDeclaration& t){return t;}
#define AUTO_RETURN_ASSIGNMENT(return_type) return_type operator()(ast::Assignment& t){return t;}
#define AUTO_RETURN_BUILTIN_OPERATOR(return_type) return_type operator()(ast::BuiltinOperator& t){return t;}
#define AUTO_RETURN_CAST(return_type) return_type operator()(ast::Cast& t){return t;}
#define AUTO_RETURN_EXPRESSION(return_type) return_type operator()(ast::Expression& t){return t;}
#define AUTO_RETURN_FALSE(return_type) return_type operator()(ast::False& t){return t;}
#define AUTO_RETURN_FLOAT(return_type) return_type operator()(ast::Float& t){return t;}
#define AUTO_RETURN_FOR_LOOP(return_type) return_type operator()(ast::For& t){return t;}
#define AUTO_RETURN_FOREACH_LOOP(return_type) return_type operator()(ast::Foreach& t){return t;}
#define AUTO_RETURN_FOREACH_IN_LOOP(return_type) return_type operator()(ast::ForeachIn& t){return t;}
#define AUTO_RETURN_FUNCTION_CALLS(return_type) return_type operator()(ast::FunctionCall& t){return t;}
#define AUTO_RETURN_GLOBAL_ARRAY_DECLARATION(return_type) return_type operator()(ast::GlobalArrayDeclaration& t){return t;}
#define AUTO_RETURN_GLOBAL_VARIABLE_DECLARATION(return_type) return_type operator()(ast::GlobalVariableDeclaration& t){return t;}
#define AUTO_RETURN_IMPORT(return_type) return_type operator()(ast::Import& t){return t;}
#define AUTO_RETURN_INTEGER(return_type) return_type operator()(ast::Integer& t){return t;}
#define AUTO_RETURN_INTEGER_SUFFIX(return_type) return_type operator()(ast::IntegerSuffix& t){return t;}
#define AUTO_RETURN_LITERAL(return_type) return_type operator()(ast::Literal& t){return t;}
#define AUTO_RETURN_CHAR_LITERAL(return_type) return_type operator()(ast::CharLiteral& t){return t;}
#define AUTO_RETURN_NEW(return_type) return_type operator()(ast::New& t){return t;}
#define AUTO_RETURN_PREFIX_OPERATION(return_type) return_type operator()(ast::PrefixOperation& t){return t;}
#define AUTO_RETURN_RETURN(return_type) return_type operator()(ast::Return& t){return t;}
#define AUTO_RETURN_STANDARD_IMPORT(return_type) return_type operator()(ast::StandardImport& t){return t;}
#define AUTO_RETURN_STRUCT(return_type) return_type operator()(ast::Struct& t){return t;}
#define AUTO_RETURN_SWAP(return_type) return_type operator()(ast::Swap& t){return t;}
#define AUTO_RETURN_TRUE(return_type) return_type operator()(ast::True& t){return t;}
#define AUTO_RETURN_NULL(return_type) return_type operator()(ast::Null& t){return t;}
#define AUTO_RETURN_VARIABLE_DECLARATION(return_type) return_type operator()(ast::VariableDeclaration& t){return t;}
#define AUTO_RETURN_VARIABLE_VALUE(return_type) return_type operator()(ast::VariableValue& t){return t;}

//The following macros should be used very sparsely as they are increasing the
//difficulty to add new AST Node in the right way

#define AUTO_IGNORE_OTHERS() template<typename T> void operator()(T&){}
#define AUTO_IGNORE_OTHERS_CONST() template<typename T> void operator()(T&) const {}
#define AUTO_IGNORE_OTHERS_CONST_CONST() template<typename T> void operator()(const T&) const {}

#define AUTO_RETURN_OTHERS(return_type) template<typename T> return_type operator()(T& t){return t;}
#define AUTO_RETURN_OTHERS_CONST(return_type) template<typename T> return_type operator()(T& t) const {return t;}

/* Break macros  */
    
#define AUTO_BREAK_OTHERS() template<typename T> result_type operator()(T&){ eddic_unreachable("Type not supported in the visitor"); }
#define AUTO_BREAK_OTHERS_CONST() template<typename T> result_type operator()(T&) const { eddic_unreachable("Type not supported in the visitor"); }

#endif
