//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_IS_PARAM_SAFE_VISITOR_H
#define TAC_IS_PARAM_SAFE_VISITOR_H

#include <boost/variant/static_visitor.hpp>

#include "ast/values_def.hpp"

namespace eddic {

namespace tac {

struct IsParamSafeVisitor : public boost::static_visitor<bool> {
    bool operator()(const ast::VariableValue& value) const;
    bool operator()(const ast::Integer& value) const;
    bool operator()(const ast::Float& value) const;
    bool operator()(const ast::True& value) const;
    bool operator()(const ast::False& value) const;
    bool operator()(const ast::Litteral& value) const;
    bool operator()(const ast::ArrayValue& value) const;
    bool operator()(const ast::ComposedValue& value) const;
    bool operator()(const ast::Minus& value) const;
    bool operator()(const ast::Plus& value) const;
    bool operator()(const ast::FunctionCall& value) const;
    bool operator()(const ast::BuiltinOperator& value) const;
    bool operator()(const ast::SuffixOperation& value) const;
    bool operator()(const ast::PrefixOperation& value) const;
    bool operator()(const ast::Assignment& value) const;
};

} //end of tac

} //end of eddic

#endif
