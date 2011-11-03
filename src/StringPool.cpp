//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "StringPool.hpp"
#include "ParseNode.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/DataSection.hpp"

#include <sstream>

using std::string;

using namespace eddic;

StringPool::StringPool() : currentString(0) {
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

void StringPool::writeIL(IntermediateProgram& program){
    program.addInstruction(program.factory().createDataSection(pool));
}
