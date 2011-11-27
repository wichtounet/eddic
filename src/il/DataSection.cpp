//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/DataSection.hpp"

#include "AssemblyFileWriter.hpp"

using namespace eddic;

DataSection::DataSection(std::unordered_map<std::string, std::string> pool) : m_pool(pool) {}

void DataSection::write(AssemblyFileWriter& writer) const {
    writer.stream() << std::endl;
    writer.stream() << ".data" << std::endl;

    for (auto it : m_pool){
        writer.stream() << it.second << ":" << std::endl;
        writer.stream() << ".string " << it.first << std::endl;
    }
}
