//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VISITOR_UTILS_H
#define VISITOR_UTILS_H

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <boost/optional/optional.hpp>
#include <boost/utility/enable_if.hpp>

#define ASSIGN(Type, Value)\
result_type operator()(Type & ){\
    return Value;\
}

#define ASSIGN_INSIDE(Visitor, Type, Value)\
result_type Visitor::operator()(Type & ){\
    return Value;\
}

namespace eddic {

/* non-const-non-const version */

template<typename Visitor, typename Visitable>
inline typename boost::disable_if<boost::is_void<typename Visitor::result_type>, typename Visitor::result_type>::type 
visit(Visitor& visitor, Visitable& visitable){
    return boost::apply_visitor(visitor, visitable);
}

template<typename Visitor, typename Visitable>
inline void visit(Visitor& visitor, Visitable& visitable){
    boost::apply_visitor(visitor, visitable);
}

/* const-const version */

template<typename Visitor, typename Visitable>
inline typename boost::disable_if<boost::is_void<typename Visitor::result_type>, typename Visitor::result_type>::type 
visit(const Visitor& visitor, const Visitable& visitable){
    return boost::apply_visitor(visitor, visitable);
}

template<typename Visitor, typename Visitable>
inline void visit(const Visitor& visitor, const Visitable& visitable){
    boost::apply_visitor(visitor, visitable);
}

/* non const non variant version */

template<typename Visitor, typename Visitable>
inline typename boost::disable_if<boost::is_void<typename Visitor::result_type>, typename Visitor::result_type>::type 
visit_non_variant(Visitor& visitor, Visitable& visitable){
    return visitor(visitable);
}

template<typename Visitor, typename Visitable>
inline void visit_non_variant(Visitor& visitor, Visitable& visitable){
    visitor(visitable);
}

/* const non variant version */

template<typename Visitor, typename Visitable>
inline typename boost::disable_if<boost::is_void<typename Visitor::result_type>, typename Visitor::result_type>::type 
visit_non_variant(const Visitor& visitor, const Visitable& visitable){
    return visitor(visitable);
}

template<typename Visitor, typename Visitable>
inline void visit_non_variant(const Visitor& visitor, const Visitable& visitable){
    visitor(visitable);
}

/* optional versions : no return */
 
template<typename Visitor, typename Visitable>
inline void visit_optional(Visitor& visitor, boost::optional<Visitable>& optional){
    if(optional){
        boost::apply_visitor(visitor, *optional);
    }
}

template<typename Visitor, typename Visitable>
inline void visit_optional_non_variant(Visitor& visitor, boost::optional<Visitable>& optional){
    if(optional){
        visit_non_variant(visitor, *optional);
    }
}

/* Visit a set : only void version */

template<typename Visitor, typename Visitable>
inline void visit_each(Visitor& visitor, std::vector<Visitable>& elements){
    for_each(elements.begin(), elements.end(), [&](Visitable& visitable){ visit(visitor, visitable); });
}

template<typename Visitor, typename Visitable>
inline void visit_each_non_variant(Visitor& visitor, std::vector<Visitable>& elements){
    for_each(elements.begin(), elements.end(), [&](Visitable& visitable){ visit_non_variant(visitor, visitable); });
}

} //end of eddic

#endif
