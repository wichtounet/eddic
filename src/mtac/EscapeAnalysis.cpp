//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "Variable.hpp"

#include "mtac/EscapeAnalysis.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

mtac::escaped_variables_ptr mtac::escape_analysis(mtac::Function& function){
    auto pointer_escaped = std::make_unique<mtac::escaped_variables_ptr::element_type>();

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

    return std::move(pointer_escaped);
}
