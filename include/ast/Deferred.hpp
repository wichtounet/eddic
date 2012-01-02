//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEFERRED_H
#define DEFERRED_H

#include <boost/intrusive_ptr.hpp>

namespace eddic {

namespace ast {

template <typename T>
inline T* Allocate(){
    return new T;
}

template <typename T>
inline void Deallocate(T* ptr){
    delete ptr;
}

template<typename T>
inline void intrusive_ptr_add_ref(T* expr){
    ++expr->references;
}

template<typename T>
inline void intrusive_ptr_release(T* expr){
    if(--expr->references == 0)
        Deallocate(expr);
}

/*!
 * \class Deferred
 * \brief Deferred construction container for AST nodes.
 *
 * This class is used to speed up the construction of the AST. The construction of the node is only done when an access to the data is issued. 
 * This code has been taken from the Epoch Compiler project (http://code.google.com/p/epoch-language/) and has been adapted a little.
 */
template <typename T>
struct Deferred {
    Deferred() : Contents(reinterpret_cast<T*>(NULL)){
        Content.Owner = this;
    }

    Deferred(const T& expr) : Contents(new (Allocate<T>()) T(expr)){
        Content.Owner = this;
    }

    Deferred(const Deferred& rhs) : Contents(rhs.Contents){
        Content.Owner = this;
    }

    template <typename VariantContentT>
    Deferred(const VariantContentT& content) : Contents(new (Allocate<T>()) T(content)){
        Content.Owner = this;
    }

    operator T () {
        if(Contents)
            return *Contents;

        return T();
    }

    operator T () const {
        if(Contents)
            return *Contents;

        return T();
    }

    Deferred& operator = (const Deferred& rhs){
        if(this != &rhs)
            Contents = rhs.Contents;

        return *this;
    }

    /*!
     * \struct SafeContentAccess
     * \brief Provide safe content access to the content of the deferred node
     */
    struct SafeContentAccess {
        T& operator * () const {
            if(!Owner->Contents)
                Owner->Contents.reset(new (Allocate<T>()) T());

            return *(Owner->Contents);
        }

        T* operator -> () const {
            if(!Owner->Contents)
                Owner->Contents.reset(new (Allocate<T>()) T());

            return Owner->Contents.get();
        }

        Deferred* Owner;
    } Content;

    protected:
    mutable boost::intrusive_ptr<T> Contents;
};

} //end of ast

} //end of eddic

#endif
