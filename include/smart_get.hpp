//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef EDDIC_BOOST_SMART_GET_H
#define EDDIC_BOOST_SMART_GET_H

namespace boost {

template <typename T, typename ...Types>
struct x3_smart_get {
    static inline T const& get(boost::spirit::x3::variant<Types...> const& x){
        return boost::get<T>(x.get());
    }

    static inline T& get(boost::spirit::x3::variant<Types...>& x){
        return boost::get<T>(x.get());
    }

    static inline const T* get(boost::spirit::x3::variant<Types...> const* x){
        return boost::get<T>(&x->get());
    }

    static inline T* get(boost::spirit::x3::variant<Types...>* x){
        return boost::get<T>(&x->get());
    }
};

} //end of namespace boost

#define X3_FORWARD_AST(Type) \
namespace boost { \
template <typename ...Types> \
struct x3_smart_get<Type, Types...> { \
    static inline const Type& get(boost::spirit::x3::variant<Types...> const& x){ \
        return boost::get<x3::forward_ast<Type>>(x.get()).get(); \
    } \
    static inline Type& get(boost::spirit::x3::variant<Types...>& x){ \
        return boost::get<x3::forward_ast<Type>>(x.get()).get(); \
    } \
    static inline const Type* get(boost::spirit::x3::variant<Types...> const* x){ \
        return boost::get<x3::forward_ast<Type>>(&x->get())->get_pointer(); \
    } \
    static inline Type* get(boost::spirit::x3::variant<Types...>* x){ \
        return boost::get<x3::forward_ast<Type>>(&x->get())->get_pointer(); \
    } \
};\
}

namespace boost {

//x3::variant versions delegate to x3_smart_get

template <typename T, typename ...Types>
inline const T& smart_get(boost::spirit::x3::variant<Types...> const& x){
    return boost::x3_smart_get<T, Types...>::get(x);
}

template <typename T, typename ...Types>
inline T& smart_get(boost::spirit::x3::variant<Types...>& x){
    return boost::x3_smart_get<T, Types...>::get(x);
}

template <typename T, typename ...Types>
inline const T* smart_get(boost::spirit::x3::variant<Types...> const* x){
    return boost::x3_smart_get<T, Types...>::get(x);
}

template <typename T, typename ...Types>
inline T* smart_get(boost::spirit::x3::variant<Types...>* x){
    return boost::x3_smart_get<T, Types...>::get(x);
}

//boost::variant versions simply delegate to boost::get

template <typename T, typename ...Types>
inline const T & smart_get(boost::variant<Types...> const& x){
    return boost::get<T>(x);
}

template <typename T, typename ...Types>
inline T& smart_get(boost::variant<Types...>& x){
    return boost::get<T>(x);
}

template <typename T, typename ...Types>
inline T const* smart_get(boost::variant<Types...> const* x){
    return boost::get<T>(x);
}

template <typename T, typename ...Types>
inline T* smart_get(boost::variant<Types...>* x){
    return boost::get<T>(x);
}

} //end of namespace boost



#endif
