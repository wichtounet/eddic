//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"
#include "IsConstantVisitor.hpp"
#include "VisitorUtils.hpp"

#include "ast/Value.hpp"

using namespace eddic;

ASSIGN_INSIDE_CONST(IsConstantVisitor, ast::Litteral, true)
ASSIGN_INSIDE_CONST(IsConstantVisitor, ast::Integer, true)
ASSIGN_INSIDE_CONST(IsConstantVisitor, ast::True, true)
ASSIGN_INSIDE_CONST(IsConstantVisitor, ast::False, true)

ASSIGN_INSIDE_CONST(IsConstantVisitor, ast::ArrayValue, false)
ASSIGN_INSIDE_CONST(IsConstantVisitor, ast::FunctionCall, false)
ASSIGN_INSIDE_CONST(IsConstantVisitor, ast::SuffixOperation, false)
ASSIGN_INSIDE_CONST(IsConstantVisitor, ast::PrefixOperation, false)

bool IsConstantVisitor::operator()(ast::Minus& value) const {
    return visit(*this, value.Content->value);
}

bool IsConstantVisitor::operator()(ast::Plus& value) const {
    return visit(*this, value.Content->value);
}

bool IsConstantVisitor::operator()(ast::VariableValue& variable) const {
    return variable.Content->var->type().isConst();
}

bool IsConstantVisitor::operator()(ast::ComposedValue& value) const {
    if(visit(*this, value.Content->first)){
        for(auto& op : value.Content->operations){
            if(!visit(*this, op.get<1>())){
                return false;
            }
        }

        return true;
    }

    return false;
}
