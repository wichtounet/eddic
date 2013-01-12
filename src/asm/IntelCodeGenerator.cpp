//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "asm/IntelCodeGenerator.hpp"

#include "mtac/Program.hpp"

#include "AssemblyFileWriter.hpp"
#include "assert.hpp"
#include "Labels.hpp"
#include "GlobalContext.hpp"
#include "StringPool.hpp"
#include "Type.hpp"
#include "Variable.hpp"
#include "FloatPool.hpp"

using namespace eddic;

as::IntelCodeGenerator::IntelCodeGenerator(AssemblyFileWriter& w, std::shared_ptr<GlobalContext> context) : CodeGenerator(w), context(context) {}

void as::IntelCodeGenerator::generate(mtac::Program& program, std::shared_ptr<StringPool> pool, std::shared_ptr<FloatPool> float_pool){
    resetNumbering();

    writeRuntimeSupport(); 

    for(auto& function : program.functions){
        compile(function);
    }

    addStandardFunctions();

    addGlobalVariables(pool, float_pool);
}

void as::IntelCodeGenerator::addGlobalVariables(std::shared_ptr<StringPool> pool, std::shared_ptr<FloatPool> float_pool){
    defineDataSection();
     
    for(auto& it : context->getVariables()){
        auto type = it.second->type();
        
        //The const variables are not stored
        if(type->is_const()){
            continue;
        }

        if(type->is_array()){
            if(type->data_type() == INT || type->data_type()->is_pointer()){
                declareIntArray(it.second->name(), type->elements());
            } else if(type->data_type() == FLOAT){
                declareFloatArray(it.second->name(), type->elements());
            } else if(type->data_type() == STRING){
                declareStringArray(it.second->name(), type->elements());
            }
        } else {
            if (type == INT) {
                declareIntVariable(it.second->position().name(), boost::get<int>(it.second->val()));
            } else if (type == STRING) {
                auto value = boost::get<std::pair<std::string, int>>(it.second->val());
  
                //If that's not the case, there is a problem with the pool 
                assert(value.first.size() > 0);
    
                declareStringVariable(it.second->position().name(), pool->label(value.first), value.second);            
            }
        }
    }
    
    for (auto it : pool->getPool()){
        declareString(it.second, it.first);
    }
    
    for (auto it : float_pool->get_pool()){
        declareFloat(it.second, it.first);
    }
}

void as::IntelCodeGenerator::output_function(const std::string& function){
    std::string name = "functions/" + function + ".s";
    std::ifstream stream(name.c_str());

    eddic_assert(stream, "One file in the functions folder does not exist");

    std::string line;
    while (getline(stream, line))
    {
        if (!line.empty() &&
            (line[0] != ';'))
        {
            writer.stream() << line << '\n';
        }
    }
    
    writer.stream() << '\n';
}

bool as::IntelCodeGenerator::is_enabled_println(){
    return context->referenceCount("_F7println") || 
            context->referenceCount("_F7printlnS") || 
            context->referenceCount("_F7printlnC") || 
            context->referenceCount("_F7printlnF");
}

