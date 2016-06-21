//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "cpp_utils/assert.hpp"

#include "VisitorUtils.hpp"

#define AUTO_RECURSE_PROGRAM()\
void operator()(ast::SourceFile& program){\
    visit_each(*this, program.blocks);\
}

/* Functions */

#define AUTO_RECURSE_TEMPLATE_FUNCTION_DECLARATION()\
void operator()(ast::TemplateFunctionDeclaration& function){\
    if(function.is_template()){ \
        visit_each(*this, function.instructions);\
    } \
}

#define AUTO_RECURSE_FUNCTION_DECLARATION()\
void operator()(ast::TemplateFunctionDeclaration& function){\
    if(!function.is_template()){ \
        visit_each(*this, function.instructions);\
    } \
}

#define AUTO_RECURSE_ALL_FUNCTION_DECLARATION()\
void operator()(ast::TemplateFunctionDeclaration& function){\
    visit_each(*this, function.instructions);\
}

/* Instructions */

#define AUTO_RECURSE_TERNARY()\
void operator()(ast::Ternary& ternary){\
    visit(*this, ternary.condition);\
    visit(*this, ternary.true_value);\
    visit(*this, ternary.false_value);\
}

#define AUTO_RECURSE_ELSE()\
void operator()(ast::Else& else_){\
    visit_each(*this, else_.instructions);\
}

#define AUTO_RECURSE_BRANCHES()\
void operator()(ast::If& if_){\
    visit(*this, if_.condition);\
    visit_each(*this, if_.instructions);\
    visit_each_non_variant(*this, if_.elseIfs);\
    visit_optional_non_variant(*this, if_.else_);\
}\
void operator()(ast::ElseIf& elseIf){\
    visit(*this, elseIf.condition);\
    visit_each(*this, elseIf.instructions);\
}\
AUTO_RECURSE_ELSE()

#define AUTO_RECURSE_SIMPLE_LOOPS()\
void operator()(ast::For& for_){\
    visit_optional(*this, for_.start);\
    visit_optional(*this, for_.condition);\
    visit_optional(*this, for_.repeat);\
    visit_each(*this, for_.instructions);\
}\
void operator()(ast::While& while_){\
    visit(*this, while_.condition);\
    visit_each(*this, while_.instructions);\
}\
void operator()(ast::DoWhile& while_){\
    visit(*this, while_.condition);\
    visit_each(*this, while_.instructions);\
}

#define AUTO_RECURSE_FOREACH()\
void operator()(ast::Foreach& foreach_){\
    visit_each(*this, foreach_.instructions);\
}\
void operator()(ast::ForeachIn& foreach_){\
    visit_each(*this, foreach_.instructions);\
}

#define AUTO_RECURSE_SWITCH()\
void operator()(ast::Switch& switch_){\
    visit(*this, switch_.value);\
    visit_each_non_variant(*this, switch_.cases);\
    visit_optional_non_variant(*this, switch_.default_case);\
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
    visit(*this, assignment.left_value);\
    visit(*this, assignment.value);\
}\
void operator()(ast::VariableDeclaration& declaration){\
    visit_optional(*this, declaration.value);\
}

#define AUTO_RECURSE_RETURN_VALUES()\
void operator()(ast::Return& return_){\
    visit(*this, return_.value);\
}

#define AUTO_RECURSE_STRUCT_DECLARATION()\
void operator()(ast::StructDeclaration& declaration){\
    visit_each(*this, declaration.values);\
}

#define AUTO_RECURSE_FUNCTION_CALLS()\
void operator()(ast::FunctionCall& functionCall){\
    visit_each(*this, functionCall.values);\
}

#define AUTO_RECURSE_BUILTIN_OPERATORS()\
void operator()(ast::BuiltinOperator& builtin){\
    visit_each(*this, builtin.values);\
}

#define VISIT_COMPOSED_VALUE(value)\
visit(*this, value.first);\
for(auto& op : value.operations){\
    if(ast::has_operation_value(op)){\
        visit(*this, op.get<1>());\
    }\
}

#define AUTO_RECURSE_COMPOSED_VALUES()\
void operator()(ast::Expression& value){\
    VISIT_COMPOSED_VALUE(value)\
}

#define AUTO_RECURSE_CAST_VALUES()\
void operator()(ast::Cast& cast){\
    visit(*this, cast.value);\
}

#define AUTO_RECURSE_PREFIX()\
void operator()(ast::PrefixOperation& operation){\
    visit(*this, operation.left_value);\
}

#define AUTO_RECURSE_CONSTRUCTOR()\
void operator()(ast::Constructor& function){\
    visit_each(*this, function.instructions);\
}

#define AUTO_RECURSE_DESTRUCTOR()\
void operator()(ast::Destructor& function){\
    visit_each(*this, function.instructions);\
}

#define AUTO_RECURSE_GLOBAL_DECLARATION()\
void operator()(ast::GlobalVariableDeclaration& declaration){\
    visit_optional(*this, declaration.value);\
}

#define AUTO_RECURSE_NEW()\
void operator()(ast::New& new_){\
    visit_each(*this, new_.values);\
}

#define AUTO_RECURSE_DELETE()\
void operator()(ast::Delete& delete_){\
    visit(*this, delete_.value);\
}

#define AUTO_RECURSE_NEW_ARRAY()\
void operator()(ast::NewArray& new_){\
    visit(*this, new_.size);\
}

/* Ignore macros  */

#define AUTO_IGNORE_ARRAY_DECLARATION() void operator()(ast::ArrayDeclaration&){}
#define AUTO_IGNORE_ASSIGNMENT() void operator()(ast::Assignment&){}
#define AUTO_IGNORE_BUILTIN_OPERATOR() void operator()(ast::BuiltinOperator&){}
#define AUTO_IGNORE_CAST() void operator()(ast::Cast&){}
#define AUTO_IGNORE_DELETE() void operator()(ast::Delete&){}
#define AUTO_IGNORE_EXPRESSION() void operator()(ast::Expression&){}
#define AUTO_IGNORE_FLOAT() void operator()(ast::Float&){}
#define AUTO_IGNORE_FOR_LOOP() void operator()(ast::For&){}
#define AUTO_IGNORE_FOREACH_LOOP() void operator()(ast::Foreach&){}
#define AUTO_IGNORE_FOREACH_IN_LOOP() void operator()(ast::ForeachIn&){}
#define AUTO_IGNORE_FUNCTION_CALLS() void operator()(ast::FunctionCall&){}
#define AUTO_IGNORE_TEMPLATE_FUNCTION_DECLARATION() void operator()(ast::TemplateFunctionDeclaration&){}
#define AUTO_IGNORE_TEMPLATE_STRUCT() void operator()(ast::struct_definition&){}
#define AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION() void operator()(ast::GlobalArrayDeclaration&){}
#define AUTO_IGNORE_GLOBAL_VARIABLE_DECLARATION() void operator()(ast::GlobalVariableDeclaration&){}
#define AUTO_IGNORE_IMPORT() void operator()(ast::Import&){}
#define AUTO_IGNORE_INTEGER() void operator()(ast::Integer&){}
#define AUTO_IGNORE_INTEGER_SUFFIX() void operator()(ast::IntegerSuffix&){}
#define AUTO_IGNORE_LITERAL() void operator()(ast::Literal&){}
#define AUTO_IGNORE_CHAR_LITERAL() void operator()(ast::CharLiteral&){}
#define AUTO_IGNORE_MEMBER_DECLARATION() void operator()(ast::MemberDeclaration&){}
#define AUTO_IGNORE_NEW() void operator()(ast::New&){}
#define AUTO_IGNORE_NEW_ARRAY() void operator()(ast::NewArray&){}
#define AUTO_IGNORE_NULL() void operator()(ast::Null&){}
#define AUTO_IGNORE_PREFIX_OPERATION() void operator()(ast::PrefixOperation&){}
#define AUTO_IGNORE_RETURN() void operator()(ast::Return&){}
#define AUTO_IGNORE_STANDARD_IMPORT() void operator()(ast::StandardImport&){}
#define AUTO_IGNORE_SWITCH() void operator()(ast::Switch&){}
#define AUTO_IGNORE_SWITCH_CASE() void operator()(ast::SwitchCase&){}
#define AUTO_IGNORE_DEFAULT_CASE() void operator()(ast::DefaultCase&){}
#define AUTO_IGNORE_BOOLEAN() void operator()(ast::Boolean&){}
#define AUTO_IGNORE_TERNARY() void operator()(ast::Ternary&){}
#define AUTO_IGNORE_STRUCT_DECLARATION() void operator()(ast::StructDeclaration&){}
#define AUTO_IGNORE_VARIABLE_DECLARATION() void operator()(ast::VariableDeclaration&){}
#define AUTO_IGNORE_VARIABLE_VALUE() void operator()(ast::VariableValue&){}

/* auto return macros */

#define AUTO_RETURN_ARRAY_DECLARATION(return_type) return_type operator()(ast::ArrayDeclaration& t){return return_type(t);}
#define AUTO_RETURN_ASSIGNMENT(return_type) return_type operator()(ast::Assignment& t){return return_type(t);}
#define AUTO_RETURN_BUILTIN_OPERATOR(return_type) return_type operator()(ast::BuiltinOperator& t){return return_type(t);}
#define AUTO_RETURN_CAST(return_type) return_type operator()(ast::Cast& t){return return_type(t);}
#define AUTO_RETURN_EXPRESSION(return_type) return_type operator()(ast::Expression& t){return return_type(t);}
#define AUTO_RETURN_FLOAT(return_type) return_type operator()(ast::Float& t){return return_type(t);}
#define AUTO_RETURN_FOR_LOOP(return_type) return_type operator()(ast::For& t){return return_type(t);}
#define AUTO_RETURN_FOREACH_LOOP(return_type) return_type operator()(ast::Foreach& t){return return_type(t);}
#define AUTO_RETURN_FOREACH_IN_LOOP(return_type) return_type operator()(ast::ForeachIn& t){return return_type(t);}
#define AUTO_RETURN_FUNCTION_CALLS(return_type) return_type operator()(ast::FunctionCall& t){return return_type(t);}
#define AUTO_RETURN_GLOBAL_ARRAY_DECLARATION(return_type) return_type operator()(ast::GlobalArrayDeclaration& t){return return_type(t);}
#define AUTO_RETURN_GLOBAL_VARIABLE_DECLARATION(return_type) return_type operator()(ast::GlobalVariableDeclaration& t){return return_type(t);}
#define AUTO_RETURN_IMPORT(return_type) return_type operator()(ast::Import& t){return return_type(t);}
#define AUTO_RETURN_INTEGER(return_type) return_type operator()(ast::Integer& t){return return_type(t);}
#define AUTO_RETURN_INTEGER_SUFFIX(return_type) return_type operator()(ast::IntegerSuffix& t){return return_type(t);}
#define AUTO_RETURN_LITERAL(return_type) return_type operator()(ast::Literal& t){return return_type(t);}
#define AUTO_RETURN_CHAR_LITERAL(return_type) return_type operator()(ast::CharLiteral& t){return return_type(t);}
#define AUTO_RETURN_NEW(return_type) return_type operator()(ast::New& t){return return_type(t);}
#define AUTO_RETURN_PREFIX_OPERATION(return_type) return_type operator()(ast::PrefixOperation& t){return return_type(t);}
#define AUTO_RETURN_RETURN(return_type) return_type operator()(ast::Return& t){return return_type(t);}
#define AUTO_RETURN_STANDARD_IMPORT(return_type) return_type operator()(ast::StandardImport& t){return return_type(t);}
#define AUTO_RETURN_BOOLEAN(return_type) return_type operator()(ast::Boolean& t){return return_type(t);}
#define AUTO_RETURN_NULL(return_type) return_type operator()(ast::Null& t){return return_type(t);}
#define AUTO_RETURN_VARIABLE_DECLARATION(return_type) return_type operator()(ast::VariableDeclaration& t){return return_type(t);}
#define AUTO_RETURN_VARIABLE_VALUE(return_type) return_type operator()(ast::VariableValue& t){return return_type(t);}

//The following macros should be used very sparsely as they are increasing the
//difficulty to add new AST Node in the right way

#define AUTO_IGNORE_OTHERS() template<typename T> void operator()(T&){}
#define AUTO_IGNORE_OTHERS_CONST() template<typename T> void operator()(T&) const {}
#define AUTO_IGNORE_OTHERS_CONST_CONST() template<typename T> void operator()(const T&) const {}

#define AUTO_RETURN_OTHERS(return_type) template<typename T> return_type operator()(T& t){return return_type(t);}
#define AUTO_RETURN_OTHERS_CONST(return_type) template<typename T> return_type operator()(T& t) const {return return_type(t);}

/* Break macros  */

#define AUTO_BREAK_OTHERS() template<typename T> result_type operator()(T&){ cpp_unreachable("Type not supported in the visitor"); }
#define AUTO_BREAK_OTHERS_CONST() template<typename T> result_type operator()(T&) const { cpp_unreachable("Type not supported in the visitor"); }

#endif
