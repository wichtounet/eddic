//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_DEBUG_VISITOR_H
#define AST_DEBUG_VISITOR_H

#include <boost/variant/static_visitor.hpp>

#include "ast/SourceFile.hpp"

namespace eddic {

namespace ast {

/*!
 * \class DebugVisitor
 * \brief AST Debugger
 *  
 * This static visitor browse and print the entire AST to the console. 
 */
struct DebugVisitor : public boost::static_visitor<> {
    mutable int level;

    DebugVisitor();

    std::string indent() const ;

    void operator()(SourceFile& program) const ;
    void operator()(Import& import) const ;
    void operator()(StandardImport& import) const ;
    void operator()(FunctionDeclaration& declaration) const ;
    void operator()(GlobalVariableDeclaration&) const ;
    void operator()(GlobalArrayDeclaration&) const ;
    void operator()(Struct&) const ;
    void operator()(MemberDeclaration&) const ;
    void operator()(For& for_) const ;
    void operator()(Foreach& for_) const ;
    void operator()(ForeachIn& for_) const ;
    void operator()(While& while_) const ;
    void operator()(DoWhile& while_) const ;
    void operator()(Swap&) const ;
    void operator()(If& if_) const ;
    void operator()(VariableDeclaration& declaration) const ;
    void operator()(ArrayDeclaration&) const ;
    void operator()(Assignment& assign) const ;
    void operator()(Return& return_) const ;
    void operator()(SuffixOperation& operation_) const ;
    void operator()(PrefixOperation& operation_) const ;
    void operator()(Litteral&) const ;
    void operator()(FunctionCall&) const ;
    void operator()(BuiltinOperator&) const ;
    void operator()(Integer& integer) const ;
    void operator()(IntegerSuffix& integer) const ;
    void operator()(Float& float_) const ;
    void operator()(VariableValue&) const ;
    void operator()(DereferenceValue&) const ;
    void operator()(ArrayValue&) const ;
    void operator()(Expression& value) const ;
    void operator()(Cast& cast) const ;
    void operator()(Plus& value) const ;
    void operator()(Minus& value) const ;
    void operator()(Null& value) const ;
    void operator()(False& value) const ;
    void operator()(True& value) const ;
    void operator()(Ternary& value) const ;
};

} //end of ast

} //end of eddic

#endif
