//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"
#include "iterators.hpp"
#include "FunctionContext.hpp"

#include "mtac/reference_resolver.hpp"
#include "mtac/Program.hpp"
#include "mtac/Function.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Printer.hpp"

using namespace eddic;

mtac::Argument variant_cast(Offset source){
    if(auto* ptr = boost::get<int>(&source)){
        return *ptr;
    } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&source)){
        return *ptr;
    } else {
        eddic_unreachable("Invalid source type");
    }
}

void mtac::resolve_references(mtac::program_p program){
    for(auto& function : program->functions){
        //This pass is run before basic blocks are extracted
        auto& statements = function->getStatements();

        auto it = iterate(statements);

        std::unordered_set<std::shared_ptr<Variable>> variables;

        while(it.has_next()){
            auto& statement = *it;

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto& quadruple = *ptr;

                //x = (r)z => x = (ref(r))(z+offset(r))
                if(quadruple->op == mtac::Operator::DOT && mtac::optional_is<std::shared_ptr<Variable>>(quadruple->arg1)){
                    auto var = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

                    if(var->is_reference()){
                        auto temp = function->context->new_temporary(INT);
                        auto index = *quadruple->arg2;

                        //The reference itself is replaced by its pointed var
                        quadruple->arg1 = var->reference();

                        //The new offset is the sum of the old ones
                        quadruple->arg2 = temp;

                        it.insert(std::make_shared<mtac::Quadruple>(temp, index, mtac::Operator::ADD, variant_cast(var->reference_offset())));
                        ++it;
                    }
                } 
                //(r)z = x => (ref(r))(z+offset(r)) = x
                else if(quadruple->op == mtac::Operator::DOT_ASSIGN && quadruple->result->is_reference()){
                    auto temp = function->context->new_temporary(INT);
                    auto index = *quadruple->arg1;
                    auto var = quadruple->result;

                    //The reference itself is replaced by its pointed var
                    quadruple->result = var->reference();

                    //The new offset is the sum of the old ones
                    quadruple->arg1 = temp;

                    it.insert(std::make_shared<mtac::Quadruple>(temp, index, mtac::Operator::ADD, variant_cast(var->reference_offset())));
                    ++it;
                } 
                //ref = x
                
                if(mtac::erase_result(quadruple->op)){
                    auto result = quadruple->result;

                    if(result->is_reference()){
                        //The first times a variable is encountered, it is its initialization
                        if(variables.find(result) == variables.end()){
                            variables.insert(result);
                        } else {
                            if(result->type() == FLOAT){
                                it.insert_after(std::make_shared<mtac::Quadruple>(result->reference(), variant_cast(result->reference_offset()), mtac::Operator::DOT_FASSIGN, result));
                            } else if(result->type()->is_pointer()){
                                it.insert_after(std::make_shared<mtac::Quadruple>(result->reference(), variant_cast(result->reference_offset()), mtac::Operator::DOT_PASSIGN, result));
                            } else {
                                it.insert_after(std::make_shared<mtac::Quadruple>(result->reference(), variant_cast(result->reference_offset()), mtac::Operator::DOT_ASSIGN, result));
                            }
                        }
                    }
                }
            }

            ++it;
        }
    }
}
