//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef POSITION_H
#define POSITION_H

#include <string>
#include <boost/optional.hpp>

namespace eddic {

/*!
 * \enum PositionType
 * \brief A type of position for a variable
 */
enum class PositionType : unsigned int {
    STACK,              /**< A variable on the stack  */ 
    PARAMETER,          /**< A parameter */
    GLOBAL,             /**< A global variable */
    CONST,              /**< A const variable. Not stored. Will be replaced in each usage */
    VARIABLE,           /**< A variable that has not been allocated a stack position */
    TEMPORARY,          /**< A temporary, with no stack position */
    REGISTER,           /**< A variable stored in a register. */
    PARAM_REGISTER      /**< A param stored in a register. */
};

/*!
 * \class Position
 * \brief The position of a variable
 */
class Position {
    private:
        PositionType m_type;
        
        boost::optional<int> m_offset;
        boost::optional<std::string> m_name;

    public:
        Position(PositionType type);
        Position(PositionType type, int offset);
        Position(PositionType type, const std::string& name);

        /*!
         * Indicates if this position is on the stack. 
         * \return true if this position is on the stack, otherwise false. 
         */
        bool isStack() const;

        /*!
         * Indicates if this position is a parameter. 
         * \return true if this position is a parameter, otherwise false. 
         */
        bool isParameter() const;

        /*!
         * Indicates if this position is a global variable. 
         * \return true if this position is a global variable, otherwise false. 
         */
        bool isGlobal() const;

        /*!
         * Indicates if this position is const. 
         * \return true if this position is const, otherwise false. 
         */
        bool is_const() const;

        /*!
         * Indicates if this position is temporary. 
         * \return true if this position is temporary, otherwise false. 
         */
        bool is_temporary() const;

        /*!
         * Indicates if this position is a parameter stored in a register. 
         * \return true if this position is a parameter stored in a register, otherwise false. 
         */
        bool isParamRegister() const;

        /*!
         * Indicates if this position is a variable stored in a register. 
         * \return true if this position is a variable stored in a register, otherwise false. 
         */
        bool is_register() const;

        /*!
         * Indicates if this position is a variable, not yet allocated a stack position.
         * \return true if this position is a variable, otherwise false. 
         */
        bool is_variable() const;

        /*!
         * Returns the type of the position. 
         * \return The type of the position
         */
        PositionType type() const;

        /*!
         * Returns the offset of the variable. This value is only valid for position having a offset. 
         * \return the offset of the variable. 
         */
        int offset() const;

        /*!
         * Returns the name of the variable. This value is only valid for position having a name. 
         * \return the name of the variable. 
         */
        const std::string& name() const;
};

} //end of eddic

#endif
