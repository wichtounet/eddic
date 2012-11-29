//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/ConstantPropagationProblem.hpp"
#include "mtac/Utils.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/Statement.hpp"

#include "ltac/Statement.hpp"

using namespace eddic;

typedef mtac::ConstantPropagationProblem::ProblemDomain ProblemDomain;

ProblemDomain mtac::ConstantPropagationProblem::Boundary(mtac::function_p function){
    pointer_escaped = mtac::escape_analysis(function);

    return default_element();
}

ProblemDomain mtac::ConstantPropagationProblem::meet(ProblemDomain& in, ProblemDomain& out){
    if(in.top() && out.top()){
        typename ProblemDomain::Values values;
        ProblemDomain result(values);

        return result;
    } else if(in.top()){
        return out;
    } else if(out.top()){
        return in;
    } else {
        typename ProblemDomain::Values values;
        ProblemDomain result(values);

        for(auto& it : in){
            auto var = it.first;
            auto v1 = it.second;

            if(v1.nac()){
                result[var] = {}; //NAC
            } else {
                auto c1 = v1.value();

                if(out.find(var) != out.end()){
                    auto v2 = out[var];

                    if(v2.nac()){
                        result[var] = {}; //NAC
                    } else {
                        auto c2 = v2.value();

                        if(c1 == c2){
                            result[var] = c1;
                        } else {
                            result[var] = {}; //NAC
                        }
                    }
                } else {
                    //v2 = undef
                    result[var] = c1;
                }
            }
        }

        for(auto& it : out){
            auto var = it.first;
            auto v2 = it.second;
            
            if(in.find(var) == in.end()){
                //v1 = undef

                if(v2.nac()){
                    result[var] = {}; //NAC
                } else {
                    auto c2 = v2.value();

                    result[var] = c2;
                }
            }
        }

        return result;
    }
}

namespace {

struct ConstantCollector : public boost::static_visitor<> {
    ProblemDomain& out;
    std::shared_ptr<Variable> var;

    ConstantCollector(ProblemDomain& out, std::shared_ptr<Variable> var) : out(out), var(var) {}

    void operator()(int value){
        out[var] = {value};
    }
    
    //Warning : Do not pass it by reference to avoid going to the template function
    void operator()(std::string value){
        out[var] = {value};
    }
    
    void operator()(double value){
        out[var] = {value};
    }
    
    void operator()(std::shared_ptr<Variable> variable){
        if(variable != var){
            out[var] = {variable};
        }
    }
};

} //end of anonymous namespace

ProblemDomain mtac::ConstantPropagationProblem::transfer(mtac::basic_block_p/* basic_block*/, mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    std::shared_ptr<Variable> remove_copies;

    //Quadruple affects variable
    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr;

        if(quadruple->op == mtac::Operator::NOP){
            return out;
        }

        if(quadruple->op == mtac::Operator::ASSIGN || quadruple->op == mtac::Operator::FASSIGN){
            ConstantCollector collector(out, quadruple->result);
            visit(collector, *quadruple->arg1);
                
            remove_copies = quadruple->result;
        } else {
            auto op = quadruple->op;

            if(mtac::erase_result(op)){
                //The result is not constant at this point
                out[quadruple->result].set_nac();
    
                remove_copies = quadruple->result;
            }
        }
    }
    //Passing a variable by pointer erases its value
    else if (auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
        auto param = *ptr;

        if(param->address){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param->arg)){
                //Impossible to know if the variable is modified or not, consider it modified
                out[*ptr].set_nac();
            
                remove_copies = *ptr;
            }
        }
    }

    if(remove_copies){
        //Cancel the copy of the variable erased
        for(auto it = std::begin(out.values()); it != std::end(out.values()); ++it){
            auto& lattice = it->second;

            if(lattice.constant()){
                auto lattice_value = lattice.value();
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&lattice_value)){
                    auto variable = *ptr;

                    if (variable == remove_copies){
                        lattice.set_nac();
                    } 
                }
            } 
        }
    }

    return out;
}

namespace {

struct ConstantOptimizer : public boost::static_visitor<> {
    mtac::Domain<mtac::ConstantPropagationValues>& results;
    mtac::EscapedVariables& pointer_escaped;
    bool changes = false;

    ConstantOptimizer(mtac::Domain<mtac::ConstantPropagationValues>& results, mtac::EscapedVariables& pointer_escaped) : results(results), pointer_escaped(pointer_escaped) {}

    bool optimize_arg(mtac::Argument& arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&arg)){
            if(results.count(*ptr) && !pointer_escaped->count(*ptr)){
                if(results[*ptr].constant()){
                    arg = results[*ptr].value();
                    return true;
                }
            }
        }

        return false;
    }

    bool optimize_optional(boost::optional<mtac::Argument>& arg){
        if(arg){
            return optimize_arg(*arg);
        }

        return false;
    }

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        //If the constant is a string, we can use it in the dot operator
        if(quadruple->op == mtac::Operator::DOT){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                if((*ptr)->type() != STRING && results.count(*ptr) && !pointer_escaped->count(*ptr)){
                    if(results[*ptr].constant()){
                        auto arg = results[*ptr].value();

                        if(auto* label_ptr = boost::get<std::string>(&arg)){
                            quadruple->arg1 = *label_ptr;

                            changes = true;
                        }
                    }
                }
            }
        //Do not replace a variable by a constant when used in offset
        } else if(quadruple->op != mtac::Operator::PDOT && quadruple->op != mtac::Operator::PASSIGN){
            changes |= optimize_optional(quadruple->arg1);
        }
        
        if(quadruple->op != mtac::Operator::DOT_PASSIGN){
            changes |= optimize_optional(quadruple->arg2);
        }

        if(!mtac::erase_result(quadruple->op) && quadruple->result && quadruple->op != mtac::Operator::DOT_ASSIGN){
            if(results.find(quadruple->result) != results.end()){
                if(results[quadruple->result].constant()){
                    auto lattice_value = results[quadruple->result].value();
                    if(mtac::isVariable(lattice_value)){
                        quadruple->result = boost::get<std::shared_ptr<Variable>>(lattice_value);
                    }
                }
            }
        }
    }

    void operator()(std::shared_ptr<mtac::Param> param){
        if(!param->address){
            changes |= optimize_arg(param->arg);
        }
    }

    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        changes |= optimize_arg(if_false->arg1);
        changes |= optimize_optional(if_false->arg2);
    }

    void operator()(std::shared_ptr<mtac::If> if_){
        changes |= optimize_arg(if_->arg1);
        changes |= optimize_optional(if_->arg2);
    }

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

} //end of anonymous namespace

bool mtac::ConstantPropagationProblem::optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> global_results){
    if(global_results->IN_S[statement].top()){
        return false;
    }

    ConstantOptimizer optimizer(global_results->IN_S[statement], pointer_escaped);

    visit(optimizer, statement);

    return optimizer.changes;
}

std::ostream& mtac::operator<<(std::ostream& stream, ConstantPropagationLattice& lattice){
    if(lattice.nac()){
        return stream << "NAC";
    } else {
        return stream << lattice.value();
    }
}
