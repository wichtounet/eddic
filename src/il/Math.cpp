//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Math.hpp"

using namespace eddic;

Math::Math(Operation operation, std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) : m_operation(operation), m_lhs(lhs), m_rhs(rhs) {}

void Math::write(AssemblyFileWriter& writer){
   //TODO 
}
