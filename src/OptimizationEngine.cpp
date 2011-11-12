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

#include "StringPool.hpp"
#include "FunctionTable.hpp"

#include "IsConstantVisitor.hpp"
#include "GetTypeVisitor.hpp"
#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"
#include "Variable.hpp"

using namespace eddic;

struct GetIntValue : public boost::static_visitor<int> {
    int operator()(ast::ComposedValue& value) const {
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

    int operator()(ast::Integer& integer) const {
        return integer.value; 
    }
    
    int operator()(ast::VariableValue&) const {
        assert(false); //A variable is not constant

        return -1; 
    }
    
    int operator()(ast::Litteral&) const {
        assert(false); //A litteral is not integer

        return -1;
    }
};

struct GetStringValue : public boost::static_visitor<std::string> {
    std::string operator()(ast::ComposedValue& value) const {
        std::string acc = boost::apply_visitor(*this, value.Content->first);

        for(auto& operation : value.Content->operations){
            std::string v = boost::apply_visitor(*this, operation.get<1>());
            acc = acc.substr(0, acc.size() - 1).append(v.substr(1, acc.size() - 1));
        }

        return acc;
    }

    std::string operator()(ast::Integer&) const {
        assert(false); //An integer is not a string

        return "";
    }

    std::string operator()(ast::VariableValue&) const {
        assert(false); //A variable is not constant

        return "";
    }

    std::string operator()(ast::Litteral& litteral) const {
        return litteral.value;
    }
};

struct ValueOptimizer : public boost::static_visitor<ast::Value> {
    private:
        StringPool& pool;

    public:
        ValueOptimizer(StringPool& p) : pool(p) {}

        ast::Value operator()(ast::ComposedValue& value) const {
            if(value.Content->operations.empty()){
                return boost::apply_visitor(*this, value.Content->first);   
            }

            //If the value is constant, we can replace it with the results of the computation
            if(IsConstantVisitor()(value)){
                Type type = GetTypeVisitor()(value);

                if(type == Type::INT){
                    if (OptimizeIntegers) {
                        ast::Integer integer;
                        integer.value = GetIntValue()(value);
                        return integer; 
                    }
                } else if(type == Type::STRING){
                    if (OptimizeStrings) {
                        ast::Litteral litteral;
                        litteral.value = GetStringValue()(value);
                        litteral.label = pool.label(litteral.value);
                        return litteral;
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

        ast::Value operator()(ast::VariableValue& variable) const {
            return variable; //A variable is not optimizable
        }

        ast::Value operator()(ast::Integer& integer) const {
            return integer; //A variable is not optimizable
        }

        ast::Value operator()(ast::Litteral& litteral) const {
            return litteral; //A variable is not optimizable
        }
};

struct CanBeRemoved : public boost::static_visitor<bool> {
    private:
        FunctionTable& functionTable;

    public:
        CanBeRemoved(FunctionTable& table) : functionTable(table) {}

        bool operator()(ast::FirstLevelBlock block){
            return boost::apply_visitor(*this, block);
        }

        bool operator()(ast::GlobalVariableDeclaration& declaration){
            if(OptimizeUnused){
                if(declaration.Content->context->getVariable(declaration.Content->variableName)->referenceCount() <= 0){
                    //Removing from the AST is not enough, because it is stored in the context now
                    declaration.Content->context->removeVariable(declaration.Content->variableName);
                    
                    return true;   
                }
            }

            return false;
        }

        bool operator()(ast::FunctionDeclaration& declaration){
            if(OptimizeUnused){
                if(declaration.Content->functionName != "main" && functionTable.referenceCount(declaration.Content->mangledName) <= 0){
                    return true;
                }
            }

            return false;
        }
};

struct OptimizationVisitor : public boost::static_visitor<> {
    private:
        FunctionTable& functionTable;
        StringPool& pool;
        ValueOptimizer optimizer;

    public:
        OptimizationVisitor(FunctionTable& t, StringPool& p) : functionTable(t), pool(p), optimizer(ValueOptimizer(pool)) {}

        AUTO_RECURSE_FUNCTION_DECLARATION()  
        AUTO_RECURSE_BRANCHES()
        AUTO_RECURSE_SIMPLE_LOOPS()
        AUTO_RECURSE_FOREACH()

        void operator()(ast::Program& program){\
            auto iter = program.Content->blocks.begin();
            auto end = program.Content->blocks.end();

            CanBeRemoved visitor(functionTable);
            auto newEnd = remove_if(iter, end, visitor);

            program.Content->blocks.erase(newEnd, end);

            visit_each(*this, program.Content->blocks);
        }

        void operator()(ast::GlobalVariableDeclaration&){
            //As the constantness of the value of a global variable is enforced, there is no need to optimize it
        }

        void operator()(ast::FunctionCall& functionCall){
            auto start = functionCall.Content->values.begin();
            auto end = functionCall.Content->values.end();

            while(start != end){
                *start = boost::apply_visitor(optimizer, *start);
                
                ++start;
            }
        }

        void operator()(ast::Assignment& assignment){
            assignment.Content->value = boost::apply_visitor(optimizer, assignment.Content->value); 
        }

        void operator()(ast::Declaration& declaration){
            declaration.Content->value = boost::apply_visitor(optimizer, declaration.Content->value); 
        }

        void operator()(ast::Swap&){
            //Nothing to optimize in a swap
        }

        void operator()(ast::BinaryCondition& binaryCondition){
            binaryCondition.Content->lhs = boost::apply_visitor(optimizer, binaryCondition.Content->lhs); 
            binaryCondition.Content->rhs = boost::apply_visitor(optimizer, binaryCondition.Content->rhs); 
        }

        void operator()(ast::False&){
            //Nothing to optimize
        }

        void operator()(ast::True&){
            //Nothing to optimize
        }
};

void OptimizationEngine::optimize(ast::Program& program, FunctionTable& functionTable, StringPool& pool){
    OptimizationVisitor visitor(functionTable, pool);
    visitor(program);
}
