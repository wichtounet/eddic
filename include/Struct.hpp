//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef STRUCT_H
#define STRUCT_H

#include <memory>
#include <string>
#include <vector>

namespace eddic {

class Type;

/*!
 * \class Member
 * \brief A member of a struct.  
 */
struct Member {
    std::string name;
    std::shared_ptr<const Type> type;

    Member(const std::string& n, std::shared_ptr<const Type> t);

    Member(const Member& rhs) = default;
    Member& operator=(const Member& rhs) = default;

    Member(Member&& rhs) = default;
    Member& operator=(Member&& rhs) = default;

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
    std::vector<Member> members;
    std::shared_ptr<const Type> parent_type;
   
    Struct(const std::string& n);

    Struct(const Struct& rhs) = default;
    Struct& operator=(const Struct& rhs) = default;

    Struct(Struct&& rhs) = default;
    Struct& operator=(Struct&& rhs) = default;

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
    Member& operator[](const std::string& name);

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
