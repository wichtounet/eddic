//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_DEFAULT_CASE_H
#define AST_DEFAULT_CASE_H

#include <vector>
#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

class Context;

namespace ast {

/*!
 * \class DefaultCase 
 * \brief The AST node for a default switch case.
 */
struct DefaultCase {
    std::shared_ptr<Context> context; 

    std::vector<Instruction> instructions;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::DefaultCase, 
    (std::vector<eddic::ast::Instruction>, instructions)
)

#endif
