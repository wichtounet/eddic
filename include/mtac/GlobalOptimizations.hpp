//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_GLOBAL_OPTIMIZATIONS_H
#define MTAC_GLOBAL_OPTIMIZATIONS_H

#define BOOST_NO_RTTI
#define BOOST_NO_TYPEID
#include <boost/range/adaptors.hpp>

#include <memory>
#include <ostream>
#include <type_traits>

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

template<bool Low>
inline typename std::enable_if<Low, std::vector<ltac::Instruction>&>::type get_statements(mtac::basic_block_p& B){
    return B->l_statements;
}

template<bool Low>
inline typename std::enable_if<!Low, std::vector<mtac::Quadruple>&>::type get_statements(mtac::basic_block_p& B){
    return B->statements;
}

//Forward

template<bool Low, typename Problem>
std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>> forward_data_flow(mtac::Function& function, Problem& problem){
    typedef typename Problem::ProblemDomain Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;

    auto& OUT_S = results->OUT_S;
    auto& IN_S = results->IN_S;

    OUT[function.entry_bb()] = problem.Boundary(function);
    LOG<Dev>("Data-Flow") << "OUT[" << *function.entry_bb() << "] set to " << OUT[function.entry_bb()] << log::endl;

    for(auto& block : function){
        //Initialize all but ENTRY
        if(block->index != -1){
            OUT[block] = problem.Init(function);
            LOG<Dev>("Data-Flow") << "OUT[" << *block << "] set to " << OUT[block] << log::endl;
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

            auto& statements = get_statements<Low>(B);

            for(auto& P : B->predecessors){
                LOG<Dev>("Data-Flow") << "Meet B = " << *B << " with P = " << *P << log::endl;
                LOG<Dev>("Data-Flow") << "IN[B] before " << IN[B] << log::endl;
                LOG<Dev>("Data-Flow") << "OUT[P] before " << OUT[P] << log::endl;

                problem.meet(IN[B], OUT[P]);
                
                LOG<Dev>("Data-Flow") << "IN[B] after " << IN[B] << log::endl;

                if(statements.size() > 0){
                    IN_S[statements.front().uid()] = IN[B];

                    for(unsigned i = 0; i < statements.size(); ++i){
                        auto& statement = statements[i];

                        assign(OUT_S[statement.uid()], problem.transfer(B, statement, IN_S[statement.uid()]), changes);

                        //The entry value of the next statement are the exit values of the current statement
                        if(i != statements.size() - 1){
                            IN_S[statements[i+1].uid()] = OUT_S[statement.uid()];
                        }
                    }

                    assign(OUT[B], OUT_S[statements.back().uid()], changes);
                } else {
                    //If the basic block is empty, the OUT values are the IN values
                    assign(OUT[B], IN[B], changes);
                }
            }
        }
    }

    return results;
}

//Backward

template<bool Low, typename Problem>
std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>> backward_data_flow(mtac::Function& function, Problem& problem){
    typedef typename Problem::ProblemDomain Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_S = results->OUT_S;
    auto& IN_S = results->IN_S;

    IN[function.exit_bb()] = problem.Boundary(function);
    LOG<Dev>("Data-Flow") << "IN[" << *function.exit_bb() << "] set to " << IN[function.exit_bb()] << log::endl;
    
    for(auto& block : function){
        //Init all but EXIT
        if(block->index != -2){
            IN[block] = problem.Init(function);
            LOG<Dev>("Data-Flow") << "IN[" << *block << "] set to " << IN[block] << log::endl;
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

            auto& statements = get_statements<Low>(B);

            for(auto& S : B->successors){
                LOG<Dev>("Data-Flow") << "Meet B = " << *B << " with S = " << *S << log::endl;
                LOG<Dev>("Data-Flow") << "OUT[B] before " << OUT[B] << log::endl;
                LOG<Dev>("Data-Flow") << "IN[S]  before " << IN[S] << log::endl;

                problem.meet(OUT[B], IN[S]);
                
                LOG<Dev>("Data-Flow") << "OUT[B]  after " << OUT[B] << log::endl;

                if(statements.size() > 0){
                    LOG<Dev>("Data-Flow") << "OUT_S[" << (statements.size() - 1) << "] before transfer " << OUT_S[statements[statements.size() - 1].uid()] << log::endl;
                    assign(OUT_S[statements.back().uid()], OUT[B], changes);
                    LOG<Dev>("Data-Flow") << "OUT_S[" << (statements.size() - 1) << "] after  transfer " << OUT_S[statements[statements.size() - 1].uid()] << log::endl;

                    for(unsigned i = statements.size() - 1; i > 0; --i){
                        auto& statement = statements[i];

                        LOG<Dev>("Data-Flow") << "IN_S[" << i << "] before transfer " << IN_S[statement.uid()] << log::endl;
                        assign(IN_S[statement.uid()], problem.transfer(B, statement, OUT_S[statement.uid()]), changes);
                        LOG<Dev>("Data-Flow") << "IN_S[" << i << "] after  transfer " << IN_S[statement.uid()] << log::endl;

                        LOG<Dev>("Data-Flow") << "OUT_S[" << (i - 1) << "] before transfer " << OUT_S[statements[i - 1].uid()] << log::endl;
                        OUT_S[statements[i-1].uid()] = IN_S[statement.uid()];
                        LOG<Dev>("Data-Flow") << "OUT_S[" << (i - 1) << "] after  transfer " << OUT_S[statements[i - 1].uid()] << log::endl;
                    }

                    LOG<Dev>("Data-Flow") << "IN_S[" << 0 << "] before transfer " << IN_S[statements[0].uid()] << log::endl;
                    assign(IN_S[statements[0].uid()], problem.transfer(B, statements[0], OUT_S[statements[0].uid()]), changes);
                    LOG<Dev>("Data-Flow") << "IN_S[" << 0 << "] after  transfer " << IN_S[statements[0].uid()] << log::endl;

                    assign(IN[B], IN_S[statements.front().uid()], changes);
                } else {
                    //If the basic block is empty, the IN values are the OUT values
                    assign(IN[B], OUT[B], changes);
                }
                
                LOG<Dev>("Data-Flow") << "IN[B]   after " << IN[B] << log::endl;
            }
        }
    }

    return results;
}

//Fast forward statements

template<bool Low, typename Problem>
std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>> fast_forward_data_flow(mtac::Function& function, Problem& problem){
    typedef typename Problem::ProblemDomain Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;

    OUT[function.entry_bb()] = problem.Boundary(function);
    LOG<Dev>("Data-Flow") << "OUT[" << *function.entry_bb() << "] set to " << OUT[function.entry_bb()] << log::endl;

    for(auto& block : function){
        //Initialize all but ENTRY
        if(block->index != -1){
            OUT[block] = problem.Init(function);
            LOG<Dev>("Data-Flow") << "OUT[" << *block << "] set to " << OUT[block] << log::endl;
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
                LOG<Dev>("Data-Flow") << "Meet B = " << *B << " with P = " << *P << log::endl;
                LOG<Dev>("Data-Flow") << "IN[B] before " << IN[B] << log::endl;
                LOG<Dev>("Data-Flow") << "OUT[P] before " << OUT[P] << log::endl;

                problem.meet(IN[B], OUT[P]);
                
                LOG<Dev>("Data-Flow") << "IN[B] after " << IN[B] << log::endl;

                auto in = IN[B];

                for(auto& statement : get_statements<Low>(B)){
                    problem.transfer(B, statement, in);
                }

                if(!changes && OUT[B] != in){
                    changes = true;
                }

                OUT[B] = std::move(in);
            }
        }
    }

    return results;
}

//Fast forward block

template<typename Problem>
std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>> fast_forward_data_flow_block(mtac::Function& function, Problem& problem){
    typedef typename Problem::ProblemDomain Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;

    OUT[function.entry_bb()] = problem.Boundary(function);
    LOG<Dev>("Data-Flow") << "OUT[" << *function.entry_bb() << "] set to " << OUT[function.entry_bb()] << log::endl;

    for(auto& block : function){
        //Initialize all but ENTRY
        if(block->index != -1){
            OUT[block] = problem.Init(function);
            LOG<Dev>("Data-Flow") << "OUT[" << *block << "] set to " << OUT[block] << log::endl;
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
                LOG<Dev>("Data-Flow") << "Meet B = " << *B << " with P = " << *P << log::endl;
                LOG<Dev>("Data-Flow") << "IN[B] before " << IN[B] << log::endl;
                LOG<Dev>("Data-Flow") << "OUT[P] before " << OUT[P] << log::endl;

                problem.meet(IN[B], OUT[P]);
                
                LOG<Dev>("Data-Flow") << "IN[B] after " << IN[B] << log::endl;

                auto in = IN[B];

                problem.transfer(B, in);

                if(!changes && OUT[B] != in){
                    changes = true;
                }

                OUT[B] = std::move(in);
            }
        }
    }

    return results;
}

//Fast Backward statements

template<bool Low, typename Problem>
std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>> fast_backward_data_flow(mtac::Function& function, Problem& problem){
    typedef typename Problem::ProblemDomain Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;

    IN[function.exit_bb()] = problem.Boundary(function);
    LOG<Dev>("Data-Flow") << "IN[" << *function.exit_bb() << "] set to " << IN[function.exit_bb()] << log::endl;
    
    for(auto& block : function){
        //Init all but EXIT
        if(block->index != -2){
            IN[block] = problem.Init(function);
            LOG<Dev>("Data-Flow") << "IN[" << *block << "] set to " << IN[block] << log::endl;
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

            auto& statements = get_statements<Low>(B);

            for(auto& S : B->successors){
                LOG<Dev>("Data-Flow") << "Meet B = " << *B << " with S = " << *S << log::endl;
                LOG<Dev>("Data-Flow") << "OUT[B] before " << OUT[B] << log::endl;
                LOG<Dev>("Data-Flow") << "IN[S]  before " << IN[S] << log::endl;

                problem.meet(OUT[B], IN[S]);
                
                LOG<Dev>("Data-Flow") << "OUT[B]  after " << OUT[B] << log::endl;

                auto out = OUT[B];

                for(auto& statement : boost::adaptors::reverse(get_statements<Low>(B))){
                    problem.transfer(B, statement, out);
                }

                if(!changes && IN[B] != out){
                    changes = true;
                }

                IN[B] = std::move(out);
            }
        }
    }

    return results;
}

//Fast Backward block

template<typename Problem>
std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>> fast_backward_data_flow_block(mtac::Function& function, Problem& problem){
    typedef typename Problem::ProblemDomain Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;

    IN[function.exit_bb()] = problem.Boundary(function);
    LOG<Dev>("Data-Flow") << "IN[" << *function.exit_bb() << "] set to " << IN[function.exit_bb()] << log::endl;
    
    for(auto& block : function){
        //Init all but EXIT
        if(block->index != -2){
            IN[block] = problem.Init(function);
            LOG<Dev>("Data-Flow") << "IN[" << *block << "] set to " << IN[block] << log::endl;
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
                LOG<Dev>("Data-Flow") << "Meet B = " << *B << " with S = " << *S << log::endl;
                LOG<Dev>("Data-Flow") << "OUT[B] before " << OUT[B] << log::endl;
                LOG<Dev>("Data-Flow") << "IN[S]  before " << IN[S] << log::endl;

                problem.meet(OUT[B], IN[S]);
                
                LOG<Dev>("Data-Flow") << "OUT[B]  after " << OUT[B] << log::endl;

                auto out = OUT[B];

                problem.transfer(B, out);

                if(!changes && IN[B] != out){
                    changes = true;
                }

                IN[B] = std::move(out);
            }
        }
    }

    return results;
}

template<typename Problem>
typename std::enable_if<Problem::Type == DataFlowType::Forward, std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>>>::type 
data_flow(mtac::Function& function, Problem& problem){
    return forward_data_flow<Problem::Low>(function, problem);
}

template<typename Problem>
typename std::enable_if<Problem::Type == DataFlowType::Backward, std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>>>::type 
data_flow(mtac::Function& function, Problem& problem){
    return backward_data_flow<Problem::Low>(function, problem);
}

template<typename Problem>
typename std::enable_if<Problem::Type == DataFlowType::Fast_Forward, std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>>>::type 
data_flow(mtac::Function& function, Problem& problem){
    return fast_forward_data_flow<Problem::Low>(function, problem);
}

template<typename Problem>
typename std::enable_if<Problem::Type == DataFlowType::Fast_Forward_Block, std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>>>::type 
data_flow(mtac::Function& function, Problem& problem){
    return fast_forward_data_flow_block(function, problem);
}

template<typename Problem>
typename std::enable_if<Problem::Type == DataFlowType::Fast_Backward, std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>>>::type 
data_flow(mtac::Function& function, Problem& problem){
    return fast_backward_data_flow<Problem::Low>(function, problem);
}

template<typename Problem>
typename std::enable_if<Problem::Type == DataFlowType::Fast_Backward_Block, std::shared_ptr<DataFlowResults<typename Problem::ProblemDomain>>>::type 
data_flow(mtac::Function& function, Problem& problem){
    return fast_backward_data_flow_block(function, problem);
}

} //end of mtac

} //end of eddic

#endif
