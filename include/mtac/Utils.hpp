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
#include "variant_utils.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/Program.hpp"
#include "mtac/Operator.hpp"
#include "mtac/forward.hpp"

namespace eddic {

class Variable;
struct GlobalContext;

namespace mtac {

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

typedef std::unordered_map<mtac::basic_block_p, mtac::basic_block_p> BBClones;

void replace_bbs(BBClones& clones, mtac::Quadruple& quadruple);
void replace_bbs(BBClones& clones, mtac::basic_block_p& bb);
void replace_bbs(BBClones& clones, mtac::Function& function);

template<typename Visitor>
void visit_all_statements(Visitor& visitor, mtac::Function& function){
    for(auto& block : function){
        visit_each(visitor, block->statements);
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
