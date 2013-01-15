//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>

#include "assert.hpp"
#include "Function.hpp"
#include "Variable.hpp"

#include "mtac/pure_analysis.hpp"
#include "mtac/Function.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

namespace std {

template<>
struct hash<std::reference_wrapper<mtac::Function>> {
    private:
        hash<std::string> h;

    public:
        std::size_t operator()(const std::reference_wrapper<mtac::Function>& f) const {
            return h(f.get().get_name());
        }
};

} //end of namespace std

namespace {

mtac::Function& get_function(mtac::Program& program, const Function& function){
    for(auto& f : program.functions){
        if(f.definition() == function){
            return f;
        }
    }

    eddic_unreachable("There should an MTAC function corresponding to the definition");
}

bool has_pointer_parameters(mtac::Function& function){
    for(auto& parameter : function.definition().parameters()){
        if(parameter.type()->is_pointer()){
            return true;
        }
    }

    return false;
}

bool edit_global_variables(mtac::Function& function){
    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(mtac::erase_result(quadruple.op)){
                if(quadruple.result->position().isGlobal()){
                    return true;
                }
            }
        }
    }

    return false;
}

typedef std::unordered_set<std::reference_wrapper<mtac::Function>> Functions;

bool call_unpure_function(mtac::Program& program, mtac::Function& function, Functions& analyzed){
    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.op == mtac::Operator::CALL){
                if(!quadruple.function().standard()){
                    auto& target_function = get_function(program, quadruple.function());
                    if(!target_function.pure() && analyzed.find(target_function) != analyzed.end()){
                        return true;
                    }
                } else {
                    if(!mtac::safe(quadruple.function().mangled_name())){
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool call_not_analyzed_function(mtac::Program& program, mtac::Function& function, Functions& analyzed){
    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.op == mtac::Operator::CALL){
                if(!quadruple.function().standard()){
                    if(analyzed.find(get_function(program, quadruple.function())) == analyzed.end()){
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

} //end of anonymous namespace

bool mtac::pure_analysis::operator()(mtac::Program& program){
    Functions analyzed;

    while(true){
        auto start = analyzed.size();

        for(auto& function : program.functions){
            if(has_pointer_parameters(function)){
                function.pure() = false;
                analyzed.insert(function);
                continue;
            }

            if(edit_global_variables(function)){
                function.pure() = false;
                analyzed.insert(function);
                continue;
            }

            if(call_unpure_function(program, function, analyzed)){
                function.pure() = false;
                analyzed.insert(function);
                continue;
            }

            if(call_not_analyzed_function(program, function, analyzed)){
                continue;
            }

            function.pure() = true;
            analyzed.insert(function);
        }

        if(start == analyzed.size()){
            break;
        }
    }

    for(auto& function : program.functions){
        if(analyzed.find(function) == analyzed.end()){
            function.pure() = false;
        }
    }

    //It is only an analysis pass, it is not necessary to restart the other passes
    return false;
}
