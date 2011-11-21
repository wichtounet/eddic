//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEFERRED_H
#define DEFERRED_H

#include <memory>

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

template <typename T, typename PointerType = std::shared_ptr<T>>
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
    mutable PointerType Contents;
};

} //end of ast

} //end of eddic

#endif
