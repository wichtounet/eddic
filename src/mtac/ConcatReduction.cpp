//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "StringPool.hpp"
#include "likely.hpp"

#include "mtac/ConcatReduction.hpp"
#include "mtac/Function.hpp"
#include "mtac/Statement.hpp"

using namespace eddic;

template<typename T>
bool isParam(T& statement){
    return boost::get<std::shared_ptr<mtac::Param>>(&statement);
}
    
void mtac::optimize_concat::set_pool(std::shared_ptr<StringPool> pool){
    this->pool = pool;
}

bool mtac::optimize_concat::operator()(std::shared_ptr<mtac::Function> function){
    bool optimized = false;
    
    for(auto& block : function){
        if(likely(!block->statements.empty())){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&block->statements[0])){
                if((*ptr)->function == "_F6concatSS"){
                    //The params are on the previous block
                    auto& paramBlock = block->prev;

                    //Must have at least four params
                    if(paramBlock->statements.size() >= 4){
                        auto size = paramBlock->statements.size();
                        auto& statement1 = paramBlock->statements[size - 4];
                        auto& statement2 = paramBlock->statements[size - 3];
                        auto& statement3 = paramBlock->statements[size - 2];
                        auto& statement4 = paramBlock->statements[size - 1];

                        if(isParam(statement1) && isParam(statement2) && isParam(statement3) && isParam(statement4)){
                            auto& quadruple1 = boost::get<std::shared_ptr<mtac::Param>>(statement1);
                            auto& quadruple3 = boost::get<std::shared_ptr<mtac::Param>>(statement3);

                            if(boost::get<std::string>(&quadruple1->arg) && boost::get<std::string>(&quadruple3->arg)){
                                std::string firstValue = pool->value(boost::get<std::string>(quadruple1->arg));
                                std::string secondValue = pool->value(boost::get<std::string>(quadruple3->arg));

                                //Remove the quotes
                                firstValue.resize(firstValue.size() - 1);
                                secondValue.erase(0, 1);

                                //Compute the result of the concatenation
                                std::string result = firstValue + secondValue;

                                std::string label = pool->label(result);
                                int length = result.length() - 2;

                                auto ret1 = (*ptr)->return_;
                                auto ret2 = (*ptr)->return2_;

                                //remove the call to concat
                                block->statements.erase(block->statements.begin());

                                //Insert assign with the concatenated value 
                                block->statements.insert(block->statements.begin(), std::make_shared<mtac::Quadruple>(ret1, label, mtac::Operator::ASSIGN));
                                block->statements.insert(block->statements.begin()+1, std::make_shared<mtac::Quadruple>(ret2, length, mtac::Operator::ASSIGN));

                                //Remove the four params from the previous basic block
                                paramBlock->statements.erase(paramBlock->statements.end() - 4, paramBlock->statements.end());

                                optimized = true;
                            }
                        }
                    }
                }
            }
        }
    }

    return optimized;
}
