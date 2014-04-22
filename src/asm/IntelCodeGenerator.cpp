//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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

as::IntelCodeGenerator::IntelCodeGenerator(AssemblyFileWriter& w, mtac::Program& program, std::shared_ptr<GlobalContext> context) : CodeGenerator(w, program), context(context) {}

void as::IntelCodeGenerator::generate(StringPool& pool, FloatPool& float_pool){
    resetNumbering();

    writeRuntimeSupport(); 

    for(auto& function : program){
        compile(function);
    }

    addStandardFunctions();

    addGlobalVariables(pool, float_pool);
}

void as::IntelCodeGenerator::addGlobalVariables(StringPool& pool, FloatPool& float_pool){
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
            } else if(type == STRING) {
                auto value = boost::get<std::pair<std::string, int>>(it.second->val());
  
                //If that's not the case, there is a problem with the pool 
                assert(value.first.size() > 0);
    
                declareStringVariable(it.second->position().name(), pool.label(value.first), value.second);            
            } else if(type == CHAR){
                declareCharVariable(it.second->position().name(), boost::get<char>(it.second->val()));
            } else if(type == BOOL){
                declareBoolVariable(it.second->position().name(), boost::get<bool>(it.second->val()));
            } else {
                eddic_unreachable("Unhandled type");
            }
        }
    }
    
    for (auto it : pool.getPool()){
        declareString(it.second, it.first);
    }
    
    for (auto it : float_pool.get_pool()){
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
