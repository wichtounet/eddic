//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant/apply_visitor.hpp>

#include "IsConstantVisitor.hpp"

#include "ast/Value.hpp"

using namespace eddic;

bool IsConstantVisitor::operator()(ASTLitteral&) const {
    return true;
}

bool IsConstantVisitor::operator()(ASTInteger&) const {
    return true;
}

bool IsConstantVisitor::operator()(ASTVariable&) const {
    return false;
}

bool IsConstantVisitor::operator()(ASTComposedValue& value) const {
    if(boost::apply_visitor(*this, value.first)){
        for(auto& op : value.operations){
            if(!boost::apply_visitor(*this, op.get<1>())){
                return false;
            }
        }

        return true;
    }

    return false;
}
