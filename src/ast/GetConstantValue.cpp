//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include <boost/variant/variant.hpp>
#include <boost/variant/apply_visitor.hpp>

#include "ast/GetConstantValue.hpp"
#include "ast/Value.hpp"

#include "Variable.hpp"

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

Val ast::GetConstantValue::operator()(const ast::Minus& minus) const {
    return -1 * boost::get<int>(boost::apply_visitor(*this, minus.Content->value));
}

Val ast::GetConstantValue::operator()(const ast::VariableValue& value) const {
    Type type = value.Content->var->type();
    assert(type.isConst());
        
    auto val = value.Content->var->val();

    if(type == BaseType::INT){
        return boost::get<int>(val);
    } else if(type == BaseType::STRING){
        return boost::get<std::pair<std::string, int>>(val);
    }

    assert(false && "This variable is of a type that cannot be constant");
}
