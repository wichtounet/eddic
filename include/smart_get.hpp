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

    static inline T const& relaxed_get(boost::spirit::x3::variant<Types...> const& x){
        return boost::relaxed_get<T>(x.get());
    }

    static inline T& relaxed_get(boost::spirit::x3::variant<Types...>& x){
        return boost::relaxed_get<T>(x.get());
    }

    static inline const T* relaxed_get(boost::spirit::x3::variant<Types...> const* x){
        return boost::relaxed_get<T>(&x->get());
    }

    static inline T* relaxed_get(boost::spirit::x3::variant<Types...>* x){
        return boost::relaxed_get<T>(&x->get());
    }
};

} //end of namespace boost

#define X3_FORWARD_AST(MacroType) \
namespace boost { \
template <typename ...Types> \
struct x3_smart_get<MacroType, Types...> { \
    static inline const MacroType& get(boost::spirit::x3::variant<Types...> const& x){ \
        return boost::get<x3::forward_ast<MacroType>>(x.get()).get(); \
    } \
    static inline MacroType& get(boost::spirit::x3::variant<Types...>& x){ \
        return boost::get<x3::forward_ast<MacroType>>(x.get()).get(); \
    } \
    static inline const MacroType* get(boost::spirit::x3::variant<Types...> const* x){ \
        auto* ptr = boost::get<x3::forward_ast<MacroType>>(&x->get()); \
        return ptr ? ptr->get_pointer() : nullptr; \
    } \
    static inline MacroType* get(boost::spirit::x3::variant<Types...>* x){ \
        auto* ptr = boost::get<x3::forward_ast<MacroType>>(&x->get()); \
        return ptr ? ptr->get_pointer() : nullptr; \
    } \
    static inline const MacroType& relaxed_get(boost::spirit::x3::variant<Types...> const& x){ \
        return boost::relaxed_get<x3::forward_ast<MacroType>>(x.get()).get(); \
    } \
    static inline MacroType& relaxed_get(boost::spirit::x3::variant<Types...>& x){ \
        return boost::relaxed_get<x3::forward_ast<MacroType>>(x.get()).get(); \
    } \
    static inline const MacroType* relaxed_get(boost::spirit::x3::variant<Types...> const* x){ \
        auto* ptr = boost::relaxed_get<x3::forward_ast<MacroType>>(&x->get()); \
        return ptr ? ptr->get_pointer() : nullptr; \
    } \
    static inline MacroType* relaxed_get(boost::spirit::x3::variant<Types...>* x){ \
        auto* ptr = boost::relaxed_get<x3::forward_ast<MacroType>>(&x->get()); \
        return ptr ? ptr->get_pointer() : nullptr; \
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

//x3::variant versions delegate to x3_smart_get

template <typename T, typename ...Types>
inline const T& smart_relaxed_get(boost::spirit::x3::variant<Types...> const& x){
    return boost::x3_smart_get<T, Types...>::relaxed_get(x);
}

template <typename T, typename ...Types>
inline T& smart_relaxed_get(boost::spirit::x3::variant<Types...>& x){
    return boost::x3_smart_get<T, Types...>::relaxed_get(x);
}

template <typename T, typename ...Types>
inline const T* smart_relaxed_get(boost::spirit::x3::variant<Types...> const* x){
    return boost::x3_smart_get<T, Types...>::relaxed_get(x);
}

template <typename T, typename ...Types>
inline T* smart_relaxed_get(boost::spirit::x3::variant<Types...>* x){
    return boost::x3_smart_get<T, Types...>::relaxed_get(x);
}

//boost::variant versions simply delegate to boost::relaxed_get

template <typename T, typename ...Types>
inline const T & smart_relaxed_get(boost::variant<Types...> const& x){
    return boost::relaxed_get<T>(x);
}

template <typename T, typename ...Types>
inline T& smart_relaxed_get(boost::variant<Types...>& x){
    return boost::relaxed_get<T>(x);
}

template <typename T, typename ...Types>
inline T const* smart_relaxed_get(boost::variant<Types...> const* x){
    return boost::relaxed_get<T>(x);
}

template <typename T, typename ...Types>
inline T* smart_relaxed_get(boost::variant<Types...>* x){
    return boost::relaxed_get<T>(x);
}

} //end of namespace boost

#endif
