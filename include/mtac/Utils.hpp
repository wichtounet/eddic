//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_UTILS_H
#define MTAC_UTILS_H

#include <memory>
#include <utility>
#include <unordered_set>
#include <unordered_map>

#include "variant.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/Program.hpp"
#include "mtac/Operator.hpp"
#include "mtac/forward.hpp"

namespace eddic {

class Variable;
struct GlobalContext;

namespace mtac {

typedef std::unordered_map<mtac::basic_block_p, mtac::basic_block_p> BBClones;

void replace_bbs(BBClones& clones, mtac::Quadruple& quadruple);
void replace_bbs(BBClones& clones, mtac::basic_block_p& bb);
void replace_bbs(BBClones& clones, mtac::Function& function);

template<typename V, typename T>
inline bool equals(T& variant, V value){
    return boost::get<V>(&variant) && value == boost::get<V>(variant);
}

template<typename V, typename T>
inline bool is(T& variant){
    return boost::get<V>(&variant);
}

template<typename V, typename T>
inline bool optional_is(T& variant){
    return variant && boost::get<V>(&*variant);
}

template<typename T>
inline bool isInt(T variant){
    return boost::get<int>(&variant);
}

template<typename T>
inline bool isFloat(T& variant){
    return boost::get<double>(&variant);
}

template<typename T>
inline bool isVariable(T& variant){
    return boost::get<std::shared_ptr<Variable>>(&variant);
}

template<typename T>
inline void assertIntOrVariable(T& variant){
    assert(isInt(variant) || isVariable(variant));
    _unused(variant);
}

template<typename Visitor>
void visit_all_statements(Visitor& visitor, mtac::Function& function){
    for(auto& block : function){
        visit_each(visitor, block->statements);
    }
}

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

bool is_single_int_register(std::shared_ptr<const Type> type);
bool is_single_float_register(std::shared_ptr<const Type> type);

unsigned int compute_member_offset(std::shared_ptr<const GlobalContext> context, std::shared_ptr<const Type> typer, const std::string& member);
std::pair<unsigned int, std::shared_ptr<const Type>> compute_member(std::shared_ptr<const GlobalContext> context, std::shared_ptr<const Type> type, const std::string& member);

void computeBlockUsage(mtac::Function& function, std::unordered_set<mtac::basic_block_p>& usage);

bool is_recursive(mtac::Function& function);

bool safe(const std::string& call);
bool erase_result(mtac::Operator op);

mtac::Quadruple copy(const mtac::Quadruple& statement);

} //end of mtac

} //end of eddic

#endif
