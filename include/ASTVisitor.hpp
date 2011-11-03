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
virtual void operator()(ASTBinaryCondition& binaryCondition){\
    visit(*this, binaryCondition);\
}\
virtual void operator()(ASTEquals& equals){\
    visit(*this, equals.lhs);\
    visit(*this, equals.rhs);\
}\
virtual void operator()(ASTNotEquals& notEquals){\
    visit(*this, notEquals.lhs);\
    visit(*this, notEquals.rhs);\
}\
virtual void operator()(ASTLess& less){\
    visit(*this, less.lhs);\
    visit(*this, less.rhs);\
}\
virtual void operator()(ASTLessEquals& less){\
    visit(*this, less.lhs);\
    visit(*this, less.rhs);\
}\
virtual void operator()(ASTGreater& greater){\
    visit(*this, greater.lhs);\
    visit(*this, greater.rhs);\
}\
virtual void operator()(ASTGreaterEquals& greater){\
    visit(*this, greater.lhs);\
    visit(*this, greater.rhs);\
}

#define AUTO_RECURSE_BRANCHES()\
void operator()(ASTIf& if_){\
    visit(*this, if_.condition);\
    visit_each(*this, if_.instructions);\
    visit_each_non_variant(*this, if_.elseIfs);\
    visit_optional_non_variant(*this, if_.else_);\
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
    visit_optional(*this, for_.start);\
    visit_optional(*this, for_.condition);\
    visit_optional(*this, for_.repeat);\
    visit_each(*this, for_.instructions);\
}\
void operator()(ASTWhile& while_){\
    visit(*this, while_.condition);\
    visit_each(*this, while_.instructions);\
}

#define AUTO_RECURSE_FUNCTION_CALLS()\
void operator()(ASTFunctionCall& functionCall){\
    visit_each(*this, functionCall.values);\
}

#define AUTO_RECURSE_COMPOSED_VALUES()\
void operator()(ASTComposedValue& value){\
    visit(*this, value.first);\
    for_each(value.operations.begin(), value.operations.end(), \
        [&](boost::tuple<char, ASTValue>& operation){ visit(*this, operation.get<1>()); });\
}

#define AUTO_RECURSE_PROGRAM()\
void operator()(ASTProgram& program){\
    visit_each(*this, program.blocks);\
}

#define AUTO_RECURSE_FUNCTION_DECLARATION()\
void operator()(ASTFunctionDeclaration& function){\
    visit_each(*this, function.instructions);\
}

#endif
