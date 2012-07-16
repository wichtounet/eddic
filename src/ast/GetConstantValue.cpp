//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "variant.hpp"
#include "Variable.hpp"
#include "Type.hpp"

#include "ast/GetConstantValue.hpp"
#include "ast/Value.hpp"

using namespace eddic;

Val ast::GetConstantValue::operator()(const ast::Litteral& litteral) const {
    return make_pair(litteral.value, litteral.value.size() - 2);
}

Val ast::GetConstantValue::operator()(const ast::Integer& integer) const {
    return integer.value;
}

Val ast::GetConstantValue::operator()(const ast::IntegerSuffix& integer) const {
    return (double) integer.value;
}

Val ast::GetConstantValue::operator()(const ast::Float& float_) const {
    return float_.value;
}

Val ast::GetConstantValue::operator()(const ast::Unary& minus) const {
    if(minus.Content->op == ast::Operator::SUB){
        return -1 * boost::get<int>(boost::apply_visitor(*this, minus.Content->value));
    }

    ASSERT_PATH_NOT_TAKEN("Not constant");
}

Val ast::GetConstantValue::operator()(const ast::VariableValue& value) const {
    auto var = value.variable();
    auto type = var->type();
    auto val = var->val();

    if(type == INT){
        return boost::get<int>(val);
    } else if(type == STRING){
        return boost::get<std::pair<std::string, int>>(val);
    }

    ASSERT_PATH_NOT_TAKEN("This variable is of a type that cannot be constant");
}
