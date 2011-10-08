//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SCANNER_H
#define SCANNER_H

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>

namespace eddic {

class Scanner {
    private:
        std::ifstream stream;
        std::filebuf* buffer;

        char m_current;
        int m_line;
        int m_col;

    public:
        Scanner();

        /* File manipulations */
        void scan(const std::string& file);
        void close();

        /* Manage files */
        bool next();
        void pushBack();

        /* Getters */
        char current() const ;
        int line() const ;
        int col() const ;

        /* Utility */
        bool isAlpha() const ;
        bool isSpace() const ;
        bool isDigit() const ;
};

} //end of eddic

#endif
