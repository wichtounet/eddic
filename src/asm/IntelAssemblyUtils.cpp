//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "asm/IntelAssemblyUtils.hpp"

#include "AssemblyFileWriter.hpp"

using namespace eddic;

void eddic::as::save(AssemblyFileWriter& writer, const std::vector<std::string>& registers){
    for(auto& reg : registers){
        writer.stream() << "push " << reg << '\n';
    }
}

void eddic::as::restore(AssemblyFileWriter& writer, const std::vector<std::string>& registers){
    auto it = registers.rbegin();
    auto end = registers.rend();

    while(it != end){
        writer.stream() << "pop " << *it << '\n';
        ++it;
    }
}
