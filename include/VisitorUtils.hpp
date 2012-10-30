//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VISITOR_UTILS_H
#define VISITOR_UTILS_H

#include <list>
#include <vector>
#include <type_traits>

#include <boost/optional/optional.hpp>

#include "variant.hpp"

#define ASSIGN(Type, Value)\
result_type operator()(Type & ){\
    return Value;\
}

#define ASSIGN_INSIDE(Visitor, Type, Value)\
Visitor::result_type Visitor::operator()(Type & ){\
    return Value;\
}

#define ASSIGN_INSIDE_CONST(Visitor, Type, Value)\
Visitor::result_type Visitor::operator()(Type & ) const {\
    return Value;\
}

#define ASSIGN_INSIDE_CONST_CONST(Visitor, Type, Value)\
Visitor::result_type Visitor::operator()(const Type & ) const {\
    return Value;\
}

namespace eddic {

/*!
 * Apply the visitor to the given object. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 * \return The result of the visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<!std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type 
visit(Visitor&& visitor, Visitable& visitable){
    return boost::apply_visitor(visitor, visitable);
}

/*!
 * Apply the visitor to the given object. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type
visit(Visitor&& visitor, Visitable& visitable){
    boost::apply_visitor(visitor, visitable);
}

/*!
 * Apply the visitor to the given object. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 * \return The result of the visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<!std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type 
visit(Visitor& visitor, Visitable& visitable){
    return boost::apply_visitor(visitor, visitable);
}

/*!
 * Apply the visitor to the given object. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type 
visit(Visitor& visitor, Visitable& visitable){
    boost::apply_visitor(visitor, visitable);
}

/*!
 * Apply the visitor to the given object. The object can be a non-variant type. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 * \return The result of the visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<!std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type 
visit_non_variant(Visitor& visitor, Visitable& visitable){
    return visitor(visitable);
}

/*!
 * Apply the visitor to the given object. The object can be a non-variant type. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type 
visit_non_variant(Visitor& visitor, Visitable& visitable){
    visitor(visitable);
}

/*!
 * Apply the visitor to the given object. The object can be a non-variant type. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 * \return The result of the visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<!std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type 
visit_non_variant(const Visitor& visitor, Visitable& visitable){
    return visitor(visitable);
}

/*!
 * Apply the visitor to the given object. The object can be a non-variant type. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type 
visit_non_variant(const Visitor& visitor, Visitable& visitable){
    visitor(visitable);
}

/*!
 * Apply the visitor to the given object. The object can be a non-variant type. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 * \return The result of the visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<!std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type 
visit_non_variant(const Visitor& visitor, const Visitable& visitable){
    return visitor(visitable);
}

/*!
 * Apply the visitor to the given object. The object can be a non-variant type. 
 * \param visitor The visitor to apply. 
 * \param visitable The object to visit. 
 */
template<typename Visitor, typename Visitable>
inline typename std::enable_if<std::is_void<typename Visitor::result_type>::value, typename Visitor::result_type>::type 
visit_non_variant(const Visitor& visitor, const Visitable& visitable){
    visitor(visitable);
}

/*!
 * Apply the visitor to the given optional object if it is initialized. 
 * \param visitor The visitor to apply. 
 * \param optional The object to visit. 
 */
template<typename Visitor, typename Visitable>
inline void visit_optional(Visitor& visitor, boost::optional<Visitable>& optional){
    if(optional){
        boost::apply_visitor(visitor, *optional);
    }
}

/*!
 * Apply the visitor to the given optional object if it is initialized. The object can be a non-variant type. 
 * \param visitor The visitor to apply. 
 * \param optional The object to visit. 
 */
template<typename Visitor, typename Visitable>
inline void visit_optional_non_variant(Visitor& visitor, boost::optional<Visitable>& optional){
    if(optional){
        visit_non_variant(visitor, *optional);
    }
}

/*!
 * Apply the visitor to each variant object inside the vector. 
 * \param visitor The visitor to apply. 
 * \param elements The elements to visit. 
 */
template<typename Visitor, typename Visitable>
inline void visit_each(Visitor& visitor, std::vector<Visitable>& elements){
    for_each(elements.begin(), elements.end(), [&](Visitable& visitable){ visit(visitor, visitable); });
}

/*!
 * Apply the visitor to each variant object inside the vector. 
 * \param visitor The visitor to apply. 
 * \param elements The elements to visit. 
 */
template<typename Visitor, typename Visitable>
inline void visit_each(Visitor& visitor, std::list<Visitable>& elements){
    for_each(elements.begin(), elements.end(), [&](Visitable& visitable){ visit(visitor, visitable); });
}

/*!
 * Apply the visitor to each variant object inside the vector. The elements can be non-variant type. 
 * \param visitor The visitor to apply. 
 * \param elements The elements to visit. 
 */
template<typename Visitor, typename Visitable>
inline void visit_each_non_variant(Visitor& visitor, std::vector<Visitable>& elements){
    for_each(elements.begin(), elements.end(), [&](Visitable& visitable){ visit_non_variant(visitor, visitable); });
}

/*!
 * Apply the visitor to each variant object inside the vector. The elements can be non-variant type. 
 * \param visitor The visitor to apply. 
 * \param elements The elements to visit. 
 */
template<typename Visitor, typename Visitable>
inline void visit_each_non_variant(Visitor& visitor, std::list<Visitable>& elements){
    for_each(elements.begin(), elements.end(), [&](Visitable& visitable){ visit_non_variant(visitor, visitable); });
}

} //end of eddic

#endif
