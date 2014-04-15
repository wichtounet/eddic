//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/global_cp.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

typedef mtac::ConstantPropagationProblem::ProblemDomain ProblemDomain;

namespace {

struct ConstantCollector : public boost::static_visitor<> {
    ProblemDomain& out;
    std::shared_ptr<Variable>& var;

    ConstantCollector(ProblemDomain& out, std::shared_ptr<Variable>& var) : out(out), var(var) {}

    void operator()(int value){
        out[var] = {value};
    }

    void operator()(const std::string& value){
        out[var] = {value};
    }

    void operator()(double value){
        out[var] = {value};
    }

    void operator()(std::shared_ptr<Variable>& variable){
        if(variable != var){
            out[var] = {variable};
        }
    }
};

struct ConstantOptimizer {
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

    bool optimize(mtac::Quadruple& quadruple){
        if(quadruple.op == mtac::Operator::PPARAM){
            return changes;
        } else if(quadruple.op == mtac::Operator::PARAM){
            changes |= optimize_optional(quadruple.arg1);

            return changes;
        } 

        //If the constant is a string, we can use it in the dot operator
        if(quadruple.op == mtac::Operator::DOT){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
                if((*ptr)->type() != STRING && results.count(*ptr) && !pointer_escaped->count(*ptr)){
                    if(results[*ptr].constant()){
                        auto arg = results[*ptr].value();

                        if(auto* label_ptr = boost::get<std::string>(&arg)){
                            quadruple.arg1 = *label_ptr;

                            changes = true;
                        }
                    }
                }
            }
            //Do not replace a variable by a constant when used in offset
        } else if(quadruple.op != mtac::Operator::PDOT && quadruple.op != mtac::Operator::PASSIGN){
            changes |= optimize_optional(quadruple.arg1);
        }

        if(quadruple.op != mtac::Operator::DOT_PASSIGN){
            changes |= optimize_optional(quadruple.arg2);
        }

        if(!mtac::erase_result(quadruple.op) && quadruple.result && quadruple.op != mtac::Operator::DOT_ASSIGN){
            if(results.find(quadruple.result) != results.end()){
                if(results[quadruple.result].constant()){
                    auto lattice_value = results[quadruple.result].value();
                    if(mtac::isVariable(lattice_value)){
                        quadruple.result = boost::get<std::shared_ptr<Variable>>(lattice_value);
                    }
                }
            }
        }

        return changes;
    }
};
    
}

ProblemDomain mtac::ConstantPropagationProblem::Init(mtac::Function&) {
    //By default, return the top element
    return top_element();
}

ProblemDomain mtac::ConstantPropagationProblem::Boundary(mtac::Function& function){
    pointer_escaped = mtac::escape_analysis(function);

    return default_element();
}

void mtac::ConstantPropagationProblem::meet(ProblemDomain& in, const ProblemDomain& out){
    if(in.top() && out.top()){
        typename ProblemDomain::Values values;
        ProblemDomain result(values);
        in = result;
    } else if(in.top()){
        in = out;
    } else if(out.top()){
        //in doesn't change
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

                auto it = out.find(var);
                if(it != out.end()){
                    auto v2 = it->second;

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

        in = std::move(result);
    }
}

int compute(mtac::Operator op, int lhs, int rhs){
    switch(op){
        case mtac::Operator::ADD:
            return lhs + rhs;
        case mtac::Operator::SUB:
            return lhs - rhs;
        case mtac::Operator::MUL:
            return lhs * rhs;
        case mtac::Operator::DIV:
            return lhs / rhs;
        case mtac::Operator::MOD:
            return lhs % rhs;
        default:
            eddic_unreachable("Invalid operator");
    }
}

double compute(mtac::Operator op, double lhs, double rhs){
    switch(op){
        case mtac::Operator::FADD:
            return lhs + rhs;
        case mtac::Operator::FSUB:
            return lhs - rhs;
        case mtac::Operator::FMUL:
            return lhs * rhs;
        case mtac::Operator::FDIV:
            return lhs / rhs;
        default:
            eddic_unreachable("Invalid operator");
    }
}

void mtac::ConstantPropagationProblem::transfer(mtac::basic_block_p, mtac::Quadruple& quadruple, ProblemDomain& out){
    auto op = quadruple.op;

    if(op == mtac::Operator::NOP){
        return;
    }
    
    std::shared_ptr<Variable> remove_copies;

    if(op == mtac::Operator::ASSIGN || op == mtac::Operator::FASSIGN){
        ConstantCollector collector(out, quadruple.result);
        visit(collector, *quadruple.arg1);

        remove_copies = quadruple.result;
    } else if(op >= mtac::Operator::ADD && op <= mtac::Operator::MOD){
        if(auto* lhs = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
            if(out[*lhs].constant() && boost::get<int>(&out[*lhs].value())){
                if(auto* rhs = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
                    if(out[*rhs].constant() && boost::get<int>(&out[*rhs].value())){
                        out[quadruple.result] = {compute(op, boost::get<int>(out[*lhs].value()), boost::get<int>(out[*rhs].value()))};
                    } else {
                        out[quadruple.result].set_nac();
                    }
                } else if(auto* rhs = boost::get<int>(&*quadruple.arg2)){
                    out[quadruple.result] = {compute(op, boost::get<int>(out[*lhs].value()), *rhs)};
                } else {
                    out[quadruple.result].set_nac();
                }
            } else {
                out[quadruple.result].set_nac();
            }
        } else if(auto* rhs = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
            if(out[*rhs].constant() && boost::get<int>(&out[*rhs].value())){
                if(auto* lhs = boost::get<int>(&*quadruple.arg1)){
                    out[quadruple.result] = {compute(op, *lhs, boost::get<int>(out[*rhs].value()))};
                } else {
                    out[quadruple.result].set_nac();
                }
            } else {
                out[quadruple.result].set_nac();
            }
        } else {
            out[quadruple.result].set_nac();
        }

        remove_copies = quadruple.result;
    } else if(op >= mtac::Operator::FADD && op <= mtac::Operator::FDIV){
        if(auto* lhs = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
            if(out[*lhs].constant() && boost::get<double>(&out[*lhs].value())){
                if(auto* rhs = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
                    if(out[*rhs].constant() && boost::get<double>(&out[*rhs].value())){
                        out[quadruple.result] = {compute(op, boost::get<double>(out[*lhs].value()), boost::get<double>(out[*rhs].value()))};
                    } else {
                        out[quadruple.result].set_nac();
                    }
                } else if(auto* rhs = boost::get<double>(&*quadruple.arg2)){
                    out[quadruple.result] = {compute(op, boost::get<double>(out[*lhs].value()), *rhs)};
                } else {
                    out[quadruple.result].set_nac();
                }
            } else {
                out[quadruple.result].set_nac();
            }
        } else if(auto* rhs = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
            if(out[*rhs].constant() && boost::get<double>(&out[*rhs].value())){
                if(auto* lhs = boost::get<double>(&*quadruple.arg1)){
                    out[quadruple.result] = {compute(op, *lhs, boost::get<double>(out[*rhs].value()))};
                } else {
                    out[quadruple.result].set_nac();
                }
            } else {
                out[quadruple.result].set_nac();
            }
        } else {
            out[quadruple.result].set_nac();
        }

        remove_copies = quadruple.result;
    }
    //Passing a variable by pointer erases its value
    else if(op == mtac::Operator::PPARAM){
        if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
            //Impossible to know if the variable is modified or not, consider it modified
            out[*var_ptr].set_nac();

            remove_copies = *var_ptr;
        }
    } else {
        if(mtac::erase_result(op)){
            //The result is not constant at this point
            out[quadruple.result].set_nac();

            remove_copies = quadruple.result;
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
}

bool mtac::ConstantPropagationProblem::optimize(mtac::Function& function, std::shared_ptr<DataFlowResults<ProblemDomain>> global_results){
    bool optimized = false;

    for(auto& block : function){
        auto& in = global_results->IN[block];

        if(in.top()){
            continue;
        }

        for(auto& statement : block->statements){
            ConstantOptimizer optimizer(in, pointer_escaped);
            optimized |= optimizer.optimize(statement);

            transfer(block, statement, in);
        }
    }

    return optimized;
}

ProblemDomain mtac::ConstantPropagationProblem::top_element(){
    return ProblemDomain();
}

ProblemDomain mtac::ConstantPropagationProblem::default_element(){
    return ProblemDomain(ProblemDomain::Values());
}

std::ostream& mtac::operator<<(std::ostream& stream, const ConstantPropagationLattice& lattice){
    if(lattice.nac()){
        return stream << "NAC";
    } else {
        return stream << lattice.value();
    }
}

bool mtac::operator==(const mtac::Domain<ConstantPropagationValues>& lhs, const mtac::Domain<ConstantPropagationValues>& rhs){
    if(lhs.top() || rhs.top()){
        return lhs.top() == rhs.top();
    }

    auto& lhs_values = lhs.values();
    auto& rhs_values = rhs.values();

    if(lhs_values.size() != rhs_values.size()){
        return false;
    }

    for(auto& value_pair : lhs_values){
        auto rhs_find = rhs_values.find(value_pair.first);

        if(rhs_find == rhs_values.end()){
            return false;
        }

        auto& lhs_lattice = value_pair.second;
        auto& rhs_lattice = rhs_find->second;

        if(lhs_lattice.nac() != rhs_lattice.nac()){
            return false;
        }

        if(!lhs_lattice.nac() && lhs_lattice.constant() != rhs_lattice.constant()){
           return false; 
        }
    }

    return true;
}

bool mtac::operator!=(const mtac::Domain<ConstantPropagationValues>& lhs, const mtac::Domain<ConstantPropagationValues>& rhs){
    return !(lhs == rhs);
}
