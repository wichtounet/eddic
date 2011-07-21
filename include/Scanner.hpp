//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SCANNER_H
#define SCANNER_H

#include <fstream>
#include <string>
#include <cctype>

namespace eddic {

class Scanner {
    private:
        std::ifstream stream;

        char m_current;
        int m_line;
        int m_col;
    public:
        Scanner() : m_current(0), m_line(1), m_col(0) {}

        /* File manipulations */
        void scan(const std::string& file);
        void close();

        /* Manage files */
        bool next();
        void pushBack();

        /* Getters */
        inline char current() const {
            return m_current;
        };
        inline int line() const {
            return m_line;
        };
        inline int col() const {
            return m_col;
        };

        /* Utility */
        inline bool isAlpha() const {
            return isalpha(m_current);
        };
        inline bool isSpace() const {
            return isspace(m_current);
        };
        inline bool isDigit() const {
            return isdigit(m_current);
        };
};

} //end of eddic

#endif
