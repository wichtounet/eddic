//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GET_TYPE_VISITOR_H
#define GET_TYPE_VISITOR_H

#include "Types.hpp"

namespace eddic {

class ASTLitteral;
class ASTInteger;
class ASTVariable;
class ASTComposedValue;

struct GetTypeVisitor : public boost::static_visitor<Type> {
    Type operator()(ASTLitteral& litteral) const;
    Type operator()(ASTInteger& litteral) const;
    Type operator()(ASTVariable& variable) const;
    Type operator()(ASTComposedValue& value) const; 
};

} //end of eddic

#endif
