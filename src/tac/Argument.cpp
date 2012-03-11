//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Argument.hpp"

using namespace eddic;

bool eddic::tac::operator==(const  tac::Argument& a, const tac::Argument& b){ 
    return eddi_detail::variant_equals()(a, b); 
}

bool eddic::tac::operator==(const tac::Argument& a, int b){
    return boost::get<int>(&a) && boost::get<int>(a) == b;
}

bool eddic::tac::operator==(const tac::Argument& a, double b){
    return boost::get<double>(&a) && boost::get<double>(a) == b;
}

bool eddic::tac::operator==(const tac::Argument& a, const std::string& b){
    return boost::get<std::string>(&a) && boost::get<std::string>(a) == b;
}

bool eddic::tac::operator==(const tac::Argument& a, std::shared_ptr<Variable> b){
    return boost::get<std::shared_ptr<Variable>>(&a) && boost::get<std::shared_ptr<Variable>>(a) == b;
}
