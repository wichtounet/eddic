//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
