//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_UTILS_H
#define LTAC_UTILS_H

#include <memory>
#include "variant.hpp"

#include "mtac/forward.hpp"
#include "mtac/Argument.hpp"
#include "mtac/Utils.hpp"

#include "ltac/forward.hpp"
#include "ltac/Operator.hpp"
#include "ltac/Argument.hpp"

namespace eddic {

namespace ltac {

class RegisterManager;

bool is_float_var(std::shared_ptr<Variable> variable);
bool is_int_var(std::shared_ptr<Variable> variable);

bool transform_to_nop(ltac::Instruction& instruction);

template<typename T>
inline bool is_reg(T value){
    return mtac::is<ltac::Register>(value);
}

template<typename T>
inline bool is_float_reg(T value){
    return mtac::is<ltac::FloatRegister>(value);
}

template<typename Variant>
bool is_variable(Variant& variant){
    return boost::get<std::shared_ptr<Variable>>(&variant);
}

template<typename Variant>
std::shared_ptr<Variable> get_variable(Variant& variant){
    return boost::get<std::shared_ptr<Variable>>(variant);
}

ltac::PseudoRegister to_register(std::shared_ptr<Variable> var, ltac::RegisterManager& manager);
ltac::PseudoFloatRegister to_float_register(std::shared_ptr<Variable> var, ltac::RegisterManager& manager);
ltac::Argument to_arg(mtac::Argument argument, ltac::RegisterManager& manager);

} //end of ltac

} //end of eddic

#endif
