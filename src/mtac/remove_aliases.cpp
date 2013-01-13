//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>

#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/remove_aliases.hpp"
#include "mtac/Utils.hpp"
#include "mtac/EscapeAnalysis.hpp"
#include "mtac/Printer.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

namespace {

bool is_written_once(std::shared_ptr<Variable> variable, mtac::Function& function){
    bool written = false;

    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.op == mtac::Operator::CALL){
                if(quadruple.return1() == variable || quadruple.return2() == variable){
                    if(written){
                        return false;
                    }

                    written = true;
                }
            } else if(mtac::erase_result(quadruple.op) && quadruple.result == variable){
                if(written){
                    return false;
                }

                written = true;
            }
        }
    }

    return true;
}

bool is_not_direct_alias(std::shared_ptr<Variable> source, std::shared_ptr<Variable> target, mtac::Function& function){
    for(auto& block : function){
        for(auto& quadruple : block){
            if(quadruple.op == mtac::Operator::PASSIGN && quadruple.result == source){
                if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
                    if(*var_ptr == target){
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

std::vector<std::shared_ptr<Variable>> get_targets(std::shared_ptr<Variable> variable, mtac::Function& function){
    std::vector<std::shared_ptr<Variable>> targets;
    
    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.op == mtac::Operator::ASSIGN || quadruple.op == mtac::Operator::PASSIGN || quadruple.op == mtac::Operator::PASSIGN){
                if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
                    if(*var_ptr == variable){
                        targets.push_back(quadruple.result); 
                    }
                }
            }
        }
    }

    return targets;
}

std::vector<std::shared_ptr<Variable>> get_sources(std::shared_ptr<Variable> variable, mtac::Function& function){
    std::vector<std::shared_ptr<Variable>> sources;
    
    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.op == mtac::Operator::ASSIGN || quadruple.op == mtac::Operator::PASSIGN || quadruple.op == mtac::Operator::PASSIGN){
                if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
                    if(quadruple.result == variable){
                        sources.push_back(*var_ptr); 
                    }
                }
            }
        }
    }

    return sources;
}

struct VariableReplace {
    std::shared_ptr<Variable> source;
    std::shared_ptr<Variable> target;

    bool find_first = false;
    bool invalid = false;
    bool reverse = false;

    VariableReplace(std::shared_ptr<Variable> source, std::shared_ptr<Variable> target) : source(source), target(target) {}
    VariableReplace(const VariableReplace& rhs) = delete;

    void guard(mtac::Quadruple& quadruple){
        if(!reverse){
            if(quadruple.result == source || quadruple.secondary == source){
                find_first = true;
                return;
            } 

            if(find_first && (quadruple.result == target || quadruple.secondary == target)){
                invalid = true;
            }
        }
    }

    inline bool optimize_optional(boost::optional<mtac::Argument>& arg){
        if(arg){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*arg)){
                if(*ptr == source){
                    arg = target;
                    return true;
                }
            }
        }

        return false;
    }

    bool optimize(mtac::Quadruple& quadruple){
        guard(quadruple);
        
        if(invalid){
            return false;
        }

        bool optimized = false;

        //Do not replace source by target in the lhs of an assign
        //because it can be invalidated lated
        //and it will removed by other passes if still there
        
        if(quadruple.op == mtac::Operator::CALL){
            if(quadruple.result == source){
                quadruple.result = target;
                optimized = true;
            }
            
            if(quadruple.secondary == source){
                quadruple.secondary = target;
                optimized = true;
            }
        } else if(reverse || !mtac::erase_result(quadruple.op)){
            if(quadruple.result == source){
                quadruple.result = target;
                optimized = true;
            }
        }

        return optimized | optimize_optional(quadruple.arg1) | optimize_optional(quadruple.arg2);
    }
};

}

bool mtac::remove_aliases::operator()(mtac::Function& function){
    bool optimized = false;

    auto pointer_escaped = mtac::escape_analysis(function);

    for(auto& var : function.context->stored_variables()){
        auto position = var->position();
        auto type = var->type();

        if((position.is_temporary() || position.is_variable() || position.isStack()) && (type->is_standard_type() || type->is_pointer()) && type != STRING){
            if(is_written_once(var, function)){
                auto targets = get_targets(var, function);

                if(targets.size() == 1){
                    if(pointer_escaped->find(var) == pointer_escaped->end()){
                        if(is_not_direct_alias(var, targets[0], function) && targets[0]->type() != STRING){
                            VariableReplace replacer(var, targets[0]);
                            replacer.reverse = true;

                            for(auto& block : function){
                                for(auto& statement : block->statements){
                                    optimized |= replacer.optimize(statement);
                                }
                            }

                            continue;
                        }
                    }
                } 

                //TODO Perhaps not valid anymore with the new properties of the temporaries
                if(position.is_temporary()){
                    auto sources = get_sources(var, function);

                    if(pointer_escaped->find(var) == pointer_escaped->end()){
                        if(sources.size() == 1){
                            if(is_not_direct_alias(var, sources[0], function) && sources[0]->type() != STRING){
                                VariableReplace replacer(var, sources[0]);

                                for(auto& block : function){
                                    for(auto& statement : block->statements){
                                        optimized |= replacer.optimize(statement);
                                    }
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
