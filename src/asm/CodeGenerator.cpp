//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <iostream>

#include <boost/variant.hpp>

#include "AssemblyFileWriter.hpp"
#include "Types.hpp"
#include "GlobalContext.hpp"
#include "Variable.hpp"
#include "StringPool.hpp"

#include "asm/CodeGenerator.hpp"

using namespace eddic;

as::CodeGenerator::CodeGenerator(AssemblyFileWriter& w) : writer(w){}

void as::CodeGenerator::addGlobalVariables(std::shared_ptr<GlobalContext> context, StringPool& pool){
    writer.stream() << std::endl << "section .data" << std::endl;
     
    for(auto it : context->getVariables()){
        Type type = it.second->type();
        
        //The const variables are not stored
        if(type.isConst()){
            continue;
        }

        if(type.isArray()){
            writer.stream() << "V" << it.second->position().name() << ":" <<std::endl;
            writer.stream() << "%rep " << type.size() << std::endl;

            if(type.base() == BaseType::INT){
                writer.stream() << "dd 0" << std::endl;
            } else if(type.base() == BaseType::STRING){
                writer.stream() << "dd S3" << std::endl;
                writer.stream() << "dd 0" << std::endl;
            }

            writer.stream() << "%endrep" << std::endl;
            writer.stream() << "dd " << type.size() << std::endl;
        } else {
            if (type.base() == BaseType::INT) {
                writer.stream() << "V" << it.second->position().name() << " dd " << boost::get<int>(it.second->val()) << std::endl;
            } else if (type.base() == BaseType::STRING) {
                auto value = boost::get<std::pair<std::string, int>>(it.second->val());
  
                //If that's not the case, there is a problem with the pool 
                assert(value.first.size() > 0);
                
                writer.stream() << "V" << it.second->position().name() << " dd " << pool.label(value.first) << ", " << value.second << std::endl;
            }
        }
    }
    
    for (auto it : pool.getPool()){
        writer.stream() << it.second << " dd " << it.first  << std::endl;
    }
}
