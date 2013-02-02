//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"

#include "mtac/cse.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::are_equivalent(mtac::Quadruple& quadruple, expression& exp){
    if(exp.op == quadruple.op && exp.type == quadruple.result->type()){
        if(exp.arg1 == *quadruple.arg1 && exp.arg2 == *quadruple.arg2){
            return true;
        } else if(mtac::is_distributive(quadruple.op) && exp.arg1 == *quadruple.arg2 && exp.arg2 == *quadruple.arg1){
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

bool mtac::is_valid(mtac::Quadruple& quadruple, mtac::EscapedVariables& escaped){
    if(quadruple.op == mtac::Operator::DOT){
        auto var = boost::get<std::shared_ptr<Variable>>(*quadruple.arg1);

        if(var->type()->is_pointer()){
            return false;
        }
    }
    
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
        if(escaped->find(*ptr) != escaped->end()){
            return false;
        }
    }
    
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
        if(escaped->find(*ptr) != escaped->end()){
            return false;
        }
    }

    return true;
}

//TODO Should be called is_commutative
bool mtac::is_distributive(mtac::Operator op){
    return op == mtac::Operator::ADD || op == mtac::Operator::FADD || op == mtac::Operator::MUL || op == mtac::Operator::FMUL;
}

bool mtac::is_expression(mtac::Operator op){
    return (op >= mtac::Operator::ADD && op <= mtac::Operator::FDIV) || op == mtac::Operator::DOT;
}
