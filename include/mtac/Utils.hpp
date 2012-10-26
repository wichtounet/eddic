//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_UTILS_H
#define MTAC_UTILS_H

#include <memory>
#include <utility>
#include <unordered_set>
#include <unordered_map>

#include "variant.hpp"
#include "Type.hpp"

#include "mtac/Program.hpp"
#include "mtac/Operator.hpp"

namespace eddic {

class Variable;
class GlobalContext;

namespace mtac {

template<typename V, typename T>
inline bool equals(T& variant, V value){
    return boost::get<V>(&variant) && value == boost::get<V>(variant);
}

template<typename V, typename T>
inline bool is(T& variant){
    return boost::get<V>(&variant);
}

template<typename T>
inline bool isInt(T& variant){
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
}

template<typename Visitor>
void visit_all_statements(Visitor& visitor, std::shared_ptr<mtac::Function> function){
    for(auto& block : function){
        visit_each(visitor, block->statements);
    }
}

bool is_single_int_register(std::shared_ptr<const Type> type);
bool is_single_float_register(std::shared_ptr<const Type> type);

unsigned int compute_member_offset(std::shared_ptr<GlobalContext> context, std::shared_ptr<Variable> var, const std::vector<std::string>& memberNames);
std::pair<unsigned int, std::shared_ptr<const Type>> compute_member(std::shared_ptr<GlobalContext> context, std::shared_ptr<Variable> var, const std::vector<std::string>& memberNames);

void computeBlockUsage(std::shared_ptr<mtac::Function> function, std::unordered_set<std::shared_ptr<mtac::BasicBlock>>& usage);

typedef std::unordered_map<std::shared_ptr<Variable>, unsigned int> VariableUsage;

VariableUsage compute_variable_usage(std::shared_ptr<mtac::Function> function);
VariableUsage compute_variable_usage_with_depth(std::shared_ptr<mtac::Function> function, int factor);

bool is_recursive(std::shared_ptr<mtac::Function> function);

bool safe(const std::string& call);
bool safe(std::shared_ptr<mtac::Call> call);
bool erase_result(mtac::Operator op);
bool is_distributive(mtac::Operator op);
bool is_expression(mtac::Operator op);

mtac::Statement copy(const mtac::Statement& statement, std::shared_ptr<GlobalContext> context);

} //end of mtac

} //end of eddic

#endif
