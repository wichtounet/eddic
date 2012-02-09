//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#define AUTO_RECURSE_BINARY_CONDITION()\
void operator()(ast::BinaryCondition& binaryCondition){\
    visit(*this, binaryCondition.Content->lhs);\
    visit(*this, binaryCondition.Content->rhs);\
}\

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
void operator()(ast::Else& else_){\
    visit_each(*this, else_.instructions);\
}

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
}

#define AUTO_RECURSE_FOREACH()\
void operator()(ast::Foreach& foreach_){\
    visit_each(*this, foreach_.Content->instructions);\
}\
void operator()(ast::ForeachIn& foreach_){\
    visit_each(*this, foreach_.Content->instructions);\
}

#define AUTO_RECURSE_VARIABLE_OPERATIONS()\
void operator()(ast::Assignment& assignment){\
    visit(*this, assignment.Content->value);\
}\
void operator()(ast::VariableDeclaration& declaration){\
    visit(*this, *declaration.Content->value);\
}

#define AUTO_RECURSE_RETURN_VALUES()\
void operator()(ast::Return& return_){\
    visit(*this, return_.Content->value);\
}

#define AUTO_RECURSE_ARRAY_ASSIGNMENT()\
void operator()(ast::ArrayAssignment& assignment){\
    visit(*this, assignment.Content->indexValue);\
    visit(*this, assignment.Content->value);\
}

#define AUTO_RECURSE_FUNCTION_CALLS()\
void operator()(ast::FunctionCall& functionCall){\
    visit_each(*this, functionCall.Content->values);\
}

#define AUTO_RECURSE_BUILTIN_OPERATORS()\
void operator()(ast::BuiltinOperator& builtin){\
    visit_each(*this, builtin.Content->values);\
}

#define AUTO_RECURSE_COMPOSED_VALUES()\
void operator()(ast::ComposedValue& value){\
    visit(*this, value.Content->first);\
    for_each(value.Content->operations.begin(), value.Content->operations.end(), \
        [&](ast::Operation& operation){ visit(*this, operation.get<1>()); });\
}

#define AUTO_RECURSE_MINUS_PLUS_VALUES()\
void operator()(ast::Plus& value){\
    visit(*this, value.Content->value);\
}\
void operator()(ast::Minus& value){\
    visit(*this, value.Content->value);\
}
        
#define AUTO_RECURSE_ARRAY_VALUES()\
void operator()(ast::ArrayValue& array){\
    visit(*this, array.Content->indexValue);\
}

#define AUTO_RECURSE_PROGRAM()\
void operator()(ast::SourceFile& program){\
    visit_each(*this, program.Content->blocks);\
}

#define AUTO_RECURSE_FUNCTION_DECLARATION()\
void operator()(ast::FunctionDeclaration& function){\
    visit_each(*this, function.Content->instructions);\
}

#define AUTO_RECURSE_GLOBAL_DECLARATION()\
void operator()(ast::GlobalVariableDeclaration& declaration){\
    visit(*this, *declaration.Content->value);\
}

#endif
