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

Type GetTypeVisitor::operator()(const ASTLitteral&) const {
    return Type::STRING;
}

Type GetTypeVisitor::operator()(const ASTInteger&) const {
    return Type::INT;
}

Type GetTypeVisitor::operator()(const ASTVariable& variable) const {
    return variable.Content->context->getVariable(variable.Content->variableName)->type();
}

Type GetTypeVisitor::operator()(const ASTComposedValue& value) const {
    //No need to recurse into operations because type are enforced in the check variables phase
    return boost::apply_visitor(*this, value.Content->first);
}
