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

/*!
 * \enum PositionType
 * \brief A type of position for a variable
 */
enum class PositionType : unsigned int {
    STACK,          /**< A variable on the stack  */ 
    PARAMETER,      /**< A parameter */
    GLOBAL,         /**< A global variable */
    CONST,          /**< A const variable. Not stored. Will be replaced in each usage */
    TEMPORARY       /**< A temporary, used only in three-address-code. Not stored. */
};

/*!
 * \class Position
 * \brief The position of a variable
 */
class Position {
    private:
        PositionType m_type;
        int m_offset;
        std::string m_name;

    public:
        Position(PositionType type);
        Position(PositionType type, int offset);
        Position(PositionType type, const std::string& name);

        bool isStack() const;
        bool isParameter() const;
        bool isGlobal() const;
        bool isConst() const;
        bool isTemporary() const;

        int offset() const;

        PositionType type() const;
		
        const std::string& name() const;
};

} //end of eddic

#endif
