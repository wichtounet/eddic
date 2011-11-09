//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include <boost/variant/static_visitor.hpp>

#include "ast/Program.hpp"

#define AUTO_RECURSE_BINARY_CONDITION()\
void operator()(ASTBinaryCondition& binaryCondition){\
    visit(*this, binaryCondition.Content->lhs);\
    visit(*this, binaryCondition.Content->rhs);\
}\

#define AUTO_RECURSE_BRANCHES()\
void operator()(ASTIf& if_){\
    visit(*this, if_.Content->condition);\
    visit_each(*this, if_.Content->instructions);\
    visit_each_non_variant(*this, if_.Content->elseIfs);\
    visit_optional_non_variant(*this, if_.Content->else_);\
}\
void operator()(ASTElseIf& elseIf){\
    visit(*this, elseIf.condition);\
    visit_each(*this, elseIf.instructions);\
}\
void operator()(ASTElse& else_){\
    visit_each(*this, else_.instructions);\
}

#define AUTO_RECURSE_SIMPLE_LOOPS()\
void operator()(ASTFor& for_){\
    visit_optional(*this, for_.Content->start);\
    visit_optional(*this, for_.Content->condition);\
    visit_optional(*this, for_.Content->repeat);\
    visit_each(*this, for_.Content->instructions);\
}\
void operator()(ASTWhile& while_){\
    visit(*this, while_.Content->condition);\
    visit_each(*this, while_.Content->instructions);\
}

#define AUTO_RECURSE_FOREACH()\
void operator()(ASTForeach& foreach_){\
    visit_each(*this, foreach_.Content->instructions);\
}

#define AUTO_RECURSE_VARIABLE_OPERATIONS()\
void operator()(ASTAssignment& assignment){\
    visit(*this, assignment.Content->value);\
}\
void operator()(ASTDeclaration& declaration){\
    visit(*this, declaration.Content->value);\
}

#define AUTO_RECURSE_FUNCTION_CALLS()\
void operator()(ASTFunctionCall& functionCall){\
    visit_each(*this, functionCall.Content->values);\
}

#define AUTO_RECURSE_COMPOSED_VALUES()\
void operator()(ASTComposedValue& value){\
    visit(*this, value.Content->first);\
    for_each(value.Content->operations.begin(), value.Content->operations.end(), \
        [&](boost::tuple<char, ASTValue>& operation){ visit(*this, operation.get<1>()); });\
}

#define AUTO_RECURSE_PROGRAM()\
void operator()(ASTProgram& program){\
    visit_each(*this, program.Content->blocks);\
}

#define AUTO_RECURSE_FUNCTION_DECLARATION()\
void operator()(ASTFunctionDeclaration& function){\
    visit_each(*this, function.Content->instructions);\
}

#define AUTO_RECURSE_GLOBAL_DECLARATION()\
void operator()(GlobalVariableDeclaration& declaration){\
    visit(*this, declaration.Content->value);\
}

#endif
