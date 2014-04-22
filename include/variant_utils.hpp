//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef VARIANT_UTILS_H
#define VARIANT_UTILS_H

namespace eddic {

/*!
 * \brief Execute the given functor if the given variant is of the given template type. 
 * \param variant The variant. 
 * \param functor The functor to execute. Must have an operator()(T).
 * \tparam T The type that must be contained in the variant. 
 * \tparam Variant The variant type. 
 * \tparam Functor the type of the functor. 
 */
template<typename T, typename Variant, typename Functor>
void if_type(Variant& variant, Functor&& functor){
    if(auto* ptr = boost::get<T>(&*variant)){
        functor(*ptr);
    }
}

/*!
 * \brief Execute the given functor if the given optional variant is initialized and of the given template type. 
 * \param opt_variant The optional variant. 
 * \param functor The functor to execute. Must have an operator()(T).
 * \tparam T The type that must be contained in the variant. 
 * \tparam OptVariant The optional variant type. 
 * \tparam Functor the type of the functor. 
 */
template<typename T, typename OptVariant, typename Functor>
void if_init(OptVariant& opt_variant, Functor&& functor){
    if(opt_variant){
        if(auto* ptr = boost::get<T>(&*opt_variant)){
            functor(*ptr);
        }
    }
}

/*!
 * \brief Execute the given functor if the given optional variant is initialized and is equals to the given value. 
 * \param opt_variant The optional variant. 
 * \param cmp The object the variant must be equals to. 
 * \param functor The functor to execute. Must have an operator()().
 * \tparam T The type that must be contained in the variant. 
 * \tparam OptVariant The optional variant type. 
 * \tparam Functor the type of the functor. 
 */
template<typename T, typename OptVariant, typename Functor>
void if_init_equals(OptVariant& opt_variant, T& cmp, Functor&& functor){
    if(opt_variant){
        if(auto* ptr = boost::get<T>(&*opt_variant)){
            if(*ptr == cmp){
                functor();
            }
        }
    }
}

/*!
 * \brief Execute the given functor if the given optional variant is initialized and is not equals to the given value. 
 * \param opt_variant The optional variant. 
 * \param cmp The object the variant must be not equals to. 
 * \param functor The functor to execute. Must have an operator()(T).
 * \tparam T The type that must be contained in the variant. 
 * \tparam OptVariant The optional variant type. 
 * \tparam Functor the type of the functor. 
 */
template<typename T, typename OptVariant, typename Functor>
void if_init_not_equals(OptVariant& opt_variant, T& cmp, Functor&& functor){
    if(opt_variant){
        if(auto* ptr = boost::get<T>(&*opt_variant)){
            if(*ptr != cmp){
                functor(*ptr);
            }
        }
    }
}

} //end of eddic

#endif
