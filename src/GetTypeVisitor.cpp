//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant/apply_visitor.hpp>

#include "GetTypeVisitor.hpp"

#include "ast/Value.hpp"

#include "Context.hpp"
#include "Variable.hpp"

using namespace eddic;

Type GetTypeVisitor::operator()(const ast::Litteral&) const {
    return Type(BaseType::STRING, false);
}

Type GetTypeVisitor::operator()(const ast::Integer&) const {
    return Type(BaseType::INT, false);
}

Type GetTypeVisitor::operator()(const ast::Plus&) const {
    return Type(BaseType::INT, false);
}

Type GetTypeVisitor::operator()(const ast::Minus&) const {
    return Type(BaseType::INT, false);
}

Type GetTypeVisitor::operator()(const ast::VariableValue& variable) const {
    return variable.Content->context->getVariable(variable.Content->variableName)->type();
}

Type GetTypeVisitor::operator()(const ast::ArrayValue& array) const {
    return Type(array.Content->context->getVariable(array.Content->arrayName)->type().base(), false);
}

Type GetTypeVisitor::operator()(const ast::ComposedValue& value) const {
    //No need to recurse into operations because type are enforced in the check variables phase
    return boost::apply_visitor(*this, value.Content->first);
}

Type GetTypeVisitor::operator()(const ast::FunctionCall& call) const {
    std::string name = call.Content->functionName;

    assert(name != "println" && name != "print");

    return call.Content->function->returnType;
}
