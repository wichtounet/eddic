//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BINARY_OPERATOR_H
#define BINARY_OPERATOR_H

#include "Value.hpp"

namespace eddic {

class BinaryOperator : public Value {
    protected:
        std::shared_ptr<Value> lhs;
        std::shared_ptr<Value> rhs;

        BinaryOperator(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> l, std::shared_ptr<Value> r) : Value(context, token), lhs(l), rhs(r) {}

        bool isConstant();
        void checkVariables();
        void checkStrings(StringPool& pool);
        std::string getStringValue();
        int getIntValue();
        void optimize();

        virtual Type checkTypes(Type left, Type right);
        virtual std::string compute(const std::string& left, const std::string& right);
        virtual int compute(int left, int right);
};

} //end of eddic

#endif
