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

bool IsImmediateVisitor::operator()(ASTLitteral&) const {
    return true;
}

bool IsImmediateVisitor::operator()(ASTInteger&) const {
    return true;
}

bool IsImmediateVisitor::operator()(ASTVariable&) const {
    return true;
}

bool IsImmediateVisitor::operator()(ASTComposedValue&) const {
    return false;
}
