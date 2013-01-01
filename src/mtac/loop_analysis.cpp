//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <stack>

#include "VisitorUtils.hpp"
#include "logging.hpp"
#include "Variable.hpp"
#include "FunctionContext.hpp"

#include "mtac/loop_analysis.hpp"
#include "mtac/dominators.hpp"
#include "mtac/Loop.hpp"
#include "mtac/Program.hpp"
#include "mtac/Statement.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

namespace {

struct DepthInit : public boost::static_visitor<void> {
    template<typename T>
    void operator()(T t){
        t->depth = 0;
    }
};

struct DepthIncrementer : public boost::static_visitor<void> {
    template<typename T>
    void operator()(T t){
        ++t->depth;
    }
};

void init_depth(mtac::basic_block_p bb){
    DepthInit init;

    bb->depth = 0;
    visit_each(init, bb->statements);
}

void increase_depth(mtac::basic_block_p bb){
    DepthIncrementer incrementer;

    ++bb->depth;
    visit_each(incrementer, bb->statements);
}

mtac::InductionVariables find_all_candidates(std::shared_ptr<mtac::Loop> loop){
    mtac::InductionVariables candidates;

    for(auto& bb : loop){
        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if((*ptr)->op == mtac::Operator::ADD || (*ptr)->op == mtac::Operator::MUL || (*ptr)->op == mtac::Operator::SUB || (*ptr)->op == mtac::Operator::MINUS){
                    candidates[(*ptr)->result] = {*ptr, nullptr, 0, 0, false};
                }
            }
        }
    }

    return candidates;
}

void clean_defaults(mtac::InductionVariables& induction_variables){
    auto it = iterate(induction_variables);

    //Erase induction variables that have been created by default
    while(it.has_next()){
        auto equation = (*it).second;

        if(!equation.i){
            it.erase();
            continue;
        }

        ++it;
    }
}

void find_basic_induction_variables(std::shared_ptr<mtac::Loop> loop){
    loop->basic_induction_variables() = find_all_candidates(loop);

    for(auto& bb : loop){
        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ptr;
                auto var = quadruple->result;

                //If it is not a candidate, do not test it
                if(!loop->basic_induction_variables().count(var)){
                    continue;
                }

                auto value = loop->basic_induction_variables()[var];

                //TODO In the future, induction variables written several times could be splitted into several induction variables
                if(value.i){
                    loop->basic_induction_variables().erase(var);

                    continue;
                }

                if(quadruple->op == mtac::Operator::ADD){
                    auto arg1 = *quadruple->arg1;
                    auto arg2 = *quadruple->arg2;

                    if(mtac::isInt(arg1) && mtac::equals(arg2, var)){
                        loop->basic_induction_variables()[var] = {quadruple, var, 1, boost::get<int>(arg1), false};
                        continue;
                    } else if(mtac::isInt(arg2) && mtac::equals(arg1, var)){
                        loop->basic_induction_variables()[var] = {quadruple, var, 1, boost::get<int>(arg2), false}; 
                        continue;
                    } 
                } 
                
                loop->basic_induction_variables().erase(var);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                auto call = *ptr;

                if(call->return_){
                    loop->basic_induction_variables().erase(call->return_);
                }

                if(call->return2_){
                    loop->basic_induction_variables().erase(call->return2_);
                }
            }
        }
    }

    clean_defaults(loop->basic_induction_variables());
}

void find_dependent_induction_variables(std::shared_ptr<mtac::Loop> loop, mtac::Function& function){
    loop->dependent_induction_variables() = find_all_candidates(loop);

    for(auto& bb : loop){
        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ptr;
                auto var = quadruple->result;

                //If it is not a candidate, do not test it
                if(!loop->dependent_induction_variables().count(var)){
                    continue;
                }
                
                //We know for sure that all the candidates have a first arg
                auto arg1 = *quadruple->arg1;
                
                //If it is a basic induction variable, it is not a dependent induction variable
                if(loop->basic_induction_variables().count(var)){
                    loop->dependent_induction_variables().erase(var);

                    continue;
                }

                auto source_equation = loop->dependent_induction_variables()[var];

                if(source_equation.i && (mtac::equals(arg1, var) || (quadruple->arg2 && mtac::equals(*quadruple->arg2, var))) ){
                    auto tj = function.context->new_temporary(INT);

                    source_equation.def->result = tj;
                    
                    loop->dependent_induction_variables().erase(var);
                    loop->dependent_induction_variables()[tj] = source_equation;

                    if(mtac::equals(arg1, var)){
                        quadruple->arg1 = tj;
                    }

                    if(quadruple->arg2 && mtac::equals(*quadruple->arg2, var)){
                        quadruple->arg2 = tj;
                    }
                
                    arg1 = *quadruple->arg1;
                }

                if(quadruple->op == mtac::Operator::MUL){
                    auto arg2 = *quadruple->arg2;

                    if(mtac::isInt(arg1) && mtac::isVariable(arg2)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg2);
                        auto e = boost::get<int>(arg1);
                        
                        if(variable != var){
                            if(loop->basic_induction_variables().count(variable)){
                                loop->dependent_induction_variables()[var] = {quadruple, variable, e, 0, false}; 
                                continue;
                            } else if(loop->dependent_induction_variables()[variable].i){
                                auto equation = loop->dependent_induction_variables()[variable];
                                loop->dependent_induction_variables()[var] = {quadruple, equation.i, equation.e * e, equation.d * e, false}; 
                                continue;
                            }
                        }
                    } else if(mtac::isInt(arg2) && mtac::isVariable(arg1)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg1);
                        auto e = boost::get<int>(arg2);
                        
                        if(variable != var){
                            if(loop->basic_induction_variables().count(variable)){
                                loop->dependent_induction_variables()[var] = {quadruple, variable, e, 0, false}; 
                                continue;
                            } else if(loop->dependent_induction_variables()[variable].i){
                                auto equation = loop->dependent_induction_variables()[variable];
                                loop->dependent_induction_variables()[var] = {quadruple, equation.i, equation.e * e, equation.d * e, false}; 
                                continue;
                            }
                        }
                    } 
                } else if(quadruple->op == mtac::Operator::ADD){
                    auto arg2 = *quadruple->arg2;

                    if(mtac::isInt(arg1) && mtac::isVariable(arg2)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg2);
                        auto e = boost::get<int>(arg1);

                        if(variable != var){
                            if(loop->basic_induction_variables().count(variable)){
                                loop->dependent_induction_variables()[var] = {quadruple, variable, 1, boost::get<int>(arg1), false}; 
                                continue;
                            } else if(loop->dependent_induction_variables()[variable].i){
                                auto equation = loop->dependent_induction_variables()[variable];
                                loop->dependent_induction_variables()[var] = {quadruple, equation.i, equation.e, equation.d + e, false}; 
                                continue;
                            }
                        }
                    } else if(mtac::isInt(arg2) && mtac::isVariable(arg1)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg1);
                        auto e = boost::get<int>(arg2);

                        if(variable != var){
                            if(loop->basic_induction_variables().count(variable)){
                                loop->dependent_induction_variables()[var] = {quadruple, variable, 1, boost::get<int>(arg2), false}; 
                                continue;
                            } else if(loop->dependent_induction_variables()[variable].i){
                                auto equation = loop->dependent_induction_variables()[variable];
                                loop->dependent_induction_variables()[var] = {quadruple, equation.i, equation.e, equation.d + e, false}; 
                                continue;
                            }
                        }
                    } else if(mtac::isVariable(arg1) && mtac::isVariable(arg2)){
                        auto var1 = boost::get<std::shared_ptr<Variable>>(arg1);
                        auto var2 = boost::get<std::shared_ptr<Variable>>(arg2);

                        if(var1 == var2 && var1 != var){
                            if(loop->basic_induction_variables().count(var1)){
                                loop->dependent_induction_variables()[var] = {quadruple, var1, 2, 0, false}; 
                                continue;
                            } else if(loop->dependent_induction_variables()[var1].i){
                                auto equation = loop->dependent_induction_variables()[var1];
                                loop->dependent_induction_variables()[var] = {quadruple, equation.i, equation.e * 2, equation.d * 2, false}; 
                                continue;
                            }
                        }
                    }
                } else if(quadruple->op == mtac::Operator::SUB){
                    auto arg2 = *quadruple->arg2;

                    if(mtac::isInt(arg1) && mtac::isVariable(arg2)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg2);
                        auto e = boost::get<int>(arg1);

                        if(variable != var){
                            if(loop->basic_induction_variables().count(variable)){
                                loop->dependent_induction_variables()[var] = {quadruple, variable, -1, -1 * e, false}; 
                                continue;
                            } else if(loop->dependent_induction_variables()[variable].i){
                                auto equation = loop->dependent_induction_variables()[variable];
                                loop->dependent_induction_variables()[var] = {quadruple, equation.i, -1 * equation.e, e - equation.d, false}; 
                                continue;
                            }
                        }
                    } else if(mtac::isInt(arg2) && mtac::isVariable(arg1)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg1);
                        auto e = boost::get<int>(arg2);

                        if(variable != var){
                            if(loop->basic_induction_variables().count(variable)){
                                loop->dependent_induction_variables()[var] = {quadruple, variable, 1, -1 * boost::get<int>(arg2), false}; 
                                continue;
                            } else if(loop->dependent_induction_variables()[variable].i){
                                auto equation = loop->dependent_induction_variables()[variable];
                                loop->dependent_induction_variables()[var] = {quadruple, equation.i, equation.e, equation.d - e, false}; 
                                continue;
                            }
                        }
                    } 
                } else if(quadruple->op == mtac::Operator::MINUS){
                    if(mtac::isVariable(arg1)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg1);

                        if(variable != var){
                            if(loop->basic_induction_variables().count(variable)){
                                loop->dependent_induction_variables()[var] = {quadruple, variable, -1, 0, false}; 
                                continue;
                            } else if(loop->dependent_induction_variables()[variable].i){
                                auto equation = loop->dependent_induction_variables()[variable];
                                loop->dependent_induction_variables()[var] = {quadruple, equation.i, -1 * equation.e, -1 * equation.d, false}; 
                                continue;
                            }
                        }
                    } 
                }
                
                loop->dependent_induction_variables().erase(var);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                auto call = *ptr;

                if(call->return_){
                    loop->dependent_induction_variables().erase(call->return_);
                }

                if(call->return2_){
                    loop->dependent_induction_variables().erase(call->return2_);
                }
            }
        }
    }

    clean_defaults(loop->dependent_induction_variables());
}

std::pair<bool, int> get_initial_value(mtac::basic_block_p bb, std::shared_ptr<Variable> var){
    auto it = bb->statements.rbegin();
    auto end = bb->statements.rend();

    while(it != end){
        auto statement = *it;

        if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
            if((*ptr)->result == var){
                if((*ptr)->op == mtac::Operator::ASSIGN){
                    if(auto* val_ptr = boost::get<int>(&*(*ptr)->arg1)){
                        return std::make_pair(true, *val_ptr);                    
                    }
                }

                return std::make_pair(false, 0);
            }
        }

        ++it;
    }
    
    return std::make_pair(false, 0);
}

int number_of_iterations(mtac::LinearEquation& linear_equation, int initial_value, mtac::Statement& if_statement){
    if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&if_statement)){
        auto if_ = *ptr;

        if(mtac::isVariable(if_->arg1)){
            auto var = boost::get<std::shared_ptr<Variable>>(if_->arg1);

            if(var != linear_equation.i){
                return -1;   
            }

            if(auto* cst_ptr = boost::get<int>(&*if_->arg2)){
                int number = *cst_ptr;

                //We found the form "var op number"
                
                if(if_->op == mtac::BinaryOperator::LESS){
                    return (number - initial_value) / linear_equation.d + 1;
                } else if(if_->op == mtac::BinaryOperator::LESS_EQUALS){
                    return (number + 1 - initial_value) / linear_equation.d + 1;
                }

                return -1;
            } 
        } else if(auto* cst_ptr = boost::get<int>(&if_->arg1)){
            int number = *cst_ptr;

            if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*if_->arg2)){
                if(*var_ptr != linear_equation.i){
                    return -1;   
                }
                
                //We found the form "number op var"

                if(if_->op == mtac::BinaryOperator::GREATER){
                    return (number - initial_value) / linear_equation.d + 1;
                } else if(if_->op == mtac::BinaryOperator::GREATER_EQUALS){
                    return (number + 1 - initial_value) / linear_equation.d + 1;
                }

                return -1;
            } 
        } 
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&if_statement)){
        auto if_ = *ptr;

        if(mtac::isVariable(if_->arg1)){
            auto var = boost::get<std::shared_ptr<Variable>>(if_->arg1);

            if(var != linear_equation.i){
                return -1;   
            }

            if(auto* cst_ptr = boost::get<int>(&*if_->arg2)){
                int number = *cst_ptr;

                //We found the form "var op number"
                
                if(if_->op == mtac::BinaryOperator::GREATER_EQUALS){
                    return (number - initial_value) / linear_equation.d + 1;
                } else if(if_->op == mtac::BinaryOperator::GREATER){
                    return (number + 1 - initial_value) / linear_equation.d + 1;
                }

                return -1;
            } 
        } else if(auto* cst_ptr = boost::get<int>(&if_->arg1)){
            int number = *cst_ptr;

            if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*if_->arg2)){
                if(*var_ptr != linear_equation.i){
                    return -1;   
                }
                
                //We found the form "number op var"
                
                if(if_->op == mtac::BinaryOperator::LESS_EQUALS){
                    return (number - initial_value) / linear_equation.d + 1;
                } else if(if_->op == mtac::BinaryOperator::LESS){
                    return (number + 1 - initial_value) / linear_equation.d + 1;
                }

                return -1;
            } 
        } 
    }

    return -1;
}

} //end of anonymous namespace

void mtac::full_loop_analysis(mtac::Program& program){
    for(auto& function : program.functions){
        full_loop_analysis(function);
    }
}

void mtac::full_loop_analysis(mtac::Function& function){
    compute_dominators(function);

    for(auto& bb : function){
        init_depth(bb);
    }

    //Run the analysis on the function
    mtac::loop_analysis()(function);

    for(auto& loop : function.loops()){
        for(auto& bb : loop){
            increase_depth(bb);
        }
    }
}

bool mtac::loop_analysis::operator()(mtac::Function& function){
    std::vector<std::pair<mtac::basic_block_p, mtac::basic_block_p>> back_edges;

    for(auto& block : function){
        for(auto& succ : block->successors){
            //A node dominates itself
            if(block == succ){
                back_edges.emplace_back(block, succ);
            } else {
                if(block->dominator == succ){
                    back_edges.emplace_back(block, succ);
                }
            }
        }
    }

    function.loops().clear();

    //Get all edges n -> d
    for(auto& back_edge : back_edges){
        std::set<mtac::basic_block_p> natural_loop;

        auto n = back_edge.first;
        auto d = back_edge.first;

        natural_loop.insert(d);
        natural_loop.insert(n);

        LOG<Trace>("Control-Flow") << "Back edge n = B" << n->index << log::endl;
        LOG<Trace>("Control-Flow") << "Back edge d = B" << d->index << log::endl;

        if(n != d){
            std::stack<mtac::basic_block_p> vertices;
            vertices.push(n);

            while(!vertices.empty()){
                auto source = vertices.top();
                vertices.pop();

                for(auto& target : source->predecessors){
                    if(target != source && target != d && !natural_loop.count(target)){
                        natural_loop.insert(target);
                        vertices.push(target);
                    }
                }
            }
        }

        LOG<Trace>("Control-Flow") << "Natural loop of size " << natural_loop.size() << log::endl;

        auto loop = std::make_shared<mtac::Loop>(natural_loop);
        function.loops().push_back(loop);
    }

    LOG<Trace>("Control-Flow") << "Found " << function.loops().size() << " natural loops" << log::endl;

    //Find BIV and DIV of the loops
    for(auto& loop : function.loops()){
        find_basic_induction_variables(loop);
        find_dependent_induction_variables(loop, function);
    }

    //Basic computation of estimates
    for(auto& loop : function.loops()){
        if(loop->blocks().size() == 1){
            auto& basic_induction_variables = loop->basic_induction_variables();

            if(basic_induction_variables.size() == 1){
                auto bb = *loop->begin();

                if(bb->prev){
                    auto biv = *basic_induction_variables.begin();
                    auto linear_equation = biv.second;

                    auto initial_value = get_initial_value(bb->prev, linear_equation.i);

                    if(initial_value.first){
                        auto it = number_of_iterations(linear_equation, initial_value.second, bb->statements[bb->statements.size() - 1]);

                        //number_of_iterations gives the upper bound
                        it = it - 1;

                        loop->estimate() = it;
                        loop->initial_value() = initial_value.second;
                    }
                }
            }
        }
    }

    //Analysis only
    return false;
}
