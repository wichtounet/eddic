//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_DEBUG_VISITOR_H
#define AST_DEBUG_VISITOR_H

#include <boost/variant/static_visitor.hpp>

#include "ast/Program.hpp"

namespace eddic {

struct DebugVisitor : public boost::static_visitor<> {
    mutable int level;

    DebugVisitor();

    std::string indent() const ;

    void operator()(ast::Program& program) const ;
    void operator()(ast::FunctionDeclaration& declaration) const ;
    void operator()(ast::GlobalVariableDeclaration&) const ;
    void operator()(ast::GlobalArrayDeclaration&) const ;
    void operator()(ast::For& for_) const ;
    void operator()(ast::Foreach& for_) const ;
    void operator()(ast::ForeachIn& for_) const ;
    void operator()(ast::While& while_) const ;
    void operator()(ast::Swap&) const ;
    void operator()(ast::If& if_) const ;
    void operator()(ast::VariableDeclaration& declaration) const ;
    void operator()(ast::ArrayDeclaration&) const ;
    void operator()(ast::Assignment& assign) const ;
    void operator()(ast::Return& return_) const ;
    void operator()(ast::ArrayAssignment& assign) const ;
    void operator()(ast::Litteral&) const ;
    void operator()(ast::FunctionCall&) const ;
    void operator()(ast::Integer& integer) const ;
    void operator()(ast::VariableValue&) const ;
    void operator()(ast::ArrayValue&) const ;
    void operator()(ast::ComposedValue& value) const ;
};

} //end of eddic

#endif
