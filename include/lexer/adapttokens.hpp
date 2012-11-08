//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SPIRIT_LEXER_ADAPT_TOKENS_H
#define SPIRIT_LEXER_ADAPT_TOKENS_H

namespace boost { namespace spirit {

BOOST_SPIRIT_TERMINAL(adapttokens)

template <>
struct use_directive<qi::domain, tag::adapttokens> : mpl::true_ {};

}}

namespace boost { namespace spirit { namespace qi {

using spirit::adapttokens;
using spirit::adapttokens_type;

template <typename Subject>
struct adapttokens_directive : unary_parser<adapttokens_directive<Subject>> {
    typedef Subject subject_type;

    adapttokens_directive(Subject const& subject) : subject(subject) {}

    template <typename Context, typename Iterator>
    struct attribute {
        typedef iterator_range<Iterator> type;
    };

    template<typename Iterator>
    inline std::string convert(Iterator& first, Iterator& last) const {
        std::stringstream out;

        while(first != last){
            out << *first;

            ++first;   
        }

        return out.str();
    }

    template <typename Iterator, typename Context, typename Skipper, typename Attribute>
    bool parse(Iterator& first, Iterator const& last, Context& context, Skipper const& skipper, Attribute& attr) const {
        qi::skip_over(first, last, skipper);

        auto tokenfirst = first->value().begin();
        auto tokenlast = first->value().end();

        if (subject.parse(tokenfirst, tokenlast, context, skipper, unused) && tokenfirst == tokenlast){
            tokenfirst = first->value().begin();
            tokenlast = first->value().end();

            attr = *subject.ref.get().find(convert(tokenfirst, tokenlast));
            ++first;

            return true;
        }

        return false;
    }

    template <typename Context>
    info what(Context& context) const {
        return info("adapttokens", subject.what(context));

    }

    Subject subject;
};

template <typename Subject, typename Modifiers>
struct make_directive<tag::adapttokens, Subject, Modifiers> {
    typedef adapttokens_directive<Subject> result_type;
    result_type operator()(unused_type, Subject const& subject, unused_type) const {
        return result_type(subject);
    }
};

}}} //end of boost::spirit::qi

namespace boost { namespace spirit { namespace traits {

template <typename Subject>
struct has_semantic_action<qi::adapttokens_directive<Subject>> : unary_has_semantic_action<Subject> {};

template <typename Subject, typename Attribute, typename Context, typename Iterator>
struct handles_container<qi::adapttokens_directive<Subject>, Attribute, Context, Iterator>
    : unary_handles_container<Subject, Attribute, Context, Iterator> {};

}}} //end of boost::spirit::traits

#endif
