//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef POSITION_H
#define POSITION_H

#include <string>

namespace eddic {

enum PositionType {
    STACK, 
    PARAMETER,
    GLOBAL
};

class Position {
    private:
        const PositionType m_type;
        const int m_offset;
        const std::string m_name;

    public:
        Position(PositionType type, int offset);
        Position(PositionType type, std::string name);

        bool isStack();
        bool isParameter();
        bool isGlobal();
        int offset();
        std::string name();
};

} //end of eddic

#endif
