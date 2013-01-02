//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Quadruple.hpp"

using namespace eddic;

mtac::Quadruple::Quadruple(){
    //Nothing to init    
}

mtac::Quadruple::Quadruple(mtac::Operator o) : op(o) {
    //Nothing to init    
}

mtac::Quadruple::Quadruple(std::shared_ptr<Variable> result, mtac::Argument a1, mtac::Operator o) : result(result), arg1(a1), op(o) {
    //Nothing to init    
}

mtac::Quadruple::Quadruple(std::shared_ptr<Variable> result, mtac::Argument a1, mtac::Operator o, mtac::Argument a2) : result(result), arg1(a1), arg2(a2), op(o) {
    //Nothing to init    
}

mtac::Quadruple::Quadruple(mtac::Operator o, mtac::Argument a1) : arg1(a1), op(o) {
    //Nothing to init    
}

mtac::Quadruple::Quadruple(mtac::Operator o, mtac::Argument a1, mtac::Argument a2) : arg1(a1), arg2(a2), op(o) {
    //Nothing to init    
}

const std::string& mtac::Quadruple::label() const {
    return boost::get<std::string>(*arg1);
}
