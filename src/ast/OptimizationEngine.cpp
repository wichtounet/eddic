//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "variant.hpp"
#include "Type.hpp"
#include "Options.hpp"
#include "StringPool.hpp"
#include "SymbolTable.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"

#include "ast/OptimizationEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/IsConstantVisitor.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/ASTVisitor.hpp"

using namespace eddic;

struct GetStringValue : public boost::static_visitor<std::string> {
    std::string operator()(ast::Expression& value) const {
        std::string acc = visit(*this, value.Content->first);

        for(auto& operation : value.Content->operations){
            std::string v = visit(*this, operation.get<1>());
            acc = acc.substr(0, acc.size() - 1).append(v.substr(1, acc.size() - 1));
        }

        return acc;
    }
    
    std::string operator()(ast::Litteral& litteral) const {
        return litteral.value;
    }

    std::string operator()(ast::VariableValue& variable) const {
        auto type = variable.Content->var->type();
        assert(type->is_const() && type->non_const() == STRING);

        auto value = boost::get<std::pair<std::string, int>>(variable.Content->var->val());
        return value.first;
    }
    
    //Other values are not strings
    AUTO_BREAK_OTHERS_CONST()
};

struct ValueOptimizer : public boost::static_visitor<ast::Value> {
    private:
        StringPool& pool;

    public:
        ValueOptimizer(StringPool& p) : pool(p){}

        ast::Value operator()(ast::Expression& value) const {
            assert(value.Content->operations.size() > 0); //Should have been transformed before

            //If the value is constant, we can replace it with the results of the computation
            if(ast::IsConstantVisitor()(value)){
                auto type = ast::GetTypeVisitor()(value);

                if(type == STRING){
                    ast::Litteral litteral;
                    litteral.value = GetStringValue()(value);
                    litteral.label = pool.label(litteral.value);
                    return litteral;
                }
            }

            //Optimize the first value
            value.Content->first = visit(*this, value.Content->first);

            //We can try to optimize every part of the composed value
            auto start = value.Content->operations.begin();
            auto end = value.Content->operations.end();

            while(start != end){
                start->get<1>() = visit(*this, start->get<1>());

                ++start;
            }

            //If we get there, that means that no optimization has been (or can be) performed
            return value;
        }
        
        ast::Value operator()(ast::ArrayValue& value) const {
            value.Content->indexValue = visit(*this, value.Content->indexValue); 

            return value;
        }

        //Cannot be done in the TAC Optimizer as the string variables are splitted into two parts
        ast::Value operator()(ast::VariableValue& variable) const {
            auto type = variable.Content->var->type();

            if(type->is_const()){
                if(type->non_const() == INT){
                    ast::Integer integer;
                    integer.value = boost::get<int>(variable.Content->var->val());
                    return integer; 
                } else if(type->non_const() == STRING){
                    auto value = boost::get<std::pair<std::string, int>>(variable.Content->var->val());

                    ast::Litteral litteral;
                    litteral.value = value.first;
                    litteral.label = pool.label(litteral.value);

                    return litteral;
                }
            }

            return variable;
        }

        //No optimizations for other kind of values
        AUTO_RETURN_OTHERS_CONST(ast::Value)
};

struct CanBeRemoved : public boost::static_visitor<bool> {
        bool operator()(ast::FirstLevelBlock block){
            return visit(*this, block);
        }

        bool optimizeVariable(std::shared_ptr<Context> context, const std::string& variable){
            if(context->getVariable(variable)->referenceCount() <= 0){
                //Removing from the AST is not enough, because it is stored in the context now
                context->removeVariable(variable);

                return true;   
            }

            return false;
        }

        bool operator()(ast::GlobalVariableDeclaration& declaration){
            return optimizeVariable(declaration.Content->context, declaration.Content->variableName);
        }

        bool operator()(ast::VariableDeclaration& declaration){
            return optimizeVariable(declaration.Content->context, declaration.Content->variableName);
        }
        
        bool operator()(ast::ArrayDeclaration& declaration){
            return optimizeVariable(declaration.Content->context, declaration.Content->arrayName);
        }

        bool operator()(ast::FunctionDeclaration& declaration){
            if(declaration.Content->functionName != "main" && symbols.referenceCount(declaration.Content->mangledName) <= 0){
                return true;
            }

            return false;
        }

        bool operator()(ast::Instruction& instruction){
           return visit(*this, instruction); 
        }

        //Nothing to optimize for the other types
        template<typename T>
        bool operator()(T&) {
            return false;
        }
};

struct OptimizationVisitor : public boost::static_visitor<> {
    private:
        StringPool& pool;
        ValueOptimizer optimizer;

    public:
        OptimizationVisitor(StringPool& p) : pool(p), optimizer(ValueOptimizer(pool)) {}

        template<typename T>
        void removeUnused(std::vector<T>& vector){
            auto iter = vector.begin();
            auto end = vector.end();

            CanBeRemoved visitor;
            auto newEnd = remove_if(iter, end, visitor);

            vector.erase(newEnd, end);

            visit_each(*this, vector);
        }

        void operator()(ast::SourceFile& program){\
            removeUnused(program.Content->blocks);
        }
        
        void operator()(ast::FunctionDeclaration& function){
            removeUnused(function.Content->instructions);
        }

        void operator()(ast::If& if_){
            visit(*this, if_.Content->condition);
            removeUnused(if_.Content->instructions);
            visit_each_non_variant(*this, if_.Content->elseIfs);
            visit_optional_non_variant(*this, if_.Content->else_);
        }

        void operator()(ast::ElseIf& elseIf){
            visit(*this, elseIf.condition);
            removeUnused(elseIf.instructions);
        }

        void operator()(ast::Else& else_){
            removeUnused(else_.instructions);
        }

        void operator()(ast::For& for_){
            visit_optional(*this, for_.Content->start);
            visit_optional(*this, for_.Content->condition);
            visit_optional(*this, for_.Content->repeat);
            removeUnused(for_.Content->instructions);
        }

        void operator()(ast::While& while_){
            visit(*this, while_.Content->condition);
            removeUnused(while_.Content->instructions);
        }

        void operator()(ast::Foreach&){
            ASSERT_PATH_NOT_TAKEN("Foreach should have been tranformed into a For"); 
        }

        void operator()(ast::ForeachIn& foreach){
            removeUnused(foreach.Content->instructions);
        }

        void operator()(ast::FunctionCall& functionCall){
            auto start = functionCall.Content->values.begin();
            auto end = functionCall.Content->values.end();

            while(start != end){
                *start = visit(optimizer, *start);
                
                ++start;
            }
        }

        void operator()(ast::Assignment& assignment){
            assignment.Content->left_value = to_left_value(visit(optimizer, assignment.Content->left_value));
            assignment.Content->value = visit(optimizer, assignment.Content->value); 
        }

        void operator()(ast::Return& return_){
            return_.Content->value = visit(optimizer, return_.Content->value); 
        }

        void operator()(ast::VariableDeclaration& declaration){
            if(declaration.Content->value){
                declaration.Content->value = visit(optimizer, *declaration.Content->value); 
            }
        }

        void operator()(ast::BinaryCondition& binaryCondition){
            binaryCondition.Content->lhs = visit(optimizer, binaryCondition.Content->lhs); 
            binaryCondition.Content->rhs = visit(optimizer, binaryCondition.Content->rhs); 
        }

        AUTO_IGNORE_OTHERS()
};

void ast::optimizeAST(ast::SourceFile& program, StringPool& pool){
    OptimizationVisitor visitor(pool);
    visitor(program);
}
