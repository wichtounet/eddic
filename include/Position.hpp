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
        Position(PositionType type, int offset) : m_type(type), m_offset(offset), m_name("") {}
        Position(PositionType type, std::string name) : m_type(type), m_offset(0), m_name(name) {}

        bool isStack(){
            return m_type == STACK;
        }

        bool isParameter(){
            return m_type == PARAMETER;
        }

        bool isGlobal(){
            return m_type == GLOBAL;
        }

        int offset(){
            return m_offset;
        }

        std::string name(){
            return m_name;
        }
};

} //end of eddic

#endif
