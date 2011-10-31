//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PROGRAM_H
#define AST_PROGRAM_H

#include <vector>

#include <boost/variant/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/spirit/home/support/attributes.hpp>

#include "ast/FunctionDeclaration.hpp"
#include "ast/GlobalVariableDeclaration.hpp"

namespace eddic {

typedef boost::variant<FunctionDeclaration, GlobalVariableDeclaration> FirstLevelBlock;
typedef std::vector<FirstLevelBlock> ProgramEquivalence;

//A source EDDI program
struct Program {
    std::vector<FirstLevelBlock> blocks;
};

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::Program, 
    (std::vector<eddic::FirstLevelBlock>, blocks)
)

//Enable the use as one-attribute
namespace boost { namespace spirit { namespace traits {
    /*
    template <>
    struct assign_to_attribute_from_value <eddic::Program, eddic::ProgramEquivalence, qi::domain> {
        static void call(const eddic::ProgramEquivalence& val, eddic::Program& attr){
            attr.blocks = val;
        }
    };
    */

    /*
    template<>
    struct transform_attribute<eddic::Program, std::vector<eddic::FirstLevelBlock>, qi::domain> {
        typedef std::vector<eddic::FirstLevelBlock>& type;

        static type pre(eddic::Program& program) { return program.blocks; }
        static void post(eddic::Program&, const type) {}
        static void fail(eddic::Program&) {}
    };*/
}}}

#endif
