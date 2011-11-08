//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "OptimizationEngine.hpp"

#include <boost/variant/static_visitor.hpp>

#include "Types.hpp"
#include "Options.hpp"

#include "IsConstantVisitor.hpp"
#include "GetTypeVisitor.hpp"
#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

using namespace eddic;

struct GetIntValue : public boost::static_visitor<int> {
    int operator()(ASTComposedValue& value) const {
        int acc = boost::apply_visitor(*this, value.Content->first);

        for(auto& operation : value.Content->operations){
            int v = boost::apply_visitor(*this, operation.get<1>());
            char op = operation.get<0>();

            switch(op){
                case '+':
                    acc += v;
                    break;
                case '-':
                    acc -= v;
                    break;
                case '*':
                    acc *= v;
                    break;
                case '/':
                    acc /= v;
                    break;
                case '%':
                    acc %= v;
                    break;
            }
        }

        return acc;
    }

    int operator()(ASTInteger& integer) const {
        return integer.value; 
    }
    
    int operator()(ASTVariable&) const {
        assert(false); //A variable is not constant

        return -1; 
    }
    
    int operator()(ASTLitteral&) const {
        assert(false); //A litteral is not integer

        return -1;
    }
};

struct ValueOptimizer : public boost::static_visitor<ASTValue> {
    ASTValue operator()(ASTComposedValue& value) const {
        if(value.Content->operations.empty()){
            return boost::apply_visitor(*this, value.Content->first);   
        }

        //If the value is constant, we can replace it with the results of the computation
        if(IsConstantVisitor()(value)){
            Type type = GetTypeVisitor()(value);

            if(type == Type::INT){
                if (options.count("optimize-integers")) {
                    ASTInteger integer;
                    integer.value = GetIntValue()(value);
                    return integer; 
                }
            } else if(type == Type::STRING){
                if (options.count("optimize-strings")) {
                    //No optimizations for now
                }
            }
        }

        //Optimize the first value
        value.Content->first = boost::apply_visitor(*this, value.Content->first);

        //We can try to optimize every part of the composed value
        auto start = value.Content->operations.begin();
        auto end = value.Content->operations.end();

        while(start != end){
            start->get<1>() = boost::apply_visitor(*this, start->get<1>());
            
            ++start;
        }

        assert(value.Content->operations.size() > 0); //Once here, there is no more empty composed value 

        //If we get there, that means that no optimization has been (or can be) performed
        return value;
    }

    ASTValue operator()(ASTVariable& variable) const {
        return variable; //A variable is not optimizable
    }
    
    ASTValue operator()(ASTInteger& integer) const {
        return integer; //A variable is not optimizable
    }
    
    ASTValue operator()(ASTLitteral& litteral) const {
        return litteral; //A variable is not optimizable
    }
};

struct OptimizationVisitor : public boost::static_visitor<> {
    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_DECLARATION()  
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_FOREACH()

    void operator()(GlobalVariableDeclaration&){
        //As the constantness of the value of a global variable is enforced, there is no need to optimize it
    }

    void operator()(ASTFunctionCall& functionCall){
        auto start = functionCall.Content->values.begin();
        auto end = functionCall.Content->values.end();

        ValueOptimizer optimizer;

        while(start != end){
            *start = boost::apply_visitor(optimizer, *start);
            
            ++start;
        }
    }

    void operator()(ASTAssignment& assignment){
        assignment.Content->value = boost::apply_visitor(ValueOptimizer(), assignment.Content->value); 
    }

    void operator()(ASTDeclaration& declaration){
        declaration.Content->value = boost::apply_visitor(ValueOptimizer(), declaration.Content->value); 
    }

    void operator()(ASTSwap&){
        //Nothing to optimize in a swap
    }

    void operator()(ASTBinaryCondition& binaryCondition){
        binaryCondition.Content->lhs = boost::apply_visitor(ValueOptimizer(), binaryCondition.Content->lhs); 
        binaryCondition.Content->rhs = boost::apply_visitor(ValueOptimizer(), binaryCondition.Content->rhs); 
    }

    void operator()(ASTFalse&){
        //Nothing to optimize
    }

    void operator()(ASTTrue&){
        //Nothing to optimize
    }
};

void OptimizationEngine::optimize(ASTProgram& program){
    OptimizationVisitor visitor;
    visitor(program);
}
