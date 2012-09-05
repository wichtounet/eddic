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
#include <vector>

#include <boost/optional.hpp>

#include "BaseType.hpp"

namespace eddic {

class GlobalContext;

/*!
 * \class Type
 * \brief A type descriptor.
 * Can describe any type in an EDDI source file. 
 */
class Type : public std::enable_shared_from_this<Type> {
    public:
        /*!
         * Deleted copy constructor
         */
        Type(const Type& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        Type& operator=(const Type& rhs) = delete;

        /*!
         * Return the number of elements of the array type. 
         * \return The number of elements. 
         */
        virtual unsigned int elements() const;

        /*!
         * Return the name of the struct type. 
         * \return the name of the struct;
         */
        virtual std::string type() const;

        /*!
         * Return the data type. In the case of an array it is the type of the elements and 
         * in the case of a pointer, it is the type of the pointed element. 
         * \return the data type
         */
        virtual std::shared_ptr<const Type> data_type() const;
        
        /*
         * Return the template types of a template type. 
         * \return A vector containing all the template types of the current type. 
         */
        virtual std::vector<std::shared_ptr<const Type>> template_types() const;

        /*!
         * Indicates if it is an array type
         * \return true if it's an array type, false otherwise.
         */
        virtual bool is_array() const;

        /*!
         * Indicates if it is a custom type
         * \return true if it's a custom type, false otherwise.
         */
        virtual bool is_custom_type() const;

        /*!
         * Indicates if it is a standard type
         * \return true if it's a standard type, false otherwise.
         */
        virtual bool is_standard_type() const;

        /*!
         * Indicates if it is a pointer type
         * \return true if it's a pointer type, false otherwise.
         */
        virtual bool is_pointer() const;

        /*!
         * Indicates if the type is const
         * \return true if the type is const, false otherwise.
         */
        virtual bool is_const() const;

        /*!
         * Indicates if the type is a template type. 
         * \return true if the type is a template type, false otherwise.
         */
        virtual bool is_template() const;

        /*!
         * Return the size of the type in memory in octets. 
         * \return the size of the type, in octets.
         */
        virtual unsigned int size() const;

        /*!
         * Return the mangled name of the type. 
         * \return The mangled name of the type. 
         */
        std::string mangle() const;

        /*!
         * Return a non_const copy of the type. If the type is already non-const, a pointer to the current type is returned. 
         * \return a non-const version of this type;
         */
        std::shared_ptr<const Type> non_const() const;

        friend bool operator==(std::shared_ptr<const Type> lhs, std::shared_ptr<const Type> rhs);
        friend bool operator!=(std::shared_ptr<const Type> lhs, std::shared_ptr<const Type> rhs);

    protected:
        /*!
         * Construct a new Type. 
         */
        Type();
        
        /*!
         * Return the base type of a standard type
         * \return the base type.
         */
        virtual BaseType base() const;
};

/*!
 * \class StandardType
 * \brief A standard type descriptor.
 */
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
        
        unsigned int size() const override;
};

/*!
 * \class CustomType
 * \brief A custom type descriptor.
 */
class CustomType : public Type {
    private:
        std::shared_ptr<GlobalContext> context;
        std::string m_type;
    
    public:
        CustomType(std::shared_ptr<GlobalContext> context, const std::string& type); 
    
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
        
        unsigned int size() const override;
};

/*!
 * \class ArrayType
 * \brief An array type descriptor.
 */
class ArrayType : public Type {
    private:
        std::shared_ptr<const Type> sub_type;
        unsigned int m_elements = 0;
    
    public:
        ArrayType(std::shared_ptr<const Type> sub_type, int size = 0);
    
        /*!
         * Deleted copy constructor
         */
        ArrayType(const ArrayType& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        ArrayType& operator=(const ArrayType& rhs) = delete;

        unsigned int elements() const override;

        std::shared_ptr<const Type> data_type() const override;

        bool is_array() const override;
        
        unsigned int size() const override;
};

/*!
 * \class PointerType
 * \brief A pointer type descriptor.
 */
class PointerType : public Type {
    private:
        std::shared_ptr<const Type> sub_type;
    
    public:
        PointerType(std::shared_ptr<const Type> sub_type); 
    
        /*!
         * Deleted copy constructor
         */
        PointerType(const PointerType& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        PointerType& operator=(const PointerType& rhs) = delete;

        std::shared_ptr<const Type> data_type() const override;

        bool is_pointer() const override;
        
        unsigned int size() const override;
};

/*!
 * \class TemplateType
 * \brief A template type descriptor.
 */
class TemplateType : public Type {
    private:
        std::shared_ptr<GlobalContext> context;
        std::string main_type;
        std::vector<std::shared_ptr<const Type>> sub_types;
    
    public:
        TemplateType(std::shared_ptr<GlobalContext> context, std::string main_type, std::vector<std::shared_ptr<const Type>> sub_types); 
    
        /*!
         * Deleted copy constructor
         */
        TemplateType(const PointerType& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        TemplateType& operator=(const PointerType& rhs) = delete;

        std::string type() const override;
        std::vector<std::shared_ptr<const Type>> template_types() const override;

        bool is_template() const override;
        
        unsigned int size() const override;
};

/* Relational operators  */
        
bool operator==(std::shared_ptr<const Type> lhs, std::shared_ptr<const Type> rhs);
bool operator!=(std::shared_ptr<const Type> lhs, std::shared_ptr<const Type> rhs);

extern std::shared_ptr<const Type> BOOL;
extern std::shared_ptr<const Type> INT;
extern std::shared_ptr<const Type> CHAR;
extern std::shared_ptr<const Type> FLOAT;
extern std::shared_ptr<const Type> STRING;
extern std::shared_ptr<const Type> VOID;

/*!
 * \brief Parse the given type into an EDDI std::shared_ptr<Type>. 
 *
 * \param context The current global context
 * \param type The type to parse. 
 */
std::shared_ptr<const Type> new_type(std::shared_ptr<GlobalContext> context, const std::string& type, bool const_ = false);

/*!
 * Create a new array type of the given type.
 * \param data_type The type of data hold by the array. 
 * \param size The number of elements, if known.
 * \return the created type;
 */
std::shared_ptr<const Type> new_array_type(std::shared_ptr<const Type> data_type, int size = 0);

/*!
 * Create a new pointer type of the given type.
 * \param data_type The type of data pointed. 
 * \return the created type;
 */
std::shared_ptr<const Type> new_pointer_type(std::shared_ptr<const Type> data_type);

std::shared_ptr<const Type> new_template_type(std::shared_ptr<GlobalContext> context, std::string data_type, std::vector<std::shared_ptr<const Type>> template_types);

/*!
 * Indicates if the given type is a standard type or not. 
 * \param type The type to test. 
 * \return true if the type is standard, false otherwise.
 */
bool is_standard_type(const std::string& type);

} //end of eddic

#endif
