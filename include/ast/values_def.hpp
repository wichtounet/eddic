//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VALUES_DEF_H
#define VALUES_DEF_H

#include <boost/intrusive_ptr.hpp>
#include "ast/Deferred.hpp"

namespace eddic {

struct ASTInteger;
struct ASTLitteral;

struct TmpVariable;
typedef Deferred<TmpVariable> ASTVariable;

struct ComposedValue;
typedef Deferred<ComposedValue, boost::intrusive_ptr<ComposedValue>> ASTComposedValue;

} //end of eddic

#endif
