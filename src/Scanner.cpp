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

Scanner::Scanner() : m_current(0), m_line(1), m_col(0) {}

char Scanner::current() const {
    return m_current;
}

int Scanner::line() const {
    return m_line;
}

int Scanner::col() const {
    return m_col;
}

bool Scanner::isAlpha() const {
    return isalpha(m_current);
}

bool Scanner::isSpace() const {
    return isspace(m_current);
}

bool Scanner::isDigit() const {
    return isdigit(m_current);
}

void Scanner::scan(const string& file) {
    stream.open(file.c_str());

    if (!stream) {
        //TODO throw CompilerException("Unable to open the input file");
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
