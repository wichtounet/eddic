//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_ARGUMENT_H
#define MTAC_ARGUMENT_H

#include <memory>
#include <string>

#include "variant.hpp"

#define CUSTOM_STRONG_TYPEDEF(T, D)                                         \
struct D {                                                                  \
    T t;                                                                    \
    explicit D(const T t_) : t(t_) {};                                      \
                                                                            \
    D(){};                                                                  \
    D(const D & t_) : t(t_.t){}                                             \
                                                                            \
    D(int t_) : t(t_) {}                                                    \
    D(unsigned int t_) : t((int) t_) {}                                     \
    D(double t_) : t(t_) {}                                                 \
    D(const std::string& t_) : t(t_) {}                                     \
    D(std::shared_ptr<Variable> t_) : t(t_) {}                              \
                                                                            \
    D& operator=(const D & rhs) { t = rhs.t; return *this;}                 \
    D& operator=(const T & rhs) { t = rhs; return *this;}                   \
                                                                            \
    D& operator=(int rhs) { t = rhs; return *this; }                        \
    D& operator=(unsigned int rhs) { t = (int) rhs; return *this; }         \
    D& operator=(double rhs) { t = rhs; return *this; }                     \
    D& operator=(const std::string& rhs) { t = rhs; return *this; }         \
    D& operator=(std::shared_ptr<Variable> rhs) { t = rhs; return *this; }  \
    operator const T &() const {return t; }                                 \
    operator T &() { return t; }                                            \
                                                                            \
    T* int_ptr() {return &t; }                                              \
    const T* int_ptr() const  {return &t; }                                 \
                                                                            \
    T& int_ref() {return t; }                                               \
    const T& int_ref() const {return t; }                                   \
                                                                            \
    template<typename Visitor>                                              \
    typename Visitor::result_type apply_visitor(Visitor& visitor)           \
    { return int_ref().apply_visitor(visitor); }                            \
};

namespace eddic {
class Variable;
} //end of eddic

namespace eddi_detail {
    typedef boost::variant<std::shared_ptr<eddic::Variable>, double, int, std::string> variant_t;

    struct equals_visitor : boost::static_visitor<bool> {
        template <typename T>
        bool operator()(const T& a, const T& b) const
        { return a == b; }

        template<typename T, typename U>
        bool operator()(const T&, const U&) const
        { return false; }
    };

    struct variant_equals {
        equals_visitor _helper;

        bool operator()(const variant_t& a, const variant_t& b) const
        { return boost::apply_visitor(_helper, a, b); }
    };
} //end of eddi_detail

namespace eddic {

namespace mtac {

CUSTOM_STRONG_TYPEDEF(eddi_detail::variant_t, Argument)

bool operator==(const Argument& a, const Argument& b);
bool operator==(const Argument& a, int b);
bool operator==(const Argument& a, double b);
bool operator==(const Argument& a, std::shared_ptr<Variable> b);
bool operator==(const Argument& a, const std::string& b);

} //end of mtac

} //end of eddic

namespace boost {

template<typename T>
inline T* get(eddic::mtac::Argument* argument){
    return boost::get<T>(argument->int_ptr());
}

template<typename T>
inline const T* get(const eddic::mtac::Argument* argument){
    return boost::get<T>(argument->int_ptr());
}

template<typename T>
inline T& get(eddic::mtac::Argument& argument){
    return boost::get<T>(argument.int_ref());
}

template<typename T>
inline const T& get(const eddic::mtac::Argument& argument){
    return boost::get<T>(argument.int_ref());
}

} //end of boost

#endif
