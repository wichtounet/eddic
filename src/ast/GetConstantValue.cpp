//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "variant.hpp"
#include "Variable.hpp"
#include "Type.hpp"

#include "ast/GetConstantValue.hpp"
#include "ast/Value.hpp"

using namespace eddic;

Val ast::GetConstantValue::operator()(const ast::Literal& literal) const {
    return make_pair(literal.value, literal.value.size() - 2);
}

Val ast::GetConstantValue::operator()(const ast::Integer& integer) const {
    return integer.value;
}

Val ast::GetConstantValue::operator()(const ast::Boolean& literal) const {
    return literal.value;
}

Val ast::GetConstantValue::operator()(const ast::IntegerSuffix& integer) const {
    return (double) integer.value;
}

Val ast::GetConstantValue::operator()(const ast::Float& float_) const {
    return float_.value;
}

Val ast::GetConstantValue::operator()(const ast::PrefixOperation& minus) const {
    if(minus.Content->op == ast::Operator::SUB){
        return -1 * boost::get<int>(boost::apply_visitor(*this, minus.Content->left_value));
    }

    cpp_unreachable("Not constant");
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

    cpp_unreachable("This variable is of a type that cannot be constant");
}
