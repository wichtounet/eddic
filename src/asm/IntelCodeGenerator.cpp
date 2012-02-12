//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "asm/IntelCodeGenerator.hpp"

#include "Labels.hpp"
#include "GlobalContext.hpp"
#include "StringPool.hpp"

using namespace eddic;

as::IntelCodeGenerator::IntelCodeGenerator(AssemblyFileWriter& w) : CodeGenerator(w){}

void as::IntelCodeGenerator::generate(tac::Program& program, StringPool& pool, FunctionTable& table){
    resetNumbering();

    writeRuntimeSupport(table); 

    for(auto& function : program.functions){
        compile(function);
    }

    addStandardFunctions();

    addGlobalVariables(program.context, pool);
}

void as::IntelCodeGenerator::addGlobalVariables(std::shared_ptr<GlobalContext> context, StringPool& pool){
    defineDataSection();
     
    for(auto it : context->getVariables()){
        Type type = it.second->type();
        
        //The const variables are not stored
        if(type.isConst()){
            continue;
        }

        if(type.isArray()){
            if(type.base() == BaseType::INT){
                declareIntArray(it.second->name(), type.size());
            } else if(type.base() == BaseType::STRING){
                declareIntArray(it.second->name(), type.size());
            }
        } else {
            if (type.base() == BaseType::INT) {
                declareIntVariable(it.second->position().name(), boost::get<int>(it.second->val()));
            } else if (type.base() == BaseType::STRING) {
                auto value = boost::get<std::pair<std::string, int>>(it.second->val());
  
                //If that's not the case, there is a problem with the pool 
                assert(value.first.size() > 0);
    
                declareStringVariable(it.second->position().name(), pool.label(value.first), value.second);            
            }
        }
    }
    
    for (auto it : pool.getPool()){
        declareString(it.second, it.first);
    }
}
