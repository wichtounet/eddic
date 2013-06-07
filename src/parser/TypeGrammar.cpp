//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

//#define BOOST_SPIRIT_DEBUG
//#include <boost/spirit/include/qi.hpp>

#include "parser/TypeGrammar.hpp"
#include "lexer/position.hpp"

using namespace eddic;

parser::TypeGrammar::TypeGrammar(const lexer::Lexer& lexer) : 
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

    auto template_type = boost::spirit::qi::as<ast::TemplateType>()[(
            lexer.identifier
        >>  qi::omit[lexer.less]
        >>  type
        >>  *(lexer.comma >> type)
        >>  qi::omit[lexer.greater]
    )];

    auto array_type = boost::spirit::qi::as<ast::ArrayType>()[(
            (
                    template_type
                |   simple_type
            )
        >>  lexer.left_bracket
        >>  lexer.right_bracket
    )];
    
    auto pointer_type = boost::spirit::qi::as<ast::PointerType>()[(
            (
                    template_type
                |   simple_type
            )
        >>  lexer.multiplication
    )];

    type %=
            array_type
        |   pointer_type
        |   template_type
        |   simple_type;
}
