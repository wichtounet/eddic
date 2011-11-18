//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant/apply_visitor.hpp>

#include "IsImmediateVisitor.hpp"

#include "ast/Value.hpp"

using namespace eddic;

bool IsImmediateVisitor::operator()(ast::Litteral&) const {
    return true;
}

bool IsImmediateVisitor::operator()(ast::Integer&) const {
    return true;
}

bool IsImmediateVisitor::operator()(ast::VariableValue&) const {
    return true;
}

bool IsImmediateVisitor::operator()(ast::ArrayValue&) const {
    return true;//TODO Perhaps check if the index value is immediate instead of returning true
}

bool IsImmediateVisitor::operator()(ast::ComposedValue&) const {
    return false;
}
