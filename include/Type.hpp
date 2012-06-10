//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <memory>

#include <boost/optional.hpp>

#include "BaseType.hpp"

namespace eddic {

/*!
 * \struct Type
 * \brief A type descriptor.
 * Can describe any type in an EDDI source file. 
 */
class Type {
    public:
        /*!
         * Deleted copy constructor
         */
        Type(const Type& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        Type& operator=(const Type& rhs) = delete;

        virtual unsigned int elements() const;
        virtual std::string type() const;
        virtual std::shared_ptr<Type> data_type() const;

        virtual bool is_array() const;
        virtual bool is_custom_type() const;
        virtual bool is_standard_type() const;
        virtual bool is_pointer() const;
        virtual bool is_const() const;

        unsigned int size() const;
        std::shared_ptr<Type> non_const() const;

        friend bool operator==(std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs);
        friend bool operator!=(std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs);

    protected:
        Type();

        virtual BaseType base() const;
};

class StandardType : public Type {
    private:
        BaseType base_type;
        bool const_;
        
        BaseType base() const override;
    
    public:
        StandardType(BaseType type, bool const_);
    
        /*!
         * Deleted copy constructor
         */
        StandardType(const StandardType& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        StandardType& operator=(const StandardType& rhs) = delete;

        bool is_standard_type() const override;
        bool is_const() const override;
};

class CustomType : public Type {
    private:
        std::string m_type;
    
    public:
        CustomType(const std::string& type); 
    
        /*!
         * Deleted copy constructor
         */
        CustomType(const CustomType& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        CustomType& operator=(const CustomType& rhs) = delete;

        std::string type() const override;

        bool is_custom_type() const override;
};

class ArrayType : public Type {
    private:
        std::shared_ptr<Type> sub_type;
        unsigned int m_elements = 0;
    
    public:
        ArrayType(std::shared_ptr<Type> sub_type, int size = 0);
    
        /*!
         * Deleted copy constructor
         */
        ArrayType(const ArrayType& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        ArrayType& operator=(const ArrayType& rhs) = delete;

        unsigned int elements() const override;

        std::shared_ptr<Type> data_type() const override;

        bool is_array() const override;
};

class PointerType : public Type {
    private:
        std::shared_ptr<Type> sub_type;
    
    public:
        PointerType(std::shared_ptr<Type> sub_type); 
    
        /*!
         * Deleted copy constructor
         */
        PointerType(const PointerType& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        PointerType& operator=(const PointerType& rhs) = delete;

        std::shared_ptr<Type> data_type() const override;

        bool is_pointer() const override;
};

/* Relational operators  */
        
bool operator==(std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs);
bool operator!=(std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs);

extern std::shared_ptr<Type> BOOL;
extern std::shared_ptr<Type> INT;
extern std::shared_ptr<Type> FLOAT;
extern std::shared_ptr<Type> STRING;
extern std::shared_ptr<Type> VOID;

/*!
 * \brief Parse the given type into an EDDI std::shared_ptr<Type>. 
 *
 * \param type The type to parse. 
 */
std::shared_ptr<Type> new_type(const std::string& type, bool const_ = false);

std::shared_ptr<Type> new_array_type(std::shared_ptr<Type> data_type, int size = 0);

//TODO Check if still useful
bool is_standard_type(const std::string& type);

} //end of eddic

#endif
