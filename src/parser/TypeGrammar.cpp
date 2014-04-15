//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

//#define BOOST_SPIRIT_DEBUG
//#include <boost/spirit/include/qi.hpp>

#include "parser/TypeGrammar.hpp"
#include "lexer/position.hpp"

using namespace eddic;

parser::TypeGrammar::TypeGrammar(const lexer::StaticLexer& lexer) :
        TypeGrammar::base_type(type, "Type Grammar")
{
    auto const_ = (
            (lexer.const_ > boost::spirit::attr(true))
        |   boost::spirit::attr(false)
    );

    auto simple_type = boost::spirit::qi::as<ast::SimpleType>()[(
            const_
        >>  lexer.identifier
    )];

    template_type %=
            lexer.identifier
        >>  qi::omit[lexer.less]
        >>  type
        >>  *(lexer.comma >> type)
        >>  qi::omit[lexer.greater];

    array_type %=
            (
                    template_type
                |   simple_type
            )
        >>  lexer.left_bracket
        >>  lexer.right_bracket;

    pointer_type %=
           (
                    template_type
                |   simple_type
            )
        >>  lexer.multiplication;

    type %=
            array_type
        |   pointer_type
        |   template_type
        |   simple_type;
}
