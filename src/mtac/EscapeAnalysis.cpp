//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"

#include "mtac/EscapeAnalysis.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

mtac::EscapedVariables mtac::escape_analysis(mtac::Function& function){
    mtac::EscapedVariables pointer_escaped = std::make_shared<mtac::EscapedVariables::element_type>();

    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.op == mtac::Operator::PASSIGN){
                if(quadruple.arg1 && mtac::isVariable(*quadruple.arg1)){
                    auto var = boost::get<std::shared_ptr<Variable>>(*quadruple.arg1);
                    pointer_escaped->insert(var);
                }
            } else if(quadruple.op == mtac::Operator::DOT_PASSIGN){
                if(quadruple.arg2 && mtac::isVariable(*quadruple.arg2)){
                    auto var = boost::get<std::shared_ptr<Variable>>(*quadruple.arg2);
                    pointer_escaped->insert(var);
                }
            } else if(quadruple.op == mtac::Operator::PDOT){
                if(quadruple.arg1 && mtac::isVariable(*quadruple.arg1)){
                    auto var = boost::get<std::shared_ptr<Variable>>(*quadruple.arg1);
                    pointer_escaped->insert(var);
                }
            } else if(quadruple.op == mtac::Operator::PPARAM){
                if(mtac::isVariable(*quadruple.arg1)){
                    auto var = boost::get<std::shared_ptr<Variable>>(*quadruple.arg1);
                    pointer_escaped->insert(var);
                }
            }
        }
    }

    return pointer_escaped;
}
