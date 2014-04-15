//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
        position(const eddic::lexer::pos_iterator_type& position_begin, int current_file) : position_begin(position_begin), current_file(current_file) {}

        template <typename Context, typename Iterator>
        struct attribute
        {
            typedef eddic::ast::Position type;
        };

        template <typename Iterator, typename Context
            , typename Skipper, typename Attribute>
            bool parse(Iterator& /*first*/, Iterator const& /*last*/
                    , Context& /*context*/, Skipper const& /*skipper*/, Attribute& attr) const
        {
            auto& pos = position_begin.get_position();

            attr.file = current_file;
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
        int current_file;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Parser generators: make_xxx function (objects)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Modifiers>
    struct make_primitive<tag::position, Modifiers>
    {
        typedef position result_type;

        template<typename Arg>
        result_type operator()(unused_type, const Arg& lexer, unused_type) const
        {
            return result_type(lexer);
        }
    };
}}}

#endif
