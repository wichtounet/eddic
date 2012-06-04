//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRUCT_H
#define STRUCT_H

#include <memory>
#include <string>
#include <vector>

#include "Types.hpp"

namespace eddic {

/*!
 * \class Member
 * \brief A member of a struct.  
 */
struct Member {
    std::string name;
    Type type;

    Member(const std::string& n, Type t);

    /*!
     * Increment the reference counter of the member. 
     */
    void add_reference();
    
    /*!
     * Return the reference counter of the member. 
     * \return The reference counter of the member. 
     */
    unsigned int get_references();

    private:
        unsigned int references = 0;
};

/*!
 * \class Struct
 * \brief A structure entry in the function table. 
 */
struct Struct {
    std::string name;
    std::vector<std::shared_ptr<Member>> members;
    
    Struct(const std::string& n);

    /*!
     * Indicates if the specified member exists in this structure. 
     * \param name The name of the member to search for. 
     * \return true if the member exists, otherwise false. 
     */
    bool member_exists(const std::string& name);
    
    /*!
     * Return the member with the specified name. 
     * \param name The name of the member to search for. 
     * \return A pointer to the member with the given name. 
     */
    std::shared_ptr<Member> operator[](const std::string& name);

    /*!
     * Increment the reference counter of the structure. 
     */
    void add_reference();
    
    /*!
     * Return the reference counter of the member. 
     * \return The reference counter of the member. 
     */
    unsigned int get_references();

    private:
        unsigned int references = 0;
};

} //end of eddic

#endif
