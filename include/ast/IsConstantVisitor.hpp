//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef IS_CONSTANT_VISITOR_H
#define IS_CONSTANT_VISITOR_H

#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>

#include "variant.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"

#include "ast/Value.hpp"

namespace eddic {

namespace ast {

/*!
 * \struct IsConstantVisitor
 * \brief AST Visitor to test if a node is constant.
 */
struct IsConstantVisitor : public boost::static_visitor<bool> {
    typedef boost::mpl::vector<ast::Integer, ast::Literal, ast::CharLiteral, ast::IntegerSuffix, ast::Float, ast::Boolean, ast::Null> constant_types;
    typedef boost::mpl::vector<ast::FunctionCall,
        ast::BuiltinOperator, ast::Assignment, ast::Ternary, ast::New, ast::NewArray> non_constant_types;

    template<typename T>
    typename std::enable_if<boost::mpl::contains<constant_types, T>::value, bool>::type operator()(T&) const {
        return true;
    }

    template<typename T>
    typename std::enable_if<boost::mpl::contains<non_constant_types, T>::value, bool>::type operator()(T&) const {
        return false;
    }

    bool operator()(ast::PrefixOperation& value) const {
        auto op = value.op;

        if(
                    op == ast::Operator::STAR || op == ast::Operator::ADDRESS || op == ast::Operator::CALL
                ||  op == ast::Operator::DOT || op == ast::Operator::INC || op == ast::Operator::DEC){
            return false;
        }

        return visit(*this, value.left_value);
    }

    bool operator()(ast::Cast& cast) const {
        return visit(*this, cast.value);
    }

    bool operator()(ast::VariableValue& variable) const {
        return variable.var->type()->is_const();
    }

    bool operator()(ast::Expression& value) const {
        if(visit(*this, value.first)){
            for(auto& op : value.operations){
                if(ast::has_operation_value(op)){
                    if(auto* ptr = boost::smart_get<ast::FunctionCall>(&op.get<1>())){
                        for(auto& v : ptr->values){
                            if(!visit(*this, v)){
                                return false;
                            }
                        }
                    } else {
                        if(!visit(*this, op.get<1>())){
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        return false;
    }

    template<typename T>
    bool operator()(x3::forward_ast<T>& value) const {
        return (*this)(value.get());
    }
};

} //end of ast

} //end of eddic

#endif
