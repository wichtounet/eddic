//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "iterators.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"
#include "FunctionContext.hpp"
#include "logging.hpp"
#include "Variable.hpp"

#include "mtac/Loop.hpp"
#include "mtac/induction_variable_optimizations.hpp"
#include "mtac/loop_analysis.hpp"
#include "mtac/VariableReplace.hpp"
#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Statement.hpp"
#include "mtac/Utils.hpp"
#include "mtac/variable_usage.hpp"

using namespace eddic;

namespace {

mtac::basic_block_p create_pre_header(std::shared_ptr<mtac::Loop> loop, mtac::function_p function){
    auto first_bb = *loop->blocks().begin();

    //Remove the fall through edge
    mtac::remove_edge(first_bb->prev, first_bb);
    
    auto pre_header = function->new_bb();
    
    function->insert_before(function->at(first_bb), pre_header);

    //Create the fall through edge
    mtac::make_edge(pre_header, pre_header->next);
    mtac::make_edge(pre_header->prev, pre_header);
    
    return pre_header;
}

bool strength_reduce(std::shared_ptr<mtac::Loop> loop, mtac::LinearEquation& basic_equation, mtac::function_p function){
    auto& dependent_induction_variables = loop->dependent_induction_variables();

    mtac::basic_block_p pre_header = nullptr;
    bool optimized = false;

    mtac::InductionVariables new_induction_variables;

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

            for(auto& bb : loop){
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

                    if(!it.has_next()){
                        break;
                    }

                    visit(replacer, *it);

                    ++it;
                }
            }

            //Create the preheader if necessary
            if(!pre_header){
                pre_header = create_pre_header(loop, function);
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

void induction_variable_removal(std::shared_ptr<mtac::Loop> loop){
    auto& dependent_induction_variables = loop->dependent_induction_variables();
    
    mtac::Usage usage = compute_read_usage(loop);

    //Remove generated copy when useless
    for(auto& bb : loop){
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
                            LOG<Trace>("Loops") << "Remove copy " << j->name() << "=" << tj->name() << " generated during strength reduction" << log::endl;

                            //There is one less read of tj
                            --usage.read[tj];

                            dependent_induction_variables.erase(j);

                            it.erase();
                            continue;
                        }
                    }
                }
            }

            ++it;
        }
    }

    std::vector<std::shared_ptr<Variable>> removed_variables;

    //Remove induction variables that contribute only to themselves
    for(auto& var : dependent_induction_variables){
        if(usage.read[var.first] == 1){
            var.second.def->op = mtac::Operator::NOP;
            var.second.def->result = nullptr;
            var.second.def->arg1.reset();
            var.second.def->arg2.reset();

            usage.read[var.first] = 0;

            removed_variables.push_back(var.first);

            LOG<Trace>("Loops") << "Remove DIV " << var.first->name() << log::endl;
        }
    }

    for(auto& variable : removed_variables){
        dependent_induction_variables.erase(variable);
    }
}

void induction_variable_replace(std::shared_ptr<mtac::Loop> loop){
    auto& basic_induction_variables = loop->basic_induction_variables();
    auto& dependent_induction_variables = loop->dependent_induction_variables();
    
    auto exit_block = *loop->blocks().rbegin();

    auto exit_statement = exit_block->statements.back();

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

    mtac::Usage usage = compute_read_usage(loop);

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

        LOG<Trace>("Loops") << "Replace BIV " << biv->name() << " by DIV " << div->name() << log::endl;
       
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

bool loop_induction_variables_optimization(std::shared_ptr<mtac::Loop> loop, mtac::function_p function){
    bool optimized = false;

    //1. Strength reduction on each dependent induction variables
    for(auto& basic : loop->basic_induction_variables()){
        optimized |= strength_reduce(loop, basic.second, function);
    }
    
    for(auto& biv : loop->basic_induction_variables()){
        LOG<Trace>("Loops") << "BIV: " << biv.first->name() << " = " << biv.second.e << " * " << biv.second.i->name() << " + " << biv.second.d << log::endl;
    }
    
    for(auto& biv : loop->dependent_induction_variables()){
        LOG<Trace>("Loops") << "DIV: " << biv.first->name() << " = " << biv.second.e << " * " << biv.second.i->name() << " + " << biv.second.d << " g:" << biv.second.generated << log::endl;
    }

    //2. Removal of dependent induction variables
    induction_variable_removal(loop);

    //3. Replace basic induction variable with another dependent variable
    induction_variable_replace(loop);

    return optimized;
}

} //end of anonymous namespace

bool mtac::loop_induction_variables_optimization::operator()(mtac::function_p function){
    if(function->loops().empty()){
        return false;
    }

    bool optimized = false;
    
    for(auto& loop : function->loops()){
        optimized |= ::loop_induction_variables_optimization(loop, function);
    }

    return optimized;
}
