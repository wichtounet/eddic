//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/GlobalArray.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

GlobalArray::GlobalArray(std::string n, BaseType t, int s) : name(n), type(t), size(s) {}

void GlobalArray::write(AssemblyFileWriter& writer){
    writer.stream() << "VA" << name << ":" <<std::endl;
    writer.stream() << ".rept " << size << std::endl;

    if(type == BaseType::INT){
        writer.stream() << ".long 0" << std::endl;
    } else if(type == BaseType::STRING){
        writer.stream() << ".long S3" << std::endl;
        writer.stream() << ".long 0" << std::endl;
    }

    writer.stream() << ".endr" << std::endl;
}
