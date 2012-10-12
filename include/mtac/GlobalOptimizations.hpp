//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_GLOBAL_OPTIMIZATIONS_H
#define MTAC_GLOBAL_OPTIMIZATIONS_H

#include <memory>
#include <iostream>

#include "assert.hpp"
#include "logging.hpp"

#include "mtac/Program.hpp"
#include "mtac/DataFlowProblem.hpp"

namespace eddic {

namespace mtac {

template<DataFlowType Type, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> data_flow(std::shared_ptr<mtac::Function> function, DataFlowProblem<Type, DomainValues>& problem){
    if(Type == DataFlowType::Forward){
        return forward_data_flow(function, problem);
    } else if(Type == DataFlowType::Backward){
        return backward_data_flow(function, problem);
    } else {
        ASSERT_PATH_NOT_TAKEN("This data-flow type is not handled");
    }
}

template<typename Left, typename Right>
inline void assign(Left& old, Right&& value, bool& changes){
    if(old.top()){
        changes = !value.top();
    } else if(old.values().size() != value.values().size()){
        changes = true;
    }

    old = value;
}

template<DataFlowType Type, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> forward_data_flow(std::shared_ptr<mtac::Function> function, DataFlowProblem<Type, DomainValues>& problem){
    typedef mtac::Domain<DomainValues> Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_S = results->OUT_S;
    auto& IN_S = results->IN_S;

    OUT[function->entry_bb()] = problem.Boundary(function);
    log::emit<Dev>("Data-Flow") << "OUT[" << *function->entry_bb() << "] set to " << OUT[function->entry_bb()] << log::endl;

    for(auto& block : function){
        //Initialize all but ENTRY
        if(block->index != -1){
            OUT[block] = problem.Init(function);
            log::emit<Dev>("Data-Flow") << "OUT[" << *block << "] set to " << OUT[block] << log::endl;
        }
    }

    bool changes = true;
    while(changes){
        changes = false;

        for(auto& B : function){
            //Do not consider ENTRY
            if(B->index == -1){
                continue;
            }

            for(auto& P : B->predecessors){
                log::emit<Dev>("Data-Flow") << "Meet B = " << *B << " with P = " << *P << log::endl;
                log::emit<Dev>("Data-Flow") << "IN[B] before " << IN[B] << log::endl;
                log::emit<Dev>("Data-Flow") << "OUT[P] before " << OUT[P] << log::endl;

                IN[B] = problem.meet(IN[B], OUT[P]);
                
                log::emit<Dev>("Data-Flow") << "IN[B] after " << IN[B] << log::endl;

                auto& statements = B->statements;

                if(statements.size() > 0){
                    IN_S[statements.front()] = IN[B];

                    for(unsigned i = 0; i < statements.size(); ++i){
                        auto& statement = statements[i];

                        assign(OUT_S[statement], problem.transfer(B, statement, IN_S[statement]), changes);

                        //The entry value of the next statement are the exit values of the current statement
                        if(i != statements.size() - 1){
                            IN_S[statements[i+1]] = OUT_S[statement];
                        }
                    }
                    
                    assign(OUT[B], OUT_S[statements.back()], changes);
                } else {
                    //If the basic block is empty, the OUT values are the IN values
                    assign(OUT[B], IN[B], changes);
                }
            }
        }
    }

    return results;
}

template<DataFlowType Type, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> backward_data_flow(std::shared_ptr<mtac::Function> function, DataFlowProblem<Type, DomainValues>& problem){
    typedef mtac::Domain<DomainValues> Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_S = results->OUT_S;
    auto& IN_S = results->IN_S;

    IN[function->exit_bb()] = problem.Boundary(function);
    log::emit<Dev>("Data-Flow") << "IN[" << *function->exit_bb() << "] set to " << IN[function->exit_bb()] << log::endl;
    
    for(auto& block : function){
        //Init all but EXIT
        if(block->index != -2){
            IN[block] = problem.Init(function);
            log::emit<Dev>("Data-Flow") << "IN[" << *block << "] set to " << IN[block] << log::endl;
        }
    }

    bool changes = true;
    while(changes){
        changes = false;

        for(auto& B : function){
            //Do not consider EXIT
            if(B->index == -2){
                continue;
            }

            for(auto& S : B->successors){
                log::emit<Dev>("Data-Flow") << "Meet B = " << *B << " with S = " << *S << log::endl;
                log::emit<Dev>("Data-Flow") << "OUT[B] before " << OUT[B] << log::endl;
                log::emit<Dev>("Data-Flow") << "IN[S] before " << IN[S] << log::endl;

                OUT[B] = problem.meet(OUT[B], IN[S]);
                
                log::emit<Dev>("Data-Flow") << "OUT[B] after " << OUT[B] << log::endl;

                auto& statements = B->statements;

                if(statements.size() > 0){
                    assign(OUT_S[statements.back()], OUT[B], changes);

                    for(unsigned i = statements.size() - 1; i > 0; --i){
                        auto& statement = statements[i];

                        log::emit<Dev>("Data-Flow") << "IN_S[" << i << "] before transfer " << IN_S[statement] << log::endl;
                        assign(IN_S[statement], problem.transfer(B, statement, OUT_S[statement]), changes);
                        log::emit<Dev>("Data-Flow") << "IN_S[" << i << "] after transfer " << IN_S[statement] << log::endl;
                            
                        OUT_S[statements[i-1]] = IN_S[statement];
                    }
                        
                    assign(IN_S[statements[0]], problem.transfer(B, statements[0], OUT_S[statements[0]]), changes);
                    
                    assign(IN[B], IN_S[statements.front()], changes);
                } else {
                    //If the basic block is empty, the IN values are the OUT values
                    assign(IN[B], OUT[B], changes);
                }
                
                log::emit<Dev>("Data-Flow") << "IN[B] after transfer " << IN[B] << log::endl;
            }
        }
    }

    return results;
}

} //end of mtac

} //end of eddic

#endif
