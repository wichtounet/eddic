//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "Variable.hpp"

#include "mtac/cse.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::are_equivalent(mtac::Quadruple& quadruple, const expression& exp){
    if(exp.op == quadruple.op && exp.type == quadruple.result->type()){
        if(exp.arg1 == *quadruple.arg1 && exp.arg2 == *quadruple.arg2){
            return true;
        } else if(mtac::is_commutative(quadruple.op) && exp.arg1 == *quadruple.arg2 && exp.arg2 == *quadruple.arg1){
            return true;
        }
    }

    return false;
}

bool mtac::is_interesting(mtac::Quadruple& quadruple){
    if(boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
        return true;
    }
    
    if(boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
        return true;
    }

    return false;
}

bool mtac::is_valid(mtac::Quadruple& quadruple, const mtac::escaped_variables& escaped){
    if(quadruple.op == mtac::Operator::DOT){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
            if((*ptr)->type()->is_pointer()){
                return false;
            }
        }
    }
    
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
        if(escaped.find(*ptr) != escaped.end()){
            return false;
        }
    }
    
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
        if(escaped.find(*ptr) != escaped.end()){
            return false;
        }
    }

    return true;
}

bool mtac::is_commutative(mtac::Operator op){
    return op == mtac::Operator::ADD || op == mtac::Operator::FADD || op == mtac::Operator::MUL || op == mtac::Operator::FMUL;
}

bool mtac::is_expression(mtac::Operator op){
    return (op >= mtac::Operator::ADD && op <= mtac::Operator::FDIV) || op == mtac::Operator::DOT;
}

bool mtac::is_killing(mtac::Quadruple& quadruple, const mtac::expression& expression){
    eddic_assert(quadruple.result, "is_killing should only be called on quadruple erasing the result, thus having a result");

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&expression.arg1)){
        if(quadruple.result == *ptr){
            return true;
        }
    }

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&expression.arg2)){
        if(quadruple.result == *ptr){
            return true;
        }
    }

    return false;
}

std::ostream& mtac::operator<<(std::ostream& stream, const expression& expression){
    return stream << "{" << expression.arg1 << " " << static_cast<unsigned int>(expression.op) << " " << expression.arg2 << "}";
}

mtac::Operator mtac::assign_op(mtac::Operator op){
    if(op == mtac::Operator::DOT){
        return mtac::Operator::ASSIGN;
    } else if(op <= mtac::Operator::ADD && op <= mtac::Operator::MOD){
        return mtac::Operator::ASSIGN;
    } else if(op <= mtac::Operator::FADD && op <= mtac::Operator::FDIV){
        return mtac::Operator::FASSIGN;
    }

    eddic_unreachable("This is not an expression operator");
}
