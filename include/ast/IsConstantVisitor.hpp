//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IS_CONSTANT_VISITOR_H
#define IS_CONSTANT_VISITOR_H

#include <type_traits>

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
    typedef boost::mpl::vector<ast::Integer, ast::Literal, ast::CharLiteral, ast::IntegerSuffix, ast::Float, ast::True, ast::False, ast::Null> constant_types;
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
        auto op = value.Content->op;

        if(
                    op == ast::Operator::STAR || op == ast::Operator::ADDRESS || op == ast::Operator::CALL 
                ||  op == ast::Operator::DOT || op == ast::Operator::INC || op == ast::Operator::DEC){
            return false;
        }

        return visit(*this, value.Content->left_value);
    }

    bool operator()(ast::Cast& cast) const {
        return visit(*this, cast.Content->value);
    }

    bool operator()(ast::VariableValue& variable) const {
        return variable.Content->var->type()->is_const();
    }

    bool operator()(ast::Expression& value) const {
        if(visit(*this, value.Content->first)){
            for(auto& op : value.Content->operations){
                if(op.get<1>()){
                    if(auto* ptr = boost::get<ast::Value>(&*op.get<1>())){
                        if(!visit(*this, *ptr)){
                            return false;
                        }
                    } else if(auto* ptr = boost::get<ast::CallOperationValue>(&*op.get<1>())){
                        for(auto& v : ptr->values){
                            if(!visit(*this, v)){
                                return false;
                            }
                        }
                    }
                }
            }

            return true;
        }

        return false;
    }
};

} //end of ast

} //end of eddic

#endif
