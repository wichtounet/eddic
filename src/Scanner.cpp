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

    if (!stream) {
        throw CompilerException("Unable to open the input file");
    }

    buffer = stream.rdbuf();
}

void Scanner::close() {
    stream.close();
}

bool Scanner::next() {
    if (buffer->sgetc() == EOF) {
        return false;
    }

    if (m_current == '\n') {
        ++m_line;
        m_col = 1;
    } else {
        ++m_col;
    }

    m_current = buffer->sbumpc();

    return true;
}

void Scanner::pushBack() {
    buffer->sputbackc(m_current);
}
