//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_GLOBAL_OPTIMIZATIONS_H
#define MTAC_GLOBAL_OPTIMIZATIONS_H

#include <memory>
#include <iostream>
#include <type_traits>

#include "assert.hpp"
#include "logging.hpp"

#include "mtac/Program.hpp"
#include "mtac/DataFlowProblem.hpp"

namespace eddic {

namespace mtac {

template<typename Left, typename Right>
inline void assign(Left& old, Right&& value, bool& changes){
    if(old.top()){
        changes = !value.top();
    } else if(old.values().size() != value.values().size()){
        changes = true;
    }

    old = value;
}

template<typename P, typename O, typename I, typename OS, typename IS, typename Statements>
inline void forward_statements(P& problem, O& OUT, I& IN, OS& OUT_S, IS& IN_S, Statements& statements, mtac::basic_block_p& B, bool& changes){
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

template<bool Low, typename P, typename R>
inline typename std::enable_if<Low, void>::type forward_statements(P& problem, R& results, mtac::basic_block_p& B, bool& changes){
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_LS = results->OUT_LS;
    auto& IN_LS = results->IN_LS;

    forward_statements(problem, OUT, IN, OUT_LS, IN_LS, B->l_statements, B, changes);
}

template<bool Low, typename P, typename R>
inline typename std::enable_if<!Low, void>::type forward_statements(P& problem, R& results, mtac::basic_block_p& B, bool& changes){
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_S = results->OUT_S;
    auto& IN_S = results->IN_S;

    forward_statements(problem, OUT, IN, OUT_S, IN_S, B->statements, B, changes);
}

template<bool Low, DataFlowType Type, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> forward_data_flow(mtac::function_p function, DataFlowProblem<Type, DomainValues>& problem){
    typedef mtac::Domain<DomainValues> Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;

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

                forward_statements<Low>(problem, results, B, changes);
            }
        }
    }

    return results;
}

template<typename P, typename O, typename I, typename OS, typename IS, typename Statements>
inline void backward_statements(P& problem, O& OUT, I& IN, OS& OUT_S, IS& IN_S, Statements& statements, mtac::basic_block_p& B, bool& changes){
    if(statements.size() > 0){
        log::emit<Dev>("Data-Flow") << "OUT_S[" << (statements.size() - 1) << "] before transfer " << OUT_S[statements[statements.size() - 1]] << log::endl;
        assign(OUT_S[statements.back()], OUT[B], changes);
        log::emit<Dev>("Data-Flow") << "OUT_S[" << (statements.size() - 1) << "] after  transfer " << OUT_S[statements[statements.size() - 1]] << log::endl;

        for(unsigned i = statements.size() - 1; i > 0; --i){
            auto& statement = statements[i];

            log::emit<Dev>("Data-Flow") << "IN_S[" << i << "] before transfer " << IN_S[statement] << log::endl;
            assign(IN_S[statement], problem.transfer(B, statement, OUT_S[statement]), changes);
            log::emit<Dev>("Data-Flow") << "IN_S[" << i << "] after  transfer " << IN_S[statement] << log::endl;

            log::emit<Dev>("Data-Flow") << "OUT_S[" << (i - 1) << "] before transfer " << OUT_S[statements[i - 1]] << log::endl;
            OUT_S[statements[i-1]] = IN_S[statement];
            log::emit<Dev>("Data-Flow") << "OUT_S[" << (i - 1) << "] after  transfer " << OUT_S[statements[i - 1]] << log::endl;
        }

        log::emit<Dev>("Data-Flow") << "IN_S[" << 0 << "] before transfer " << IN_S[statements[0]] << log::endl;
        assign(IN_S[statements[0]], problem.transfer(B, statements[0], OUT_S[statements[0]]), changes);
        log::emit<Dev>("Data-Flow") << "IN_S[" << 0 << "] after  transfer " << IN_S[statements[0]] << log::endl;

        assign(IN[B], IN_S[statements.front()], changes);
    } else {
        //If the basic block is empty, the IN values are the OUT values
        assign(IN[B], OUT[B], changes);
    }
}

template<bool Low, typename P, typename R>
inline typename std::enable_if<Low, void>::type backward_statements(P& problem, R& results, mtac::basic_block_p& B, bool& changes){
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_LS = results->OUT_LS;
    auto& IN_LS = results->IN_LS;

    backward_statements(problem, OUT, IN, OUT_LS, IN_LS, B->l_statements, B, changes);
}

template<bool Low, typename P, typename R>
inline typename std::enable_if<!Low, void>::type backward_statements(P& problem, R& results, mtac::basic_block_p& B, bool& changes){
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_S = results->OUT_S;
    auto& IN_S = results->IN_S;

    backward_statements(problem, OUT, IN, OUT_S, IN_S, B->statements, B, changes);
}

template<bool Low, DataFlowType Type, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> backward_data_flow(mtac::function_p function, DataFlowProblem<Type, DomainValues>& problem){
    typedef mtac::Domain<DomainValues> Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;

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
                log::emit<Dev>("Data-Flow") << "IN[S]  before " << IN[S] << log::endl;

                OUT[B] = problem.meet(OUT[B], IN[S]);
                
                log::emit<Dev>("Data-Flow") << "OUT[B]  after " << OUT[B] << log::endl;

                backward_statements<Low>(problem, results, B, changes);
                
                log::emit<Dev>("Data-Flow") << "IN[B]   after " << IN[B] << log::endl;
            }
        }
    }

    return results;
}

template<DataFlowType Type, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> data_flow(mtac::function_p function, DataFlowProblem<Type, DomainValues>& problem){
    if(Type == DataFlowType::Forward){
        return forward_data_flow<false>(function, problem);
    } else if(Type == DataFlowType::Backward){
        return backward_data_flow<false>(function, problem);
    } else if(Type == DataFlowType::Low_Forward){
        return forward_data_flow<true>(function, problem);
    } else if(Type == DataFlowType::Low_Backward){
        return backward_data_flow<true>(function, problem);
    } else {
        ASSERT_PATH_NOT_TAKEN("This data-flow type is not handled");
    }
}

} //end of mtac

} //end of eddic

#endif
