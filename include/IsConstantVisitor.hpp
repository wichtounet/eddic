//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IS_CONSTANT_VISITOR_H
#define IS_CONSTANT_VISITOR_H

//TODO See if it is necessary to include whole variant
#include <boost/variant/variant.hpp>

namespace eddic {

class ASTLitteral;
class ASTInteger;
class ASTVariable;
class ASTComposedValue;

struct IsConstantVisitor : public boost::static_visitor<bool> {
    bool operator()(ASTLitteral& litteral) const;
    bool operator()(ASTInteger& litteral) const;
    bool operator()(ASTVariable& variable) const;
    bool operator()(ASTComposedValue& value) const; 
};

} //end of eddic

#endif
