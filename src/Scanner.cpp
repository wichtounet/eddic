//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "Scanner.hpp"
#include "CompilerException.hpp"

using std::pair;
using std::string;
using std::ios_base;

using namespace eddic;

void Scanner::scan(const string& file) {
    stream.open(file.c_str());
    stream.unsetf(ios_base::skipws);

    if (!stream) {
        throw CompilerException("Unable to open the input file");
    }
}

void Scanner::close() {
    stream.close();
}

bool Scanner::next() {
    if (stream.eof()) {
        return false;
    }
    
    if (currentChar == '\n') {
        ++line;
        col = 1;
    } else {
        ++col;
    }
    
    stream >> currentChar;

    return true;
}

void Scanner::pushBack() {
    stream.putback(currentChar);
}
