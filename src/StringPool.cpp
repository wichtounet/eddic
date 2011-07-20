//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "StringPool.hpp"
#include "ByteCodeFileWriter.hpp"
#include "ParseNode.hpp"

#include <sstream>

using std::string;

using namespace eddic;

StringPool::StringPool() {
    currentString = 0;

    label("\"\\n\"");
}

std::string StringPool::label(const std::string& value) {
    if (pool.find(value) == pool.end()) {
        std::stringstream ss;
        ss << "S";
        ss << ++currentString;
        pool[value] = ss.str();
    }

    return pool[value];
}

void StringPool::write(ByteCodeFileWriter& writer) {
    writer.stream() << ".data" << std::endl;

    std::map<std::string, std::string>::const_iterator it;
    for (it = pool.begin(); it != pool.end(); ++it) {
        writer.stream() << it->second << ":" << std::endl;
        writer.stream() << ".string " << it->first << std::endl;
    }
}
