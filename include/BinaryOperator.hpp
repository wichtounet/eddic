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

        BinaryOperator(std::shared_ptr<Context> context, const Tok token, std::shared_ptr<Value> l, std::shared_ptr<Value> r);

        void checkVariables();
        void checkStrings(StringPool& pool);
        void optimize();

        bool isConstant();
        std::string getStringValue();
        int getIntValue();

        virtual Type checkTypes(Type left, Type right);
        virtual std::string compute(const std::string& left, const std::string& right);
        virtual int compute(int left, int right);
        
        virtual void replace(std::shared_ptr<ParseNode> old, std::shared_ptr<ParseNode> node);
};

} //end of eddic

#endif
