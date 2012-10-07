//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include <boost/graph/dominator_tree.hpp>

#include "iterators.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"
#include "FunctionContext.hpp"
#include "logging.hpp"

#include "mtac/loop_optimizations.hpp"
#include "mtac/loop_analysis.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Utils.hpp"
#include "mtac/VariableReplace.hpp"

using namespace eddic;

namespace {

typedef mtac::ControlFlowGraph::InternalControlFlowGraph G;
typedef std::set<mtac::ControlFlowGraph::BasicBlockInfo> Loop;
typedef mtac::ControlFlowGraph::BasicBlockInfo Vertex;
typedef boost::property_map<mtac::ControlFlowGraph::InternalControlFlowGraph, boost::vertex_index_t>::type IndexMap;
typedef boost::iterator_property_map<std::vector<Vertex>::iterator, IndexMap> PredMap;

struct Usage {
    std::unordered_map<std::shared_ptr<Variable>, unsigned int> written;
    std::unordered_map<std::shared_ptr<Variable>, unsigned int> read;
};

Usage compute_write_usage(const Loop& loop, const G& g){
    Usage usage;

    for(auto& vertex : loop){
        auto bb = g[vertex].block;
        
        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ptr;
                if(mtac::erase_result(quadruple->op)){
                    ++(usage.written[quadruple->result]);
                } 
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                auto call = *ptr;

                if(call->return_){
                    ++(usage.written[call->return_]);
                }
                
                if(call->return2_){
                    ++(usage.written[call->return2_]);
                }
            }
        }
    }

    return usage;
}

struct VariableReadCollector : public boost::static_visitor<> {
    Usage& usage;

    VariableReadCollector(Usage& usage) : usage(usage) {}

    void inc_usage(std::shared_ptr<Variable> variable){
        ++usage.read[variable];
    }

    template<typename T>
    void collect(T& arg){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&arg)){
            inc_usage(*variablePtr);
        }
    }

    template<typename T>
    void collect_optional(T& opt){
        if(opt){
            collect(*opt);
        }
    }

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        if(!mtac::erase_result(quadruple->op)){
            inc_usage(quadruple->result);
        }

        collect_optional(quadruple->arg1);
        collect_optional(quadruple->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::Param> param){
        collect(param->arg);
    }
    
    void operator()(std::shared_ptr<mtac::If> if_){
        collect(if_->arg1);
        collect_optional(if_->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        collect(if_false->arg1);
        collect_optional(if_false->arg2);
    }

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

Usage compute_read_usage(const Loop& loop, const G& g){
    Usage usage;
    VariableReadCollector collector(usage);

    for(auto& vertex : loop){
        auto bb = g[vertex].block;
        
        visit_each(collector, bb->statements);
    }

    return usage;
}

bool is_invariant(boost::optional<mtac::Argument>& argument, Usage& usage){
    if(argument){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*argument)){
            return usage.written[*ptr] == 0;
        }
    }

    return true;
}

bool is_invariant(mtac::Statement& statement, Usage& usage){
    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr;

        //TODO Relax this rule by making a more powerful memory analysis
        if(quadruple->op == mtac::Operator::DOT || quadruple->op == mtac::Operator::FDOT || quadruple->op == mtac::Operator::PDOT){
            return false;
        }

        if(mtac::erase_result(quadruple->op)){
            //If there are more than one write to this variable, the computation is not invariant
            if(usage.written[quadruple->result] > 1){
                return false;
            }

            return is_invariant(quadruple->arg1, usage) && is_invariant(quadruple->arg2, usage);
        }

        return false;
    }

    return false;
}

std::shared_ptr<mtac::BasicBlock> create_pre_header(const Loop& loop, std::shared_ptr<mtac::Function> function, const G& g){
    //Create a new basic block and detach it from the function
    auto pre_header = function->newBasicBlock();
    function->getBasicBlocks().pop_back();
    
    auto first_bb = g[*loop.begin()].block;

    auto bit = iterate(function->getBasicBlocks());

    while(bit.has_next()){
        if(*bit == first_bb){
            bit.insert(pre_header);

            break;
        }

        ++bit;
    }
    
    return pre_header;
}

struct UsageCollector : public boost::static_visitor<bool> {
    std::shared_ptr<Variable> var;

    UsageCollector(std::shared_ptr<Variable> var) : var(var) {}

    template<typename T>
    bool collect(T& arg){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&arg)){
            return *variablePtr == var;
        }

        return false;
    }

    template<typename T>
    bool collect_optional(T& opt){
        if(opt){
            return collect(*opt);
        }

        return false;
    }

    bool operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        return quadruple->result == var || collect_optional(quadruple->arg1) || collect_optional(quadruple->arg2);
    }
    
    bool operator()(std::shared_ptr<mtac::Param> param){
        return collect(param->arg);
    }
    
    bool operator()(std::shared_ptr<mtac::If> if_){
        return collect(if_->arg1) || collect_optional(if_->arg2);
    }
    
    bool operator()(std::shared_ptr<mtac::IfFalse> if_false){
        return collect(if_false->arg1) || collect_optional(if_false->arg2);
    }

    template<typename T>
    bool operator()(T&){
        return false;
    }
};

bool use_variable(std::shared_ptr<mtac::BasicBlock> bb, std::shared_ptr<Variable> var){
    UsageCollector collector(var);

    for(auto& statement : bb->statements){
        if(visit(collector, statement)){
            return true;
        }
    }

    return false;
}

/*!
 * \brief Test if an invariant is valid or not. 
 * An invariant defining v is valid if: 
 * 1. It is in a basic block that dominates all other uses of v
 * 2. It is in a basic block that dominates all exit blocks of the loop
 */
bool is_valid_invariant(std::shared_ptr<mtac::BasicBlock> source_bb, mtac::Statement statement, const Loop& loop, const G& g, const PredMap& domTreePredMap){
    auto quadruple = boost::get<std::shared_ptr<mtac::Quadruple>>(statement);
    auto var = quadruple->result;

    for(auto& vertex : loop){
        auto bb = g[vertex].block;

        //A bb always dominates itself => no need to consider the source basic block
        if(bb != source_bb){
            if(use_variable(bb, var)){
                if(boost::get(domTreePredMap, vertex) != boost::graph_traits<G>::null_vertex()){
                    auto dominator = boost::get(domTreePredMap, vertex);

                    //If the bb is not dominated by the source bb, it is not valid
                    if(g[dominator].block != source_bb){
                        return false;
                    }
                }
            }
        }
    }
    
    auto exit_block = *loop.rbegin();

    if(g[exit_block].block == source_bb){
        return true;
    }
                
    if(boost::get(domTreePredMap, exit_block) != boost::graph_traits<G>::null_vertex()){
        auto dominator = boost::get(domTreePredMap, exit_block);

        //If the exit bb is not dominated by the source bb, it is not valid
        if(g[dominator].block != source_bb){
            return false;
        }
    }
    
    return true;
}

bool loop_invariant_code_motion(const Loop& loop, std::shared_ptr<mtac::Function> function, const G& g, const PredMap& domTreePredMap){
    std::shared_ptr<mtac::BasicBlock> pre_header;

    bool optimized = false;

    auto usage = compute_write_usage(loop, g);

    for(auto& vertex : loop){
        auto bb = g[vertex].block;

        auto it = iterate(bb->statements); 

        while(it.has_next()){
            auto statement = *it;

            if(is_invariant(statement, usage)){
                if(is_valid_invariant(bb, statement, loop, g, domTreePredMap)){
                    //Create the preheader if necessary
                    if(!pre_header){
                        pre_header = create_pre_header(loop, function, g);
                    }

                    it.erase();
                    pre_header->statements.push_back(statement);

                    optimized = true;

                    continue;
                } 
            }

            ++it;
        }
    }

    return optimized;
}

struct LinearEquation {
    std::shared_ptr<mtac::Quadruple> def;
    std::shared_ptr<Variable> i;
    int e;
    int d;
    bool generated;
};

typedef std::map<std::shared_ptr<Variable>, LinearEquation> InductionVariables;

InductionVariables find_all_candidates(const Loop& loop, const G& g){
    InductionVariables candidates;
    
    for(auto& vertex : loop){
        auto bb = g[vertex].block;

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

void clean_defaults(InductionVariables& induction_variables){
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

InductionVariables find_basic_induction_variables(const Loop& loop, const G& g){
    auto basic_induction_variables = find_all_candidates(loop, g);

    for(auto& vertex : loop){
        auto bb = g[vertex].block;

        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ptr;
                auto var = quadruple->result;

                //If it is not a candidate, do not test it
                if(!basic_induction_variables.count(var)){
                    continue;
                }

                auto value = basic_induction_variables[var];

                //TODO In the future, induction variables written several times could be splitted into several induction variables
                if(value.i){
                    basic_induction_variables.erase(var);

                    continue;
                }

                if(quadruple->op == mtac::Operator::ADD){
                    auto arg1 = *quadruple->arg1;
                    auto arg2 = *quadruple->arg2;

                    if(mtac::isInt(arg1) && mtac::equals(arg2, var)){
                        basic_induction_variables[var] = {quadruple, var, 1, boost::get<int>(arg1), false};
                        continue;
                    } else if(mtac::isInt(arg2) && mtac::equals(arg1, var)){
                        basic_induction_variables[var] = {quadruple, var, 1, boost::get<int>(arg2), false}; 
                        continue;
                    } 
                } 
                
                basic_induction_variables.erase(var);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                auto call = *ptr;

                if(call->return_){
                    basic_induction_variables.erase(call->return_);
                }

                if(call->return2_){
                    basic_induction_variables.erase(call->return2_);
                }
            }
        }
    }

    clean_defaults(basic_induction_variables);

    return basic_induction_variables;
}

InductionVariables find_dependent_induction_variables(const Loop& loop, const G& g, const InductionVariables& basic_induction_variables, std::shared_ptr<mtac::Function> function){
    auto dependent_induction_variables = find_all_candidates(loop, g);

    for(auto& vertex : loop){
        auto bb = g[vertex].block;

        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ptr;
                auto var = quadruple->result;

                //If it is not a candidate, do not test it
                if(!dependent_induction_variables.count(var)){
                    continue;
                }
                
                //We know for sure that all the candidates have a first arg
                auto arg1 = *quadruple->arg1;
                
                //If it is a basic induction variable, it is not a dependent induction variable
                if(basic_induction_variables.count(var)){
                    dependent_induction_variables.erase(var);

                    continue;
                }

                auto source_equation = dependent_induction_variables[var];

                if(source_equation.i && (mtac::equals(arg1, var) || (quadruple->arg2 && mtac::equals(*quadruple->arg2, var))) ){
                    auto tj = function->context->new_temporary(INT);

                    source_equation.def->result = tj;
                    
                    dependent_induction_variables.erase(var);
                    dependent_induction_variables[tj] = source_equation;

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
                            if(basic_induction_variables.count(variable)){
                                dependent_induction_variables[var] = {quadruple, variable, e, 0, false}; 
                                continue;
                            } else if(dependent_induction_variables[variable].i){
                                auto equation = dependent_induction_variables[variable];
                                dependent_induction_variables[var] = {quadruple, equation.i, equation.e * e, equation.d * e, false}; 
                                continue;
                            }
                        }
                    } else if(mtac::isInt(arg2) && mtac::isVariable(arg1)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg1);
                        auto e = boost::get<int>(arg2);
                        
                        if(variable != var){
                            if(basic_induction_variables.count(variable)){
                                dependent_induction_variables[var] = {quadruple, variable, e, 0, false}; 
                                continue;
                            } else if(dependent_induction_variables[variable].i){
                                auto equation = dependent_induction_variables[variable];
                                dependent_induction_variables[var] = {quadruple, equation.i, equation.e * e, equation.d * e, false}; 
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
                            if(basic_induction_variables.count(variable)){
                                dependent_induction_variables[var] = {quadruple, variable, 1, boost::get<int>(arg1), false}; 
                                continue;
                            } else if(dependent_induction_variables[variable].i){
                                auto equation = dependent_induction_variables[variable];
                                dependent_induction_variables[var] = {quadruple, equation.i, equation.e, equation.d + e, false}; 
                                continue;
                            }
                        }
                    } else if(mtac::isInt(arg2) && mtac::isVariable(arg1)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg1);
                        auto e = boost::get<int>(arg2);

                        if(variable != var){
                            if(basic_induction_variables.count(variable)){
                                dependent_induction_variables[var] = {quadruple, variable, 1, boost::get<int>(arg2), false}; 
                                continue;
                            } else if(dependent_induction_variables[variable].i){
                                auto equation = dependent_induction_variables[variable];
                                dependent_induction_variables[var] = {quadruple, equation.i, equation.e, equation.d + e, false}; 
                                continue;
                            }
                        }
                    } else if(mtac::isVariable(arg1) && mtac::isVariable(arg2)){
                        auto var1 = boost::get<std::shared_ptr<Variable>>(arg1);
                        auto var2 = boost::get<std::shared_ptr<Variable>>(arg2);

                        if(var1 == var2 && var1 != var){
                            if(basic_induction_variables.count(var1)){
                                dependent_induction_variables[var] = {quadruple, var1, 2, 0, false}; 
                                continue;
                            } else if(dependent_induction_variables[var1].i){
                                auto equation = dependent_induction_variables[var1];
                                dependent_induction_variables[var] = {quadruple, equation.i, equation.e * 2, equation.d * 2, false}; 
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
                            if(basic_induction_variables.count(variable)){
                                dependent_induction_variables[var] = {quadruple, variable, -1, -1 * e, false}; 
                                continue;
                            } else if(dependent_induction_variables[variable].i){
                                auto equation = dependent_induction_variables[variable];
                                dependent_induction_variables[var] = {quadruple, equation.i, -1 * equation.e, e - equation.d, false}; 
                                continue;
                            }
                        }
                    } else if(mtac::isInt(arg2) && mtac::isVariable(arg1)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg1);
                        auto e = boost::get<int>(arg2);

                        if(variable != var){
                            if(basic_induction_variables.count(variable)){
                                dependent_induction_variables[var] = {quadruple, variable, 1, -1 * boost::get<int>(arg2), false}; 
                                continue;
                            } else if(dependent_induction_variables[variable].i){
                                auto equation = dependent_induction_variables[variable];
                                dependent_induction_variables[var] = {quadruple, equation.i, equation.e, equation.d - e, false}; 
                                continue;
                            }
                        }
                    } 
                } else if(quadruple->op == mtac::Operator::MINUS){
                    if(mtac::isVariable(arg1)){
                        auto variable = boost::get<std::shared_ptr<Variable>>(arg1);

                        if(variable != var){
                            if(basic_induction_variables.count(variable)){
                                dependent_induction_variables[var] = {quadruple, variable, -1, 0, false}; 
                                continue;
                            } else if(dependent_induction_variables[variable].i){
                                auto equation = dependent_induction_variables[variable];
                                dependent_induction_variables[var] = {quadruple, equation.i, -1 * equation.e, -1 * equation.d, false}; 
                                continue;
                            }
                        }
                    } 
                }
                
                dependent_induction_variables.erase(var);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                auto call = *ptr;

                if(call->return_){
                    dependent_induction_variables.erase(call->return_);
                }

                if(call->return2_){
                    dependent_induction_variables.erase(call->return2_);
                }
            }
        }
    }

    clean_defaults(dependent_induction_variables);

    return dependent_induction_variables;
}

bool strength_reduce(const Loop& loop, LinearEquation& basic_equation, const G& g, InductionVariables& dependent_induction_variables, std::shared_ptr<mtac::Function> function){
    std::shared_ptr<mtac::BasicBlock> pre_header = nullptr;
    bool optimized = false;

    InductionVariables new_induction_variables;

    auto i = basic_equation.i;

    for(auto& dependent : dependent_induction_variables){
        auto& equation = dependent.second;
        if(equation.i == i){
            auto j = dependent.first;

            auto tj = function->context->new_temporary(INT);
            auto db = equation.e * basic_equation.d;

            mtac::VariableClones variable_clones;
            variable_clones[j] = tj;

            mtac::VariableReplace replacer(variable_clones);

            //There is only a single assignment to j, replace it with j = tj
            equation.def->op = mtac::Operator::ASSIGN;
            equation.def->arg1 = tj;
            equation.def->arg2.reset();

            for(auto& vertex : loop){
                auto bb = g[vertex].block;

                auto it = iterate(bb->statements);

                while(it.has_next()){
                    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&*it)){
                        auto quadruple = *ptr;

                        //To avoid replacing j by tj
                        if(quadruple == equation.def){
                            ++it;
                        } 
                        //After an assignment to a basic induction variable, insert addition for tj
                        else if(quadruple == basic_equation.def){
                            ++it;
                            auto new_quadruple = std::make_shared<mtac::Quadruple>(tj, tj, mtac::Operator::ADD, db);
                            it.insert(new_quadruple);

                            new_induction_variables[tj] = {new_quadruple, i, equation.e, equation.d, true};

                            //To avoid replacing j by tj
                            ++it;
                        }
                    }

                    visit(replacer, *it);

                    ++it;
                }
            }

            //Create the preheader if necessary
            if(!pre_header){
                pre_header = create_pre_header(loop, function, g);
            }

            pre_header->statements.push_back(std::make_shared<mtac::Quadruple>(tj, equation.e, mtac::Operator::MUL, i));
            pre_header->statements.push_back(std::make_shared<mtac::Quadruple>(tj, tj, mtac::Operator::ADD, equation.d));

            optimized = true;
        }
    }

    for(auto& new_var : new_induction_variables){
        dependent_induction_variables[new_var.first] = new_var.second;
    }

    return optimized;
}

void induction_variable_removal(const Loop& loop, InductionVariables& dependent_induction_variables, const G& g){
    Usage usage = compute_read_usage(loop, g);

    //Remove generated copy when useless
    for(auto& vertex : loop){
        auto bb = g[vertex].block;

        auto it = iterate(bb->statements);

        while(it.has_next()){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&*it)){
                auto quadruple = *ptr;

                if(quadruple->op == mtac::Operator::ASSIGN && mtac::isVariable(*quadruple->arg1)){
                    auto j = quadruple->result;
                    auto tj = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

                    //If j = tj generated in strength reduction phase
                    if(dependent_induction_variables.count(j) && dependent_induction_variables.count(tj) && dependent_induction_variables[tj].generated){
                        if(!usage.read.count(j)){
                            //There is one less read of tj
                            --usage.read[tj];

                            it.erase();
                            continue;
                        }
                    }
                }
            }

            ++it;
        }
    }

    //Remove induction variables that contribute only to themselves
    for(auto& var : dependent_induction_variables){
        if(usage.read[var.first] == 1){
            var.second.def->op = mtac::Operator::NOP;
            var.second.def->result = nullptr;
            var.second.def->arg1.reset();
            var.second.def->arg2.reset();

            usage.read[var.first] = 0;
        }
    }
}

void induction_variable_replace(const Loop& loop, InductionVariables& basic_induction_variables, InductionVariables& dependent_induction_variables, const G& g){
    auto exit_block = *loop.rbegin();

    auto exit_statement = g[exit_block].block->statements.back();

    std::shared_ptr<Variable> biv;
    int end = 0;

    if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&exit_statement)){
        auto if_ = *ptr;

        if(if_->op && *if_->op <= mtac::BinaryOperator::LESS_EQUALS){
            if(mtac::isVariable(if_->arg1) && mtac::isInt(*if_->arg2)){
                biv = boost::get<std::shared_ptr<Variable>>(if_->arg1);
                end = boost::get<int>(*if_->arg2);
            } else if(mtac::isVariable(*if_->arg2) && mtac::isInt(if_->arg1)){
                biv = boost::get<std::shared_ptr<Variable>>(*if_->arg2);
                end = boost::get<int>(if_->arg1);
            }
        }
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&exit_statement)){
        auto if_ = *ptr;

        if(if_->op && *if_->op <= mtac::BinaryOperator::LESS_EQUALS){
            if(mtac::isVariable(if_->arg1) && mtac::isInt(*if_->arg2)){
                biv = boost::get<std::shared_ptr<Variable>>(if_->arg1);
                end = boost::get<int>(*if_->arg2);
            } else if(mtac::isVariable(*if_->arg2) && mtac::isInt(if_->arg1)){
                biv = boost::get<std::shared_ptr<Variable>>(*if_->arg2);
                end = boost::get<int>(if_->arg1);
            }
        }
    }

    //The loop is only countable if the condition depends on biv and the biv is increasing
    if(!biv || !basic_induction_variables.count(biv) || basic_induction_variables[biv].d <= 0){
        return;
    }

    Usage usage = compute_read_usage(loop, g);

    //If biv is only used to compute itself (as a basic induction variable) and in the condition
    if(usage.read[biv] == 2){
        std::shared_ptr<Variable> div;
        
        for(auto& d : dependent_induction_variables){
            auto eq = d.second;

            if(eq.def && eq.def->op != mtac::Operator::NOP && eq.i == biv && eq.e > 0){
                div = d.first;
                break;
            }
        }
        
        //If there are no candidate
        if(!div){
            return;
        }

        log::emit<Trace>("Loops") << "Replace BIV " << biv->name() << " by DIV " << div->name() << log::endl;
       
        auto div_equation = dependent_induction_variables[div];
        auto new_end = div_equation.e * end + div_equation.d;

        usage.read[biv] = 0;
    
        //Update the exit condition
        if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&exit_statement)){
            auto if_ = *ptr;

            if(mtac::isVariable(if_->arg1) && mtac::isInt(*if_->arg2)){
                if_->arg1 = div;
                if_->arg2 = new_end;
            } else if(mtac::isVariable(*if_->arg2) && mtac::isInt(if_->arg1)){
                if_->arg2 = div;
                if_->arg1 = new_end;
            }
        } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&exit_statement)){
            auto if_ = *ptr;

            if(mtac::isVariable(if_->arg1) && mtac::isInt(*if_->arg2)){
                if_->arg1 = div;
                if_->arg2 = new_end;
            } else if(mtac::isVariable(*if_->arg2) && mtac::isInt(if_->arg1)){
                if_->arg2 = div;
                if_->arg1 = new_end;
            }
        }
            
        //The unique assignment to i is not useful anymore 
        basic_induction_variables[biv].def->op = mtac::Operator::NOP;
        basic_induction_variables[biv].def->result = nullptr;
        basic_induction_variables[biv].def->arg1.reset();
        basic_induction_variables[biv].def->arg2.reset();

        //Not a basic induction variable anymore
        basic_induction_variables.erase(biv);
    }
}

bool loop_induction_variables_optimization(const Loop& loop, std::shared_ptr<mtac::Function> function, const G& g){
    bool optimized = false;

    //1. Identify all the induction variables
    auto basic_induction_variables = find_basic_induction_variables(loop, g);
    auto dependent_induction_variables = find_dependent_induction_variables(loop, g, basic_induction_variables, function);

    //2. Strength reduction on each dependent induction variables
    for(auto& basic : basic_induction_variables){
        optimized |= strength_reduce(loop, basic.second, g, dependent_induction_variables, function);
    }
    
    for(auto& biv : basic_induction_variables){
        log::emit<Trace>("Loops") << "BIV: " << biv.first->name() << " = " << biv.second.e << " * " << biv.second.i->name() << " + " << biv.second.d << log::endl;
    }
    
    for(auto& biv : dependent_induction_variables){
        log::emit<Trace>("Loops") << "DIV: " << biv.first->name() << " = " << biv.second.e << " * " << biv.second.i->name() << " + " << biv.second.d << " g:" << biv.second.generated << log::endl;
    }

    //3. Removal of dependent induction variables
    induction_variable_removal(loop, dependent_induction_variables, g);
    
    //Update induction variables for the last phase
    basic_induction_variables = find_basic_induction_variables(loop, g);
    dependent_induction_variables = find_dependent_induction_variables(loop, g, basic_induction_variables, function);

    //4. Replace basic induction variable with another dependent variable
    induction_variable_replace(loop, basic_induction_variables, dependent_induction_variables, g);

    return optimized;
}

int number_of_iterations(LinearEquation& linear_equation, int initial_value, mtac::Statement& if_statement){
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

bool mtac::loop_invariant_code_motion::operator()(std::shared_ptr<mtac::Function> function){
    auto graph = mtac::build_control_flow_graph(function);
    auto g = graph->get_graph();

    auto natural_loops = find_natural_loops(g);

    if(natural_loops.empty()){
        return false;
    }

    bool optimized = false;
    
    std::vector<Vertex> domTreePredVector = std::vector<Vertex>(boost::num_vertices(g), boost::graph_traits<G>::null_vertex());
    PredMap domTreePredMap = boost::make_iterator_property_map(domTreePredVector.begin(), boost::get(boost::vertex_index, g));

    boost::lengauer_tarjan_dominator_tree(g, boost::vertex(0, g), domTreePredMap);

    for(auto& loop : natural_loops){
        optimized |= ::loop_invariant_code_motion(loop, function, g, domTreePredMap);
    }
    
    return optimized;
}

bool mtac::loop_induction_variables_optimization::operator()(std::shared_ptr<mtac::Function> function){
    auto graph = mtac::build_control_flow_graph(function);
    auto g = graph->get_graph();
    
    auto natural_loops = find_natural_loops(g);

    if(natural_loops.empty()){
        return false;
    }

    bool optimized = false;
    
    for(auto& loop : natural_loops){
        optimized |= ::loop_induction_variables_optimization(loop, function, g);
    }

    return optimized;
}

std::shared_ptr<mtac::BasicBlock> get_previous_bb(std::shared_ptr<mtac::Function> function, std::shared_ptr<mtac::BasicBlock> bb){
    auto blocks_it = function->blocks();
    auto it = std::find(blocks_it.first, blocks_it.second, bb);

    if(it == blocks_it.second || it == blocks_it.first){
        return nullptr;
    }

    --it;

    return *it;
}

std::pair<bool, int> get_initial_value(std::shared_ptr<mtac::BasicBlock> bb, std::shared_ptr<Variable> var){
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
    }
    
    return std::make_pair(false, 0);
}

bool mtac::remove_empty_loops::operator()(std::shared_ptr<mtac::Function> function){
    auto graph = mtac::build_control_flow_graph(function);
    auto g = graph->get_graph();
    
    auto natural_loops = find_natural_loops(g);

    if(natural_loops.empty()){
        return false;
    }

    bool optimized = false;
    
    for(auto& loop : natural_loops){
        if(loop.size() == 1){
            auto bb = g[*loop.begin()].block;

            if(bb->statements.size() == 2){
                if(auto* first_ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&bb->statements[0])){
                    auto first = *first_ptr;
                
                    auto basic_induction_variables = find_basic_induction_variables(loop, g);
                    
                    auto prev_bb = get_previous_bb(function, bb);

                    if(prev_bb){
                        if(basic_induction_variables.find(first->result) != basic_induction_variables.end()){
                            auto initial_value = get_initial_value(prev_bb, first->result);
                            if(initial_value.first){
                                auto linear_equation = basic_induction_variables[first->result];
                                auto it = number_of_iterations(linear_equation, initial_value.second, bb->statements[1]);
                                
                                //The loop does not iterate
                                if(it == 0){
                                    bb->statements.clear();
                                    optimized = true;
                                } else if(it > 0){
                                    bb->statements.clear();
                                    optimized = true;

                                    bb->statements.push_back(std::make_shared<mtac::Quadruple>(first->result, initial_value.second + it * linear_equation.d, mtac::Operator::ASSIGN));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return optimized;
}
