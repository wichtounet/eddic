//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "TransformerEngine.hpp"

#include <boost/variant/static_visitor.hpp>

#include "GetTypeVisitor.hpp"
#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"
#include "Variable.hpp"

using namespace eddic;

struct TransformerVisitor : public boost::static_visitor<> {
    void operator()(ast::Program& program){
        //TODO
    }
};

void TransformerEngine::transform(ast::Program& program){
    TransformerVisitor visitor;
    visitor(program);
}
