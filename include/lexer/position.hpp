//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SPIRIT_LEXER_POSITION_H
#define SPIRIT_LEXER_POSITION_H

#include "lexer/SpiritLexer.hpp"

namespace boost { namespace spirit
{
    BOOST_SPIRIT_TERMINAL(position)

    ///////////////////////////////////////////////////////////////////////////
    // Enablers
    ///////////////////////////////////////////////////////////////////////////
    template <>
    struct use_terminal<qi::domain, tag::position> // enables position
        : mpl::true_ {};
}}

namespace boost { namespace spirit { namespace qi
{
    struct position : primitive_parser<position>
    {
        position(const eddic::lexer::pos_iterator_type& position_begin) : position_begin(position_begin) {}

        template <typename Context, typename Iterator>
        struct attribute
        {
            typedef eddic::ast::Position type;
        };

        template <typename Iterator, typename Context
            , typename Skipper, typename Attribute>
            bool parse(Iterator& first, Iterator const& last
                    , Context& /*context*/, Skipper const& skipper, Attribute& attr) const
        {
            qi::skip_over(first, last, skipper);
            
            auto& pos = position_begin.get_position();

            attr.theLine = position_begin.get_currentline();
            attr.file = pos.file;
            attr.column = pos.column;
            attr.line = pos.line;

            return true;
        }

        template <typename Context>
        info what(Context&/* context*/) const
        {
            return info("position");
        }

        const eddic::lexer::pos_iterator_type& position_begin;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Parser generators: make_xxx function (objects)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Modifiers>
    struct make_primitive<tag::position, Modifiers>
    {
        typedef position result_type;
        result_type operator()(unused_type, eddic::lexer::Lexer const& lexer, unused_type) const
        {
            return result_type(lexer);
        }
    };
}}}

#endif
